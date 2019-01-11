namespace MUEX{
  bool compat=false; // compatibility with older pre-wreco muex trunk

  struct mydom{
    double x, y, z, hv, eff, st;
  };

  struct mykey{
    int str, om;
    mykey(int str, int om){
      this->str=str, this->om=om;
    }
    bool operator< (const mykey& rhs) const {
      return str < rhs.str || (str == rhs.str && om < rhs.om);
    }

  };

  set<mykey> boms;
  map<mykey, mydom> doms;

  static const double dr=0.16510; // DOM radius
  static const double pmin=1e-9;  // minimum hit probability

  static const double dura=2.e3;    // typical event duration [ns]
  static const double nois=0.5e-6;  // noise hit rate [1/ns]
  static const double umin=nois*dura;// number of noise hits per DOM in an event

                                     // efficiency correction: SPE charge mean
  static const double efcr=0.75*0.9; // approx. equal to 1-discriminator losses
                                     // times (1-losses, e.g., cable shadowing)
  static const double zoff=1948.07;  // depth of detector center
  static const double rho=0.9216; // ice density
  static const double arho=0.26*rho, brho=0.36e-3*rho; // rate of muon energy loss [per m]
  static const double Cf=2450.08*(M_PI*dr*dr), Ff=5.21*(0.924/rho); // photons per GeV

  struct rrt{
    gsl_rng * G;
    double A[3][3], n[3];

    void ini(gsl_rng * G = NULL){  // random rotation transformation
      this->G=G;
      if(G!=NULL){
	double c=2*gsl_rng_uniform(G)-1;
	double s=sqrt(1-c*c);
	double a=2*M_PI*gsl_rng_uniform(G);
	double x=s*cos(a), y=s*sin(a), z=c;

	a=2*M_PI*gsl_rng_uniform(G);
	c=cos(a), s=sin(a);
	double C=1-c;

	A[0][0]=x*x*C+c, A[0][1]=x*y*C-z*s, A[0][2]=x*z*C+y*s;
	A[1][0]=y*x*C+z*s, A[1][1]=y*y*C+c, A[1][2]=y*z*C-x*s;
	A[2][0]=z*x*C-y*s, A[2][1]=z*y*C+x*s, A[2][2]=z*z*C+c;
      }
      else{
	for(int i=0; i<3; i++) for(int j=0; j<3; j++) A[i][j]=i==j?1:0;
      }
    }

    double * rot(double * m){  // random rotation
      for(int i=0; i<3; i++){
	double sum=0;
	for(int j=0; j<3; j++) sum+=A[i][j]*m[j];
	n[i]=sum;
      }
      return n;
    }

    double * tor(double * m){  // inverse of random rotation
      for(int i=0; i<3; i++){
	double sum=0;
	for(int j=0; j<3; j++) sum+=A[j][i]*m[j];
	n[i]=sum;
      }
      return n;
    }

    void ini(double * n){  // rotate to align z along vector n
      double zi=G!=NULL?2*M_PI*gsl_rng_uniform(G):0;
      double px=cos(zi), py=sin(zi);

      double p1[3], p2[3];
      {
	int i0=0;
	double n0=n[0];
	for(int i=1; i<3; i++) if(fabs(n[i])>fabs(n0)){ i0=i; n0=n[i]; }
	int i1=(i0+1)%3, i2=(i0+2)%3;
	double n1=n[i1], n2=n[i2], nq=n0*n0;
	double r1=1/sqrt(nq+n1*n1), r2=1/sqrt(nq+n2*n2);

	p1[i0]=-n1*r1; p1[i1]=n0*r1; p1[i2]=0;
	p2[i0]=-n2*r2; p2[i1]=0; p2[i2]=n0*r2;
      }
      {
	double q1[3], q2[3], r1=0, r2=0;
	for(int i=0; i<3; i++){
	  double a1=p1[i]-p2[i]; q1[i]=a1; r1+=a1*a1;
	  double a2=p1[i]+p2[i]; q2[i]=a2; r2+=a2*a2;
	}

	r1=1/sqrt(r1); r2=1/sqrt(r2);

	for(int i=0; i<3; i++){
	  p1[i]=q1[i]*r1;
	  p2[i]=q2[i]*r2;
	}
      }

      for(int i=0; i<3; i++){
	A[i][0]=px*p1[i]+py*p2[i];
	A[i][1]=-py*p1[i]+px*p2[i];
	A[i][2]=-n[i];
      }
    }

    double * ste(double x, double y){  // inverse stereographic projection
      double r=1+x*x+y*y;
      double m[3]={2*x/r, 2*y/r, 1-2/r};
      for(int i=0; i<3; i++){
	double sum=0;
	for(int j=0; j<3; j++) sum+=A[i][j]*m[j];
	n[i]=sum;
      }
      return n;
    }
  };

  struct dir{
    double n[3];

    dir(){}

    dir(const double n[]){
      for(int i=0; i<3; i++) this->n[i]=n[i];
    }

    bool operator< (const dir& rhs) const {
      return n[0] < rhs.n[0] || (n[0] == rhs.n[0] && (n[1] < rhs.n[1] || (n[1] == rhs.n[1] && n[2] < rhs.n[2])));
    }
  };

  struct vert:dir{
    double f;

    vert(){}

    vert(const dir n, double f) : dir(n){
      this->f=f;
    }
  };

  ostream& operator<<(ostream& out, const dir& n){
    out<<n.n[0]<<" "<<n.n[1]<<" "<<n.n[2];
    return out;
  }

  struct face{
    dir v[3];

    face(const dir & x, const dir & y, const dir & z){
      v[0]=x, v[1]=y, v[2]=z;
    }

    void half(dir h[]){
      for(int i=0; i<3; i++){
	h[0].n[i]=v[1].n[i]+v[2].n[i];
	h[1].n[i]=v[2].n[i]+v[0].n[i];
	h[2].n[i]=v[0].n[i]+v[1].n[i];
      }
      double r[3]={0, 0, 0};
      for(int i=0; i<3; i++) for(int j=0; j<3; j++) r[i]+=h[i].n[j]*h[i].n[j];
      for(int i=0; i<3; i++){
	r[i]=1/sqrt(r[i]);
	for(int j=0; j<3; j++) h[i].n[j]*=r[i];
      }
    }
  };

  struct Ico{
    int vn, fn;
    vector<dir> v;
    vector< vector<int> > f;

    Ico(int num) : vn(0), fn(0){
      vector<face> set;

      {
	const double t=(1+sqrt(5))/2;
	const double r=sqrt(1+t*t);
	const double a=t/r, b=1/r;

	const int vn=12, fn=20;
	double v[vn][3];
	int f[fn][3];

	for(int i=0; i<vn; i++){
	  int x=i/4, j=i%4;
	  int y=(x+1)%3, z=(x+2)%3;
	  v[i][x]=j==0||j==3?a:-a;
	  v[i][y]=j==0||j==1?b:-b;
	  v[i][z]=0;
	}

	for(int i=0, n=0; i<vn; i++)
	  for(int j=i+1; j<11; j++)
	    if(v[i][0]*v[j][0]+v[i][1]*v[j][1]+v[i][2]*v[j][2]>0)
	      for(int k=j+1; k<vn; k++)
		if(v[j][0]*v[k][0]+v[j][1]*v[k][1]+v[j][2]*v[k][2]>0 &&
		   v[k][0]*v[i][0]+v[k][1]*v[i][1]+v[k][2]*v[i][2]>0) f[n][0]=i, f[n][1]=j, f[n++][2]=k;

	if(false){
	  for(int i=0; i<vn; i++) cout<<v[i][0]<<" "<<v[i][1]<<" "<<v[i][2]<<endl;
	  for(int i=0; i<fn; i++) cout<<f[i][0]<<" "<<f[i][1]<<" "<<f[i][2]<<endl;
	}

	for(int i=0; i<fn; i++){
	  int x=f[i][0], y=f[i][1], z=f[i][2];
	  set.push_back(face(dir(v[x]), dir(v[y]), dir(v[z])));
	}
      }

      for(int k=0; k<num; k++){
	vector<face> add;

	for(vector<face>::iterator i=set.begin(); i!=set.end(); i++){
	  face & one = *i;
	  dir * w = one.v;

	  dir u[3];
	  one.half(u);

	  add.push_back(face(w[0], u[1], u[2]));
	  add.push_back(face(w[1], u[2], u[0]));
	  add.push_back(face(w[2], u[0], u[1]));
	  add.push_back(face(u[0], u[1], u[2]));
	}

	set=add;
      }

      {
	map<dir, int> dirs;
	for(vector<face>::const_iterator i=set.begin(); i!=set.end(); i++){
	  const dir * w = i->v;
	  for(int i=0; i<3; i++){
	    const dir & wi = w[i];
	    if(dirs.find(wi)==dirs.end()) dirs.insert(make_pair(wi, 0));
	  }
	}

	for(map<dir, int>::iterator i=dirs.begin(); i!=dirs.end(); i++) v.push_back(i->first), i->second=vn++;
	for(vector<face>::const_iterator i=set.begin(); i!=set.end(); i++){
	  vector<int> tmp;
	  for(int j=0; j<3; j++) tmp.push_back(dirs[i->v[j]]);
	  f.push_back(tmp); fn++;
	}
      }

      // cerr<<"Icosahedral mesh("<<num<<"): points="<<vn<<", triangles="<<fn<<endl;
    }
  } ico(2);

  // holds hole ice angular sensitivity parameterization
  struct sens{
    double ave;
    double a[11];

    double s(double coseta){
      double y=1, sum=a[0];
      for(int i=1; i<=10; i++){ y*=coseta; sum+=a[i]*y; }
      return sum;
    }

    double s(double coseta, double y){
      return isfinite(y)?(s(coseta)+ave*y)/(1+y):ave;
    }

    sens(){
      static double a[11]={0.32813, 0.63899, 0.20049, -1.2250, -0.14470,
			   4.1695, 0.76898, -5.8690, -2.0939, 2.3834, 1.0435};
      for(int i=0;i<11;i++) this->a[i]=a[i];
      double sum=a[0];
      for(int i=2; i<=10; i+=2){ sum+=a[i]/(i+1); }
      ave=sum; // average sensitivity
    }
  } asens;

  void check(double & x){
    if(!isfinite(x) || x<0) x=0;
  }

  struct Tilt{
#define LMAX 6    // number of dust loggers
#define LYRS 170  // number of depth points

    bool tilt;
    int lnum, lpts, l0;
    double lmin, lrdz, r0;
    double lnx, lny;
    double lr[LMAX];
    double lp[LMAX][LYRS];

    Tilt() : tilt(false), lnum(0), l0(0), r0(0) { }

    void ini(string dir){
      if(tilt) return;
      const double cv=M_PI/180, thx=225;
      lnx=cos(cv*thx), lny=sin(cv*thx);

      ifstream inFile((dir+"tilt.par").c_str(), ifstream::in);
      if(!inFile.fail()){
	int str;
	double aux;
	vector<double> vlr;
	while(inFile >> str >> aux){ if(aux==0) l0=str; vlr.push_back(aux); }
	inFile.close();

	int size=vlr.size();
	if(size>LMAX){ cerr << "File tilt.par defines too many dust maps" << endl; return; }
	for(int i=1; i<size; i++) if(vlr[i]<vlr[i-1]) { cerr << "Tilt map does not use increasing range order" << endl; return; };
	for(int i=0; i<size; i++) lr[i]=vlr[i];

	ifstream inFile((dir+"tilt.dat").c_str(), ifstream::in);
	if(!inFile.fail()){
	  lnum=size;
	  double depth;
	  vector<double> pts(lnum), ds;
	  vector< vector<double> > vlp(lnum);
	  while(inFile >> depth){
	    int i=0;
	    while(inFile >> pts[i++]) if(i>=lnum) break;
	    if(i!=lnum) break;
	    ds.push_back(depth);
	    for(i=0; i<lnum; i++) vlp[i].push_back(pts[i]);
	  }
	  inFile.close();

	  int size=ds.size();
	  if(size>LYRS){ cerr << "File tilt.dat defines too many map points" << endl; return; }
	  for(int i=1; i<size; i++) if(ds[i]<ds[i-1]) { cerr << "Tilt map does not use increasing depth order" << endl; return; };
	  for(int i=0; i<lnum; i++) for(int j=0; j<size; j++) lp[i][j]=vlp[i][size-1-j];
	  lpts=size;

	  if(size<2) lnum=0;
	  else{
	    double vmin=ds[0], vmax=ds[size-1];
	    lmin=zoff-vmax; lrdz=(lpts-1)/(vmax-vmin);
	  }
	}

	if(lnum>0) log_info_stream("Loaded "<<lnum<<"x"<<lpts<<" dust layer points");
      }

      tilt=true;
    }

    void set_r0(){
      int sn=0;
      double sx=0, sy=0;
      for(map<mykey, mydom>::const_iterator i = doms.begin(); i!=doms.end(); ++i){
	const mykey& key = i->first;
	const mydom& dom = i->second;
	if(key.str==l0) sx+=dom.x, sy+=dom.y, sn++;
      }

      if(sn>0){
	sx/=sn, sy/=sn;
	r0=lnx*sx+lny*sy;
      }

      log_debug_stream("r0 is set to "<<r0);
    }

    double zshift(double rx, double ry, double rz){
      if(!tilt || lnum==0) return 0;
      double z=(rz-lmin)*lrdz;
      int k=min(max((int)floor(z), 0), lpts-2);
      int l=k+1;

      double nr=lnx*rx+lny*ry-r0, result=0;
      for(int j=1; j<LMAX; j++) if(nr<lr[j] || j==lnum-1){
	  int i=j-1;
	  result = ( (lp[j][l]*(z-k)+lp[j][k]*(l-z))*(nr-lr[i]) +
		     (lp[i][l]*(z-k)+lp[i][k]*(l-z))*(lr[j]-nr) )/(lr[j]-lr[i]);
	  break;
	}
      return result;
    }
  } tilt;

  struct Ice{
    double wv;    // light wavelength
    double np;    // phase refractive index
    double ng;    // group refractive index
    double c, cm; // speed of light in vacuum and in medium
    double na;    // average propagation refractive index
    double le;    // scattering length
    double la;    // absorption length

  private:
    bool kurt;                 // whether kurt table is used
    int size;                  // size of kurt table
    double dh, hmin, hmax;     // step, min and max depth
    double *abs, *sca;         // arrays of absorption and scattering values
    double *abs_sum, *sca_sum; // arrays of absorption and scattering depth-integrated values
  public:
    double coschr, sinchr, tanchr;   // cos, sin, and tan of the cherenkov angle

    Ice() : wv(0.4), c(0.299792458), le(35.), la(100.), kurt(false){
      if(wv<0.3) wv=0.3; else if(wv>0.6) wv=0.6;

      double wv2=wv*wv;
      double wv3=wv*wv2;
      double wv4=wv*wv3;
      np=1.55749-1.57988*wv+3.99993*wv2-4.68271*wv3+2.09354*wv4;
      ng=np*(1+0.227106-0.954648*wv+1.42568*wv2-0.711832*wv3);
      na=(np*ng-1)/sqrt(np*np-1); cm=c/ng;
      coschr=1/np; sinchr=sqrt(1-coschr*coschr); tanchr=sinchr/coschr;
    }

    void ini(string dir){
      if(kurt) return;

      gsl_set_error_handler_off();

      double wv0=400;
      double A, B, D, E, a, k;
      double Ae, Be, De, Ee, ae, ke;
      vector<double> dp, be, ba, td;

      bool flag=true, fail=false;
      ifstream inFile((dir+"icemodel.par").c_str(), ifstream::in);

      if((flag=!inFile.fail())){
	if(flag) flag=!!(inFile >> a >> ae);
	if(flag) flag=!!(inFile >> k >> ke);
	if(flag) flag=!!(inFile >> A >> Ae);
	if(flag) flag=!!(inFile >> B >> Be); fail=!flag;
	if(flag) flag=!!(inFile >> D >> De); if(!flag) D=pow(wv0, k);
	if(flag) flag=!!(inFile >> E >> Ee); if(!flag) E=0;
	if(fail) cerr << "File icemodel.par found, but is corrupt, falling back to bulk ice" << endl;
	inFile.close(); if(fail) return;
      }
      else{ cerr<<"Using homogeneous ice"<<endl; return; }

      inFile.open((dir+"icemodel.dat").c_str(), ifstream::in);
      if(!inFile.fail()){
	size=0;
	double dpa, bea, baa, tda;
	while(inFile >> dpa >> bea >> baa >> tda){
	  dp.push_back(dpa);
	  be.push_back(bea);
	  ba.push_back(baa);
	  td.push_back(tda);
	  size++;
	}
	if(size<2){ cerr << "File icemodel.dat found, but is corrupt, falling back to bulk ice" << endl; return; }
	inFile.close();
      }
      else{ cerr<<"Using homogeneous ice"<<endl; return; }

      double wva=wv*1.e3;
      double l_a=pow(wva/wv0, -a);
      double l_k=pow(wva, -k);
      double ABl=A*exp(-B/wva);

      abs = new double[size];
      sca = new double[size];

      int num=0;
      double abs_a=0, sca_a=0;
      dh=dp[1]-dp[0];
      if(dh<=0){ cerr << "Ice table does not use increasing depth spacing, falling back to bulk ice" << endl; return; }

      for(int i=0; i<size; i++){
	if(i>1) if(fabs(dp[i]-dp[i-1]-dh)>dh*1.e-10)
		  { cerr << "Ice table does not use uniform depth spacing, falling back to bulk ice" << endl; return; }
	sca[i]=be[i]*l_a;
	abs[i]=(D*ba[i]+E)*l_k+ABl*(1+0.01*td[i]);
	if(sca[i]<=0 || abs[i]<=0){ cerr << "Invalid value of ice parameter, falling back to bulk ice" << endl; return; }
	if(1450<dp[i]+dh/2 && dp[i]-dh/2<2450){ sca_a+=sca[i]; abs_a+=abs[i]; num++; }
      }

      if(num<1){ cerr << "No layers defined inside the detector, falling back to bulk ice" << endl; return; }
      sca_a=num/sca_a;
      abs_a=num/abs_a;

      hmin=dp[0]-dh/2;
      hmax=dp[size-1]+dh/2;

      abs_sum = new double[size];
      sca_sum = new double[size];
      double abs_aux=0, sca_aux=0;

      for(int i=0; i<size; i++){
	abs_aux+=abs[i];
	sca_aux+=sca[i];
	abs_sum[i]=abs_aux;
	sca_sum[i]=sca_aux;
      }

      log_info_stream("Using 6-parameter ice model at l=" << wva << " nm: np=" << np << " ng=" << ng<<'\n'
	   << "average sca=" << sca_a << " abs=" << abs_a << " depths=(" << hmin << ";" << hmax << ")");

      kurt=true;
    }

    ~Ice(){
      if(kurt){
	delete abs;
	delete sca;
	delete abs_sum;
	delete sca_sum;
      }
    }

    // sets ice parameters for specified depth
    void lx(double x, double y, double z){
      z=zoff-(z-tilt.zshift(x, y, z));

      if(kurt){
	int i=(int) floor((z-hmin)/dh); if(i<0) i=0; else if(i>=size) i=size-1;
	this->le=1/sca[i];
	this->la=1/abs[i];
      }
    }

    // calculates average ice parameters between given depths using prescription by G. Japaridze and M. Ribordy
    void ll(double x1, double y1, double z1, double x2, double y2, double z2){
      z1=zoff-(z1-tilt.zshift(x1, y1, z1)), z2=zoff-(z2-tilt.zshift(x2, y2, z2));

      if(kurt){
	double zz, rsca, rabs;
	if(z2<z1){ zz=z1; z1=z2; z2=zz; }
	int i=(int) floor((z1-hmin)/dh); if(i<0) i=0; else if(i>=size) i=size-1;
	if(z1==z2){ rsca=sca[i]; rabs=abs[i]; }
	else{
	  int f=(int) floor((z2-hmin)/dh); if(f<0) f=0; else if(f>=size) f=size-1;
	  rsca=((sca_sum[f]-sca_sum[i])*dh-((hmin+(f+1)*dh)-z2)*sca[f]+((hmin+(i+1)*dh)-z1)*sca[i])/(z2-z1);
	  rabs=((abs_sum[f]-abs_sum[i])*dh-((hmin+(f+1)*dh)-z2)*abs[f]+((hmin+(i+1)*dh)-z1)*abs[i])/(z2-z1);
	}
	this->le=1/rsca;
	this->la=1/rabs;
      }
    }

    void set(double le, double la){
      this->le=le, this->la=la;
    }

    double flux(double d, double coseta, bool type){
      static const double dlam=1.07;  // fitted propagation length
      static const double dnrm=1.26;  // and normalization corrections

      if(d<dr) d=dr;
      double p, lp=sqrt(le*la/3)/dlam, lc=exp(le/la)*le/3;
      if(type){
	double lu=lc/sinchr; lu*=lu*2/(M_PI*lp);
	p=exp(-d/lp)/(2*M_PI*sinchr*sqrt(lu*d)*tanh(sqrt(d/lu)));
      }
      else p=exp(-d/lp)/(4*M_PI*lc*d*tanh(d/lc));
      check(p*=dnrm*asens.s(coseta, sinh(d/le)));
      return p;
    }

    double aprof(double cospsi, double dd){
      double f=exp(-5.40*pow(fabs(cospsi-coschr), 0.35))/0.06667; // M. de Jong
      double y=sinh(dd/le);
      return isfinite(y)?(f+y)/(1+y):1;
    }
  } ice;

  static const double SQRT2PI=M_SQRT2*M_SQRTPI;

  struct cpdf{  // holds the pdf parameterization
    cpdf(double ita, double sig, double ef, double es, double af, double as){
      this->ita=ita, this->sig=sig;
      this->ef=ef, this->es=es, this->af=af, this->as=as;
    }

    double lnscat(double d, double t, double Q){
      set(d);
      return Q>0 ? addn(npandel(t, Q), t) : addn(cpandel(t), t);
    }

    void relax(){ sig=sig*10; }
    void restore(){ sig=sig/10; }

  private:
    double ita;  // cpandel 1/tau
    double sig;  // cpandel jitter
    double ef, es;   // multiplicative factor: pandel lambda/eff scattering length=ef*le+es
    double af, as;   // multiplicative factor: pandel absorption/absorption length=af*la+as
    double xi, rho;  // common pandel parameters

    double gaus(double x, double sig){
      x/=sig; return exp(-x*x/2)/(SQRT2PI*sig);
    }

    double student(double x, double sig){
      x/=sig; return 1/(M_PI*(1+x*x)*sig);
    }

    double addn(double x, double t){
      return log(x*(1-umin)+umin*student(t, dura));
    }

    void set(double d){  // must be called before pandel functions
      double le=ef*ice.le+es, la=af*ice.la+as;
      xi=d/le, rho=ita+ice.cm/la;
    }

    double pandel(double t){
      if(xi<=0 || t<=0) return 0;
      else{
	double trho=t*rho;
	return rho*exp((xi-1)*log(trho)-trho-gsl_sf_lngamma(xi));
      }
    }

    double cpandel(double t){  // cpandel parameterization according to G. Japaridze
      double result;

      if(sig<=0) result=pandel(t);
      else if(xi>30 || t<-25*sig || t>3500) result=0;
      else if(xi<=0.1*rho*sig){  // Zero distance
	result=gaus(t, sig);
      }
      /*
      else if(t<rho*sig*sig){  // CHF of the Second kind solution for eta>0
	double eta=rho*sig-t/sig;
	result=pow(rho*sig, xi)*exp(-t*t/(2*sig*sig))/(M_SQRTPI*sig)*
	  gsl_sf_hyperg_U(xi/2, 0.5, eta*eta/2)/pow(2, (xi+1)/2);
      }
      */
      else if(xi<5 && -5*sig<t && t<30*sig){  // Region 1
	double eta=rho*sig-t/sig;
	double eta2=eta*eta/2, xi1=(xi+1)/2, xi2=xi/2;
	double f1=gsl_sf_hyperg_1F1(xi2, 0.5, eta2)/gsl_sf_gamma(xi1);
	double f2=M_SQRT2*eta*gsl_sf_hyperg_1F1(xi1, 1.5, eta2)/gsl_sf_gamma(xi2);
	const double xx=1.e-8;
	if(fabs(f1-f2)<fabs(f1+f2)*xx) result=0;
	else result=rho*pow(rho*sig, xi-1)*exp(-t*t/(2*sig*sig))/pow(2, xi1)*(f1-f2);
      }
      else if(xi<1){
	if(t>0){  // Region 2
	  double trho=t*rho, rhosig=rho*sig;
	  result=exp(rhosig*rhosig/2)*
	    (rho*exp((xi-1)*log(trho)-trho-gsl_sf_lngamma(xi)));
	}
	else{  // Region 5
	  result=pow(1-t/(rho*sig*sig), -xi)*gaus(t, sig);
	}
      }
      else{
	double eta=rho*sig-t/sig, xi2=2*xi-1;
	double z=fabs(eta)/sqrt(4*xi-2);
	double sqrz=sqrt(1+z*z);
	double k=0.5*(z*sqrz+log(z+sqrz));

	double b=0.5*(z/sqrz-1);
	double b2=b*b, b12=b/12;
	double N1=b12*(20*b2+30*b+9);
	double N2=0.5*b12*b12*(6160*b2*b2+18480*b2*b+19404*b2+8028*b+945);

	if(eta<0){  // Region 3
	  double Phi=1-N1/xi2+N2/(xi2*xi2);
	  double alpha=-0.5*(t*t/(sig*sig)+xi+log(sqrz))+0.25*(eta*eta+1)+
	    k*xi2-xi*M_LN2/2+(xi-1)/2*log(xi2)+xi*log(rho)+(xi-1)*log(sig);
	  result=exp(alpha)*Phi/gsl_sf_gamma(xi);
	}
	else{  // Region 4
	  double Psi=1+N1/xi2+N2/(xi2*xi2);
	  result=rho*pow(rho*sig, xi-1)*
	    exp(-t*t/(2*sig*sig)+0.25*(eta*eta-1)+xi/2-k*xi2)*
	    Psi*pow(2, (xi-1)/2)/(pow(xi2, xi/2)*SQRT2PI*sqrt(sqrz));
	}
      }
      check(result); return result;
    }

    double cf(double t){
      if(xi<=0) return t<0?1:0;
      return t>0?gsl_sf_gamma_inc_Q(xi, rho*t):1;
    }

    double dcf(double t, double s){
      if(s<0.1*t) return pandel(t);
      else return (cf(t-s/2)-cf(t+s/2))/s;
    }

    double dcf(double t, double s, double Q){
      if(s<0.1*t) return Q*pandel(t)*pow(cf(t), Q-1);
      else return (pow(cf(t-s/2), Q)-pow(cf(t+s/2), Q))/s;
    }

    double npandel(double t, double Q){
      if(Q>1){  // convoluted MPE pandel by DIMA
	if(sig<=0) return Q*cpandel(t)*pow(cf(t), Q-1);
	else if(xi<=0.1*rho*sig) return gaus(t, sig);
	else{
	  double s=2*M_SQRT3*sig;
	  double sum=dcf(t, s, Q);
	  if(xi<1){  // less precise but faster
	    double r=rho;
	    rho=r*pow(Q, 1/xi);
	    sum+=cpandel(t)-dcf(t, s);
	    rho=r;
	  }
	  check(sum); return sum;
	}
      }
      else return cpandel(t);
    }
  } pdf(0, 15, 1.0, 35., 1.0, 100);

#include "miniball.h"

  void normalize(double n[3]){
    double r=0;
    for(int i=0; i<3; i++) r+=n[i]*n[i];
    if(r>0){
      r=1/sqrt(r);
      for(int i=0; i<3; i++) n[i]*=r;
    }
    else n[2]=1;
  }

  double flle(double, void *);
  double fllh(const gsl_vector *, void *);

  struct line{
    double nx, ny, nz, x0, y0, z0, t0;

    line():nx(0), ny(0), nz(0), x0(0), y0(0), z0(0), t0(0){ }

    line(double n[3], double x, double y, double z, double t){
      x0=x, y0=y, z0=z, t0=t;
      nx=n[0], ny=n[1], nz=n[2];
    }

    line(double th, double ph, double x, double y, double z, double t){
      x0=x, y0=y, z0=z, t0=t;

      const double cf=M_PI/180;
      th*=cf, ph*=cf;
      double sinth=sin(th), costh=cos(th);
      double sinph=sin(ph), cosph=cos(ph);

      nx=sinth*cosph;
      ny=sinth*sinph;
      nz=costh;
    }
  };

  struct vtemp:line{
    bool type, en, mpe;
    unsigned int lcsp, unc;

    double nn, na;

    double xc, yc, zc, tc, dc, rn, qtot, norm;

    struct hit{
      double t, q;

      hit(double t, double q){
	this->t=t, this->q=q;
      }

      bool operator< (const hit& rhs) const {
	return t < rhs.t || (t == rhs.t && (q < rhs.q));
      }
    };

    struct set{
      double x, y, z, q, eff;

      set(const mydom& dom) : q(0.){
	x=dom.x, y=dom.y, z=dom.z; eff=dom.eff;
      }

      double rn, dd, tdel, coseta, le, la;

      void loc() const {
	ice.set(le, la);
      }

      double deff() const {
	return dd;
      }
    };

    struct sett:set{
      vector<hit> h;
      double t, ave;
      bool lc;

      sett(const mydom& dom):set(dom), ave(0.), lc(false){ }
    };

    struct setq:set{
      double p;
      bool sat;

      setq(const set& dom, bool sat):set(dom){ this->sat=sat; }
      setq(const mydom& dom):set(dom), sat(false){ }
    };

    map<mykey, sett> hits;
    map<mykey, setq> hnhs; // hit/no-hits
 
    double step;
    double ares, dres;

    bool useall;
    unsigned int nhits, hitdoms, strings;

    rrt rt;

    vtemp(I3RecoPulseSeriesMapConstPtr pmap, bool type, unsigned int lcsp, unsigned int unc, bool mpe, bool en, gsl_rng * G = NULL) : step(15.), ares(0.002), dres(1.0){
      rt.ini(G);
      this->type=type; this->lcsp=lcsp; this->unc=unc; this->mpe=mpe; this->en=en;

      qtot=0;
      vector<double> R;

      if(G!=NULL) for(I3RecoPulseSeriesMap::const_iterator i = pmap->begin(); i != pmap->end(); ++i){
	  const OMKey& key = i->first;
	  // int om = key.GetOM(), str=key.GetString();
	  const vector<I3RecoPulse> pulses = i->second;
	  
	  for(vector<I3RecoPulse>::const_iterator j = pulses.begin(); j != pulses.end(); ++j){
	    double q=j->GetCharge();
	    qtot+=q; R.push_back(q);
	  }
	}

      int K=R.size();
      std::vector<unsigned int> N(K);

      if(G!=NULL){
	std::vector<double> P(K);

	for(int i=0; i<K; i++) P[i]=R[i]/qtot;
	if((round(qtot) > UINT_MAX) || (round(qtot) < 0)){
		log_fatal("double (%e) cannot be cast to an unsigned integer!", round(qtot));
	}
	else{
		gsl_ran_multinomial(G, K, max(1UL, (unsigned long int) round(qtot)), &P.front(), &N.front());
	}

	K=0;
      }

      for(I3RecoPulseSeriesMap::const_iterator i = pmap->begin(); i != pmap->end(); ++i){
	const OMKey& omkey = i->first;


	int om = omkey.GetOM(), str=omkey.GetString();
	const mykey key(str, om);
	const mydom& dom = doms[key];
	const vector<I3RecoPulse> pulses = i->second;
	
	for(vector<I3RecoPulse>::const_iterator j = pulses.begin(); j != pulses.end(); ++j){
	  double q=j->GetCharge();
	  if(G!=NULL){ q=N[K++]; if(q<=0) continue; }
	  {
	    map<mykey, sett>::iterator it=hits.find(key);
	    if(it==hits.end()) hits.insert(make_pair(key, sett(dom)));
	    sett & dom = hits.find(key)->second;
	    vector<hit> & h = dom.h;
	    
	    double t=j->GetTime()/I3Units::ns;
	    
	    h.push_back(hit(t, q));
	    dom.q+=q; dom.ave+=q*t;
	  }
	  
	}
      }

      nhits=0, qtot=0;
      tc=0, xc=0, yc=0, zc=0;

      int nc=0;
      double qc=0;

      map<int, int> strs;

      for(map<mykey, sett>::iterator i=hits.begin(); i!=hits.end(); ++i){
	sett & dom = i->second;
	vector<hit> & h = dom.h;

	const mykey & key = i->first;
	nhits+=h.size(); strs[key.str]++;

	const double pre=1.e-4; // pre-pulse probability
	sort(h.begin(), h.end());

	double Q=dom.q;
	qtot+=Q; dom.ave/=Q;
	dom.t=h.front().t;
	double q=0, qpre=Q*pre;
	for(vector<hit>::const_iterator j=h.begin(); j!=h.end(); j++){
	  q+=j->q; if(q>qpre) break;
	  dom.t=j->t;
	}

	if(mpe){
	  h.clear();
	  h.push_back(hit(dom.t, Q));
	}

	{
	  int use=0;
	  const int lcsp=2;
	  {
	    map<mykey, sett>::const_iterator j(i); j++;
	    if(j!=hits.end() && j->first.str==key.str && j->first.om-key.om<=lcsp) use++;
	  }
	  {
	    map<mykey, sett>::const_reverse_iterator j(i);
	    if(j!=hits.rend() && j->first.str==key.str && key.om-j->first.om<=lcsp) use++;
	  }

	  if(use>0) dom.lc=true, nc++;
	}
      }

      useall=nc<50;
      for(map<mykey, sett>::iterator i=hits.begin(); i!=hits.end(); ++i){
	sett & dom = i->second;
	// const mykey & key = i->first;
	double Q=dom.q;
	if(compat){ Q=round(Q); if(Q<=0) Q=1; dom.q=Q; }
	if(useall || dom.lc) qc+=Q, tc+=Q*dom.ave, xc+=Q*dom.x, yc+=Q*dom.y, zc+=Q*dom.z;
      }

      strings=strs.size(); hitdoms=hits.size();

      norm=mpe?hitdoms:qtot;
      if(qc>0) tc/=qc, xc/=qc, yc/=qc, zc/=qc;

      if(en && hitdoms>0){
	Miniball<3> ball;

	for(map<mykey, sett>::const_iterator i=hits.begin(); i!=hits.end(); ++i){
	  const sett & dom = i->second;

	  if(useall || dom.lc){
	    const mykey & key = i->first;

	    Point<3> p;
	    p[0]=dom.x, p[1]=dom.y, p[2]=dom.z;
	    ball.check_in(p);

	    const mydom& cdom = doms[key];
	    hnhs.insert(make_pair(key, setq(dom, dom.q>cdom.st || boms.find(key)!=boms.end())));
	  }
	}

	ball.build();
	Point<3> c=ball.center();
	double cx=c[0], cy=c[1], cz=c[2];
	double r2=ball.squared_radius();

	for(map<mykey, mydom>::const_iterator i = doms.begin(); i!=doms.end(); ++i){
	  const mykey& key = i->first;
	  const mydom& dom = i->second;
	  if(dom.hv>0){
	    double dx=dom.x-cx, dy=dom.y-cy, dz=dom.z-cz;
	    if(dx*dx+dy*dy+dz*dz<r2){
	      map<mykey, setq>::iterator i=hnhs.find(key);
	      if(i==hnhs.end()) hnhs.insert(make_pair(key, setq(dom)));
	    }
	  }
	}

	mykey back=hnhs.rbegin()->first;
	int str=back.str, om=back.om, pad=1;
	for(map<mykey, setq>::const_iterator i = hnhs.begin(); i!=hnhs.end(); ++i){
	  const mykey& key = i->first;
	  if(key.str==str){
	    for(int i=om+1; i<key.om; i++){
	      const mykey newkey(str, i);
	      const mydom& dom = doms[newkey];
	      if(dom.hv>0) hnhs.insert(make_pair(newkey, setq(dom)));
	    }
	  }
	  else{
	    for(int i=1; i<=pad; i++){
	      const mykey newkey(str, om+i);
	      if(doms.find(newkey)!=doms.end()){
		const mydom& dom = doms[newkey];
		if(dom.hv>0) hnhs.insert(make_pair(newkey, setq(dom)));
	      }
	    }
	    str=key.str;
	    for(int i=1; i<=pad; i++){
	      const mykey newkey(str, om-i);
	      if(doms.find(newkey)!=doms.end()){
		const mydom& dom = doms[newkey];
		if(dom.hv>0) hnhs.insert(make_pair(newkey, setq(dom)));
	      }
	    }
	  }
	  om=key.om;
	}
      }
    }

    void ini(set& dom, bool type = true){
      double dx=dom.x-x0, dy=dom.y-y0, dz=dom.z-z0, xe, ye, ze;
      if(type){
	double rn=nx*dx+ny*dy+nz*dz;
	dx-=rn*nx, dy-=rn*ny, dz-=rn*nz;
	dom.rn=rn-this->rn; xe=dom.x-dx; ye=dom.y-dy; ze=dom.z-dz;
	dom.dd=sqrt(dx*dx+dy*dy+dz*dz);
	dom.tdel=(rn+dom.dd*ice.na)/ice.c;
	rn=max(dr, dom.dd)/ice.tanchr;
	dx+=rn*nx, dy+=rn*ny, dz+=rn*nz;
	dom.coseta=dz/sqrt(dx*dx+dy*dy+dz*dz);
      }
      else{
	dom.rn=0; xe=x0, ye=y0, ze=z0;
	dom.dd=sqrt(dx*dx+dy*dy+dz*dz);
	dom.tdel=dom.dd/ice.cm;
	dom.coseta=dz/max(dr, dom.dd);
      }

      ice.ll(dom.x, dom.y, dom.z, xe, ye, ze); dom.le=ice.le, dom.la=ice.la;
    }

    // time log likelihood function
    double illt(bool reset = true, bool all=true){
      double result=0;

      for(map<mykey, sett>::iterator i=hits.begin(); i!=hits.end(); ++i){
	sett & dom = i->second;
	if(all || useall || dom.lc){
	  if(reset) ini(dom); dom.loc();
	  const vector<hit> & h = dom.h;

	  for(vector<hit>::const_iterator j=h.begin(); j!=h.end(); ++j){
	    double q=j->q;
	    double f=-pdf.lnscat(dom.deff(), j->t-(t0+dom.tdel), mpe?q:0);
	    result += mpe ? f : q*f;
	  }
	}
      }

      return result;
    }

    void set_t0(double n[]){
      set_track(line(n, x0, y0, z0, t0));

      vector<double> all;
      for(map<mykey, sett>::iterator i=hits.begin(); i!=hits.end(); ++i){
	sett& dom=i->second;
	ini(dom);
	all.push_back(dom.t-dom.tdel);
      }

      sort(all.begin(), all.end());
      int s=all.size(), m=0;
      if(s>1){
	double dt=all[1]-all[0];
	for(int i=1; i<s-1; i++){
	  double dti=all[i+1]-all[i];
	  if(dti<dt) dt=dti, m=i;
	}
	m++;
      }

      t0=all[m];
    }

    double llh(double n[]){
      set_t0(n);
      return illt(false, false);
    }

    void fastini(){
      bool verbose=false;
      vert best;
      pdf.relax();

      {
	int imin=0;
	double fmin=0;

	for(int i=0; i<ico.vn; i++){
	  double * n = ico.v[i].n;
	  double ff=llh(rt.rot(n));
	  if(i==0 || ff<fmin) imin=i, fmin=ff;
	}
	best=vert(rt.rot(ico.v[imin].n), fmin);

	if(verbose) cout<<"best("<<ico.vn<<"): "<<best<<endl;
      }

      {
	double n[3]={0};
	for(map<mykey, sett>::iterator i=hits.begin(); i!=hits.end(); ++i){
	  sett& dom=i->second;
	  if(useall || dom.lc){
	    double qt=dom.q*dom.t;
	    n[0]+=qt*(dom.x-xc);
	    n[1]+=qt*(dom.y-yc);
	    n[2]+=qt*(dom.z-zc);
	  }
	}
	normalize(n);
	double ff=llh(n);
	if(ff<best.f) best=vert(n, ff);
      }

      set_t0(best.n);
      pdf.restore();
    }

    void recoini(){
      x0=xc, y0=yc, z0=zc, t0=tc;
      nx=0, ny=0, nz=1; rn=0, dc=0;
    }

    void advance(double r){
      x0+=r*nx, y0+=r*ny, z0+=r*nz, t0+=r/ice.c; rn-=r;
    }

    int rmode;

    double llh(){
      switch(rmode){
      case 1:
	{
	  return ille(false);
	}
	break;
      case 2:
	{
	  return illt();
	}
	break;
      default:
	{
	  return illt();
	}
      }
    }

    void set_gsl(const gsl_vector * x){
      switch(rmode){
      case 1:
	{
	  double n[3]={nx, ny, nz};
	  double dx=gsl_vector_get(x, 0)*dres, dy=gsl_vector_get(x, 1)*dres, dz=gsl_vector_get(x, 2)*dres;
	  set_track(line(n, xc+dx, yc+dy, zc+dz, t0), false);
	}
	break;
      case 2:
	{
	  double * n = rt.ste(gsl_vector_get(x, 0)*ares, gsl_vector_get(x, 1)*ares);
	  double dt=gsl_vector_get(x, 2)*dres;
	  set_track(line(n, x0, y0, z0, tc+dt), false);
	}
	break;
      default:
	{
	  double * n = rt.ste(gsl_vector_get(x, 0)*ares, gsl_vector_get(x, 1)*ares);
	  double dx=gsl_vector_get(x, 2)*dres, dy=gsl_vector_get(x, 3)*dres, dz=gsl_vector_get(x, 4)*dres;
	  set_track(line(n, xc+dx, yc+dy, zc+dz, t0));
	}
      }
    }

    double solve(int r){
      switch(rmode=r){
      case 1:
	{
	  const int num=3;
	  double xi[num]={(x0-xc)/dres, (y0-yc)/dres, (z0-zc)/dres};
	  double result=solve(xi, num)/qtot;
	  return result;
	}
	break;
      case 2:
	{
	  const int num=3;
	  double n[3]={nx, ny, nz}; rt.ini(n);
	  double xi[num]={0, 0, (t0-tc)/dres};
	  double result=solve(xi, num)/norm;
	  return result;
	}
	break;
      default:
	{
	  const int num=5;
	  double n[3]={nx, ny, nz}; rt.ini(n);
	  double xi[num]={0, 0, (x0-xc)/dres, (y0-yc)/dres, (z0-zc)/dres};
	  double result=solve(xi, num)/norm; advance(rn);
	  return result;
	}
      }
    }

    double Solve(int ini){
      if(ini>0){
	recoini();
	if(!type) solve(1);
	fastini();
      }
      else{
	if(type) advance(rn);
	else solve(1);
      }
      return solve(type?0:2);
    }

    double solve(double *xi, const double num){
      bool verbose=false;
      double size=50;

      gsl_vector * x = gsl_vector_alloc(num);
      gsl_vector * e = gsl_vector_alloc(num);

      for(int i=0; i<num; i++) gsl_vector_set(x, i, xi[i]);
      gsl_vector_set_all(e, size);

      gsl_multimin_function F;
      F.n = num;
      F.f = fllh;
      F.params = this;

      const gsl_multimin_fminimizer_type * T = gsl_multimin_fminimizer_nmsimplex;
      gsl_multimin_fminimizer * s = gsl_multimin_fminimizer_alloc(T, num);
      gsl_multimin_fminimizer_set(s, &F, x, e);

      int iter=0, max_iter=250;

      for(; iter<max_iter; iter++){
	if(GSL_SUCCESS!=gsl_multimin_fminimizer_iterate(s)){
	  if(verbose) printf("Failed to iterate\n");
	  break;
	}

	size=gsl_multimin_fminimizer_size(s);
	if(verbose){
	  cout<<iter<<" "<<s->fval/norm<<" "<<size;
	  for(int i=0; i<num; i++) cout<<" "<<gsl_vector_get(s->x, i); cout<<endl;
	}

	if(GSL_SUCCESS==gsl_multimin_test_size(size, 0.001)){
	  if(verbose) printf("Converged!\n");
	  break;
	}
      }

      set_gsl(s->x);
      double result=s->fval;
      if(verbose) cout<<"fval="<<result<<endl;

      gsl_multimin_fminimizer_free(s);
      gsl_vector_free(e);
      gsl_vector_free(x);
      return result;
    }

    double rllt(){
      return illt()/norm;
    }

    struct XL{
      int nu;
      double sig, s2, b, w, W0, zx, D;

      void ini(bool type){
	if(compat){ sig=1.; nu=0; w=0; D=0; }
	else{ sig=1.; nu=1; w=type?10.:0.; D=10; }

	s2=sig*sig;
	if(nu>0){
	  s2*=nu; b=(nu+1)/2.;
	  W0=w>0?exp(gsl_sf_lambert_W0(w*w*s2/2)/w):1;
	  zx=log(W0); zx=exp(-w*zx)+zx*zx/s2;
	}
      }

      double q, mx, x, y, z;

      double gfx(double x){
	y=log(x/mx); z=exp(-w*y);
	return (x-q)*(y*y+s2*z)+b*(2*y-w*s2*z);
      }

      double dif(double x){
	return (q>0?q*log(x/q):0)-(x-q);
      }

      double Ld(double x){
	if(D>0){
	  double y=x+D;
	  if(y<0){
	    x=y; y/=D;
	    x=x/sqrt(1+y*y)-D;
	  }
	}
	return x;
      }

      double f(double mu, double Q){
	if(compat){ q=Q; mu-=umin-0.01; }
	else q = round(Q);

	double F;
	if(sig>0){
	  if(nu>0){
	    mx=mu/W0, x=mx, y=0, z=1;
	    {
	      double xi=q>0?min(mu, q):mx*exp(-mx*(s2*(w>1?pow(mx/(b*w), w-1):1)+1)/(2*b)), xf=max(mu, q);
	      double gi=gfx(xi), gf=gfx(xf);
	      if(gi*gf>0) cerr<<"Error: "<<gi<<" "<<gf<<"  "<<xi<<" "<<xf<<endl;

	      do{
		double g=gfx(x=(xi+xf)/2);
		if(g*gi>0) xi=x, gi=g; else xf=x, gf=g;
	      } while(xf-xi>1);

	      while(true){
		double y2=y*y+s2*z, xq=x-q;
		double g=xq*y2+b*(2*y-w*z*s2);
		double dg=x*y2+2*(y*xq+b)+w*z*s2*(b*w-xq);
		double dy=-g/dg; y+=dy;
		if(g*gi>0) xi=x, gi=g; else xf=x, gf=g;
		double xa=mx*exp(y);

		if(xa<xi || xa>xf){
		  x=(xi+xf)/2;
		  y=log(x/mx); z=exp(-w*y);
		  if(xf-xi<1.e-5) break;
		}
		else{
		  x=xa; z=exp(-w*y);
		  if(fabs(x*dy)<1.e-5) break;
		}
	      }
	    }
	    F=dif(x)-b*log((z+y*y/s2)/zx);
	  }
	  else{
	    double val=s2*q;
	    if(val<50){
	      x=gsl_sf_lambert_W0(s2*mu*exp(val))/s2;
	      y=sig*(q-x);
	      F=dif(x)-y*y/2;
	      if(compat) F-=log(1+x*s2)/2;
	    }
	    else{
	      z=sig*sqrt(1+1/val);
	      y=log(q/mu)/z;
	      F=-y*y/2;
	    }
	  }
	}
	else F=dif(mu);

	const double stop=1.e6;
	if(!isfinite(F)) F=-q*stop;

	return Ld(F);
      }
    } xl;

    double lle(double n){
      double result=0;
      for(map<mykey, setq>::const_iterator i = hnhs.begin(); i!=hnhs.end(); ++i){
	const mykey & key = i->first;
	const setq & dom = i->second;
	if(dom.sat) continue;
	double p=dom.p;
	if(lcsp>0){
	  double p_ud=0;
	  int str=key.str;
          unsigned int om=key.om;
	  {
	    map<mykey, setq>::const_iterator j(i); j++;
	    for(; j!=hnhs.end() && j->first.str==str && j->first.om-om<=lcsp; ++j) p_ud+=j->second.p;
	  }
	  {
	    map<mykey, setq>::const_reverse_iterator j(i);
	    for(; j!=hnhs.rend() && j->first.str==str && om-j->first.om<=lcsp; ++j) p_ud+=j->second.p;
	  }
	  if(p_ud>0){
	    p_ud*=n;
	    p*=p_ud<1.e-5?p_ud:1-exp(-p_ud);
	  }
	}
	result+=xl.f(n*p+umin, dom.q);
      }
      return -result;
    }

    I3VectorI3Particle elist;
    I3Particle muon;

#include "nnls.cxx"
    double dlle(bool flag){
      if(!en) return 0;
      double result=0;
      vector<double> q;
      vector<vector<double> > p;

      const double dtrs=50;  // max. distance to the closest DOM at ends
      const double dpad=100; // padding to the segment of track with hits

      double dmin=0, dmax=0; // fit end points

      for(map<mykey, setq>::iterator i = hnhs.begin(); i!=hnhs.end(); ++i){
	setq& dom=i->second;

	if(flag){
	  ini(dom);
	  dom.p=ice.flux(dom.deff(), dom.coseta, true)*dom.eff*efcr*exp(-brho*dom.rn);
	}

	if(dom.q>0 && dom.dd<dtrs){
	  if(dom.rn<dmin) dmin=dom.rn;
	  else if(dom.rn>dmax) dmax=dom.rn;
	}
      }

      int jmin=(int) round((dmin-dpad)/step);
      int jmax=(int) round((dmax+dpad)/step);
      int jnum=jmax-jmin+1;

      for(map<mykey, setq>::iterator i = hnhs.begin(); i!=hnhs.end(); ++i){
	setq& dom=i->second;
	if(dom.sat) continue;
	vector<double> aux;

	for(int j=jmin; j<=jmax; j++){
	  double rn=this->rn+step*j;
	  double dx=dom.x-x0, dy=dom.y-y0, dz=dom.z-z0;
	  dx-=rn*nx, dy-=rn*ny, dz-=rn*nz;
	  dom.dd=sqrt(dx*dx+dy*dy+dz*dz);
	  dom.coseta=dz/max(dr, dom.dd);
	  double xe=dom.x-dx, ye=dom.y-dy, ze=dom.z-dz;
	  ice.ll(dom.x, dom.y, dom.z, xe, ye, ze);
	  double cospsi=(nx*dx+ny*dy+nz*dz)/max(dr, dom.dd);
	  double p=ice.flux(dom.deff(), dom.coseta, false)*dom.eff*efcr;
	  p*=ice.aprof(cospsi, dom.dd);
	  aux.push_back(p);
	}

	aux.push_back(dom.p/step);

	p.push_back(aux);
	q.push_back(dom.q);
      }

      if(!p.empty()){
	const int n=jnum+1, m=q.size();
	int mda=m, nsetp, index[n];
	double * A = new double[m*n], B[m], rnorm, w[n], zz[m], x[n];
	for(int i=0; i<m; i++){
	  B[i]=q[i]-umin;
	  for(int j=0; j<n; j++) A[i+j*mda]=p[i][j]<pmin?0:p[i][j];
	}
	nnls(A, mda, m, n, B, x, rnorm, w, zz, index, nsetp);
	delete [] A;

	xl.ini(false);

	double qtot=0, qrec=0;
	for(int i=0; i<m; i++){
	  double Q=q[i], mu=0;
	  for(int j=0; j<n; j++) mu+=p[i][j]*x[j]; mu+=umin;
	  qtot+=Q, qrec+=mu; // result-=xl.f(mu, Q);
	}
	result=qrec/qtot;

	double n0=x[jnum];/*dl=0, dx=jnum*step*/
	for(int j=0; j<jnum; j++){
	  double & xj = x[j];
	  xj+=n0*exp(-brho*step*(j+jmin));
	  xj/=Cf; /*dl+=xj;*/ xj/=Ff;
	}
	/*
	dl-=min(dx, dl/(1+Ff*arho)); dl/=Ff;
	dl=max(dl/(1-exp(-brho*dx))-arho/brho, dl);
	*/

	muon.SetShape(I3Particle::Cascade);
	muon.SetLength(0*I3Units::m);
	for(int j=0; j<jnum; j++) if(x[j]>0){
	    setParticle(rn+step*(j+jmin), x[j]);
	    elist.push_back(muon);
	  }
      }

      return result;
    }

    // calculates energy likelihood
    double ille(bool type){
      if(!en) return nn=na=0;
      double result=0, sum=0, num=0;

      for(map<mykey, setq>::iterator i = hnhs.begin(); i!=hnhs.end(); ++i){
	setq& dom=i->second;
	ini(dom, type);
	dom.p=ice.flux(dom.deff(), dom.coseta, type)*dom.eff*efcr*exp(-brho*dom.rn);
	if(!dom.sat) sum+=dom.q, num+=max(pmin, dom.p);
      }

      if(num>0) sum/=num; na=sum;

      {
	xl.ini(type);

	bool verbose=false;
	int iter=0, max_iter=100;
	const gsl_min_fminimizer_type *T;
	gsl_min_fminimizer *s;
	gsl_function F;

	const double step=2;
	double a=sum/step, m=sum, b=sum*step;
	double fa=lle(a), fm=lle(m), fb=lle(b);

	for(int i=0; i<10; ){
	  if(verbose) printf("     a(%g)=%g  m(%g)=%g  b(%g)=%g\n", a, fa, m, fm, b, fb);
	  if(fa<fm){
	    b=m, fb=fm; i=0;
	    m=a, fm=fa;
	    fa=lle(a/=step);
	  }
	  else if(fb<fm){
	    a=m, fa=fm; i=0;
	    m=b, fm=fb;
	    fb=lle(b*=step);
	  }
	  else if(fa==fm) fa=lle(a/=step), i++;
	  else if(fb==fm) fb=lle(b*=step), i++;
	  else break;

	  if(verbose) printf("     a(%g)=%g  m(%g)=%g  b(%g)=%g\n", a, fa, m, fm, b, fb);
	  if(fb<fm){
	    a=m, fa=fm; i=0;
	    m=b, fm=fb;
	    fb=lle(b*=step);
	  }
	  else if(fa<fm){
	    b=m, fb=fm; i=0;
	    m=a, fm=fa;
	    fa=lle(a/=step);
	  }
	  else if(fb==fm) fb=lle(b*=step), i++;
	  else if(fa==fm) fa=lle(a/=step), i++;
	  else break;
	}

	F.function=&flle;
	F.params=this;

	T=gsl_min_fminimizer_brent;
	s=gsl_min_fminimizer_alloc(T);
	gsl_min_fminimizer_set(s, &F, m, a, b);

	for(; iter<max_iter; iter++){
	  if(GSL_SUCCESS!=gsl_min_fminimizer_iterate(s)){
	    if(verbose) printf("Failed to iterate\n");
	    break;
	  }

	  m=gsl_min_fminimizer_x_minimum(s);
	  a=gsl_min_fminimizer_x_lower(s);
	  b=gsl_min_fminimizer_x_upper(s);

	  fm=gsl_min_fminimizer_f_minimum(s);
	  fa=gsl_min_fminimizer_f_lower(s);
	  fb=gsl_min_fminimizer_f_upper(s);


	  if(verbose) printf("%5d [%g, %g] %g %g      [%g, %g] %g\n", iter, a, b, m, b-a, fa, fb, fm);

	  if(GSL_SUCCESS==gsl_min_test_interval(a, b, 0.01, 0.01) ||
	     (GSL_SUCCESS==gsl_min_test_interval(fm, fa, 0.05, 0) &&
	      GSL_SUCCESS==gsl_min_test_interval(fm, fb, 0.05, 0))){
	    if(verbose) printf("Converged!\n");
	    break;
	  }
	}
	if(verbose) if(iter==max_iter) printf("NOT Converged!\n");

	gsl_min_fminimizer_free(s);
	nn=m; result=fm;
      }

      {
	double energy = nn/(Cf*Ff);

	if(type){
	  energy/=brho; // for muons works for E >> 1342 GeV
	  muon.SetLength(NAN);
	}

	setParticle(rn, energy);
      }
      return result;
    }

    void setParticle(double shift, double energy){
      muon.SetTime((t0+shift/ice.c)*I3Units::ns);
      muon.SetPos(I3Position((x0+shift*nx)*I3Units::m,
			     (y0+shift*ny)*I3Units::m,
			     (z0+shift*nz)*I3Units::m));
      muon.SetEnergy(energy*I3Units::GeV);
    }

    double rlle(bool type){
      return ille(type)/hitdoms;
    }

    void set_track(const line& l, bool track = true){
      x0=l.x0, y0=l.y0, z0=l.z0, t0=l.t0;
      nx=l.nx, ny=l.ny, nz=l.nz;

      double dx=xc-x0, dy=yc-y0, dz=zc-z0;
      if(track){
	rn=nx*dx+ny*dy+nz*dz;
	tc=t0+rn/ice.c;
	dx-=rn*nx, dy-=rn*ny, dz-=rn*nz;
      }
      else rn=0;
      dc=sqrt(dx*dx+dy*dy+dz*dz);
    }

    bool set_track(const I3ParticleConstPtr& track){
      muon = * track;

      t0=track->GetTime()/I3Units::ns;
      x0=track->GetX()/I3Units::m;
      y0=track->GetY()/I3Units::m;
      z0=track->GetZ()/I3Units::m;

      nx=track->GetDir().GetX();
      ny=track->GetDir().GetY();
      nz=track->GetDir().GetZ();

      double dx=xc-x0, dy=yc-y0, dz=zc-z0;
      rn=nx*dx+ny*dy+nz*dz;

      return isfinite(t0+x0+y0+z0+nx+ny+nz);
    }

    // performs reconstruction
    bool reconstruct(I3RecoPulseSeriesMapConstPtr pmap, I3FramePtr frame, std::string result){
      if(hits.empty()) return false;
      vtemp & v = *this;
      double fv=v.Solve(2);

      double sigma=M_PI;
      double ne=0;
      vector<double> ns;

      if(unc>0){
	vector<line> ls;
	int steps=unc;

	gsl_rng * G = gsl_rng_alloc(gsl_rng_default);

	for(int m=0; m<steps; m++){
	  vtemp u(pmap, type, lcsp, unc, mpe, en, G);

	  u.Solve(1); u.ille(type);
	  u.advance((v.t0-u.t0)*ice.c);
	  ls.push_back(u); ns.push_back(u.nn);
	  // cout<<m<<" "<<u.nx<<" "<<u.ny<<" "<<u.nz<<" "<<u.x0<<" "<<u.y0<<" "<<u.z0<<" "<<u.t0<<" "<<u.nn<<endl;
	}

	gsl_rng_free(G);

	double n[3]={0}, d[3]={0}, t=0;
	for(vector<line>::const_iterator i=ls.begin(); i!=ls.end(); i++){
	  n[0]+=i->nx, n[1]+=i->ny, n[2]+=i->nz;
	  d[0]+=i->x0, d[1]+=i->y0, d[2]+=i->z0; t+=i->t0;
	}
	normalize(n); for(int i=0; i<3; i++) d[i]/=steps; t/=steps;

	{
	  vtemp u(pmap, type, lcsp, unc, mpe, en);
	  u.set_track(line(n, d[0], d[1], d[2], t), type);
	  double fu=u.Solve(0);
	  if(fu<fv) v=u, fv=fu;

	  for(vector<line>::const_iterator i=ls.begin(); i!=ls.end(); i++){
	    u.set_track(*i);
	    double fu=u.Solve(0);
	    if(fu<fv) v=u, fv=fu;
	  }
	}

	vector<double> s;
	for(vector<line>::const_iterator i=ls.begin(); i!=ls.end(); i++) s.push_back(n[0]*i->nx+n[1]*i->ny+n[2]*i->nz);
	sort(s.begin(), s.end());

	sigma=acos((s[steps/2]+s[(steps-1)/2])/2);

	double nn=0;
	for(int i=0; i<steps; i++) nn+=log(ns[i]); nn/=steps;
	for(int i=0; i<steps; i++) ns[i]=fabs(log(ns[i])-nn);
	sort(ns.begin(), ns.end());
	ne=(ns[steps/2]+ns[(steps-1)/2])/2;
      }

      double fe=v.rlle(type);
      if(!en) setParticle(rn, NAN);
      muon.SetFitStatus(I3Particle::OK);
      muon.SetShape(type?I3Particle::InfiniteTrack:I3Particle::Cascade);
      muon.SetDir(I3Direction(v.nx, v.ny, v.nz));

      frame->Put(result+"_rllt", I3DoublePtr(new I3Double(fv)));
      if(en) frame->Put(result+"_rlle", I3DoublePtr(new I3Double(fe)));
      if(unc>0){
	frame->Put(result+"_Sigma", I3DoublePtr(new I3Double(sigma*I3Units::rad)));
	if(en) frame->Put(result+"_EnUnc", I3DoublePtr(new I3Double(ne)));
      }

      return true;
    }
  };

  double flle(double n, void *p){
    return ((vtemp *)p)->lle(n);
  }

  double fllh(const gsl_vector *x, void *p){
    vtemp * v = (vtemp *) p;
    v->set_gsl(x); return v->llh();
  }
}
