#include <cmath>
#include <cstdlib>
#include <vector>

#include <iostream>

#include <gsl/gsl_multifit.h>

using namespace std;

bool verbose=false;
bool girdle=true;
bool orefr=false;
double xx=1.e-10;

float xrnd(){
  return drand48();
}

class myvect{
public:
  double x, y, z;

  myvect() : x(0), y(0), z(0) {}

  myvect(double x, double y, double z){
    this->x=x, this->y=y, this->z=z;
  }

  void set(myvect q){
    x=q.x, y=q.y, z=q.z;
  }

  void divide(double p){
    x/=p, y/=p, z/=p;
  }

  void multiply(double p){
    x*=p, y*=p, z*=p;
  }

  myvect operator* (double p){
    return myvect(x*p, y*p, z*p);
  }

  myvect operator/ (double p){
    return myvect(x/p, y/p, z/p);
  }

  myvect operator+ (myvect q){
    return myvect(x+q.x, y+q.y, z+q.z);
  }

  myvect operator- (myvect q){
    return myvect(x-q.x, y-q.y, z-q.z);
  }

  myvect smallest(){
    myvect temp(fabs(x), fabs(y), fabs(z));
    switch( temp.z<temp.y? temp.z<temp.x?2:0 : temp.y<temp.x?1:0 ){
    case 0: temp=myvect(1, 0, 0); break;
    case 1: temp=myvect(0, 1, 0); break;
    case 2: temp=myvect(0, 0, 1); break;
    }
    return temp;
  }

  myvect cross(myvect q){
    return myvect(y*q.z-q.y*z, z*q.x-q.z*x, x*q.y-q.x*y);
  }

  double dot(myvect q){
    return x*q.x+y*q.y+z*q.z;
  }

  double norm(){
    return sqrt(this->dot(*this));
  }

  void normalize(){
    divide(norm());
  }

  myvect np(myvect q){ // normalized vector perpendicular to this and q (assume |q|=1)
    // q.normalize();
    myvect temp=this->cross(q);
    double size=temp.norm();
    if(size>0) temp.divide(size);
    else{
      temp=this->cross(q.smallest());
      temp.normalize();
    }
    return temp;
  }
};

ostream& operator<<(ostream& o, const myvect& q){
  o << "(" << q.x << "," << q.y << "," << q.z << ")";
  return o;
}

class surface:
  public myvect // normal vector pointing first into second medium
{
public:
  surface() : myvect() {}
  surface(double x, double y, double z) : myvect(x, y, z) {}

  myvect p1, p2;

  void setp(myvect q){ // (assume |*this|=1)
    q.normalize();
    p1=this->np(q);
    p2=this->cross(p1);
    // p2.normalize();
  }

  void setp(){ // (assume |*this|=1)
    p1=this->cross(this->smallest());
    p1.normalize();
    p2=this->cross(p1);
    // p2.normalize();
  }

  void rand(){ // random from sphere
    double ct=2*xrnd()-1;
    double st=sqrt(1-ct*ct);
    double ph=2*M_PI*xrnd();
    double cp=cos(ph), sp=sin(ph);
    x=st*cp, y=st*sp, z=ct;
  }

  // random surface segment of an ellipsoid, representing the average grain
  // a,b,c are the x,y,z radia
  // computes gradients of ellipsoid surface from x^2/a^2+y^2/b^2+z^2/c^2=1 defining equation
  // for directions sampled from sphere and weights according to
  // https://math.stackexchange.com/questions/973101/how-to-generate-points-uniformly-distributed-on-the-surface-of-an-ellipsoid
  void ellipsoid(double a, double b, double c){
    float weight = 0;
    float maxweight = min(a, min(b, c));

    do{
      double costheta=2*xrnd()-1;
      double sintheta=sqrt(1-costheta*costheta);
      double ph=2*M_PI*xrnd();
      double cosphi=cos(ph), sinphi=sin(ph);

      x=sintheta*cosphi/a;
      y=sintheta*sinphi/b;
      z=costheta/c;

      weight=maxweight*sqrt(x*x+y*y+z*z);
    } while(xrnd() >= weight);

    this->normalize();
  }

  void rand_i(myvect q){
    // gets grain boundary plane according to uniform distribution on sphere (ellipsoid optional)
    // dot product between boundary and poynting vector = probability to see plane
    do rand(); while(xrnd() >= fabs(this->dot(q)/q.norm()));
  }

  myvect rand_c(){ // c-axis oriention
    myvect r;
    if(girdle){
      double ph=2*M_PI*xrnd();
      double cp=cos(ph), sp=sin(ph);
      r=p1*cp+p2*sp;
    }
    else{
      surface q;
      q.rand();
      r=q;
    }
    return r;
  }
};

class photon:
  public myvect // coordinates
{
public:
  photon() : myvect() {}
  photon(myvect q) : myvect(q) {}
  photon(double x, double y, double z) : myvect(x, y, z) {}

  double n;
  myvect s; // photon propagation direction
  myvect k; // wave vector
  myvect H; // magnetic field
  myvect E; // electric field
  myvect D; // displacement field and polarization

  void advance(double r){
    set(* this + s * (r/s.norm()));
  }
};

ostream& operator<<(ostream& o, const photon& q){
  o << "n = " << q.n << endl;
  o << "r = " << (myvect) q << endl;
  o << "s = " << q.s << endl;
  o << "k = " << q.k << endl;
  o << "H = " << q.H << endl;
  o << "E = " << q.E << endl;
  o << "D = " << q.D << endl;
  return o;
}

class medium:
  public myvect // optical axis
{
public:
  double no,  ne; // refractive indices
  double beta;

  medium() : myvect() {
    set_n();
  }

  medium(myvect q) : myvect(q) {
    set_n();
  }

  medium(double x, double y, double z) : myvect(x, y, z) {
    set_n();
  }

  void set_n(){
    no=1.3185, ne=1.3200; // at 405 nm
    double aux=ne/no;
    beta=aux*aux-1;
  }

  void set_n(double n){
    no=n, ne=n, beta=0;
  }

  void set_k(myvect q, photon & o, photon & e, bool p = false){ // o: ordinary; e: extraordinary; q=p?s:k
    myvect& axis = *this;

    q.normalize();

    o.D=axis.np(q);
    o.n=no;
    o.k=q;
    o.k.multiply(o.n);

    myvect ort = axis.cross(o.D);

    if(p){
      myvect E=q.cross(o.D);
      e.D=axis*(axis.dot(E)*(ne*ne))+ort*(ort.dot(E)*(no*no));
      e.D.normalize();
      q=o.D.cross(e.D);
    }
    else e.D=q.cross(o.D);

    double cs=axis.dot(q)/no, sn=axis.dot(e.D)/ne;
    e.n=1/sqrt(cs*cs+sn*sn);
    e.k=q;
    e.k.multiply(e.n);

    o.E=o.D/(no*no);
    o.H=o.k.cross(o.E);
    o.s=o.E.cross(o.H);

    e.E=axis*(axis.dot(e.D)/(ne*ne))+ort*(ort.dot(e.D)/(no*no));
    e.H=e.k.cross(e.E);
    e.s=e.E.cross(e.H);

    if(verbose){
      cout<<"\tk="<<o.k<<"\ts="<<o.s<<"\tdot="<<o.k.dot(o.s)*o.n*o.n<<endl;
      cout<<"\tk="<<e.k<<"\ts="<<e.s<<"\tdot="<<e.k.dot(e.s)*e.n*e.n<<endl;
    }
  }

  vector<photon> set_k(myvect k, surface s, bool same){
    vector<photon> result;

    double ky=s.dot(k);
    myvect Y=s;
    if(ky>0){
      ky*=-1;
      Y.multiply(-1);
    }

    myvect X=k-Y*ky;
    double kx=X.norm();
    if(kx>0) X.divide(kx);

    {
      double D=no*no-kx*kx;
      if(-xx<D && D<0) D=0;
      if(D>=0){
	D=sqrt(D);
	double ry=same?D:-D;
	myvect K=X*kx+Y*ry;
	photon O, E;
	set_k(K, O, E);
	result.push_back(O);
      }
    }

    {
      double ax=this->dot(X);
      double ay=this->dot(Y);
      double D=ne*ne*(1+beta*ay*ay)-kx*kx*(1+beta*(ax*ax+ay*ay));
      if(-xx<D && D<0) D=0;
      if(D>=0){
	D=sqrt(D);
	double b=-beta*ax*ay*kx, a=1+beta*ay*ay;
	double r1=(b+D)/a, r2=(b-D)/a;
	double ry=same?r1:r2;
	myvect K=X*kx+Y*ry;
	photon O, E;
	set_k(K, O, E);
	result.push_back(E);
      }
    }

    return result;
  }
};

bool interact(medium one, medium two, surface plane, photon & p){
  if(verbose){
    cout<<"one: "<<one<<endl;
    cout<<"two: "<<two<<endl;
    cout<<"ref: "<<plane<<endl;
  }

  bool same=true;
  if(fabs(p.s.dot(plane))<xx){
    cerr<<"photon collinear to interface surface"<<endl;
    return same;
  }

  vector<photon> reflected=one.set_k(p.k, plane, true);
  vector<photon> refracted=two.set_k(p.k, plane, false);

  int refl=reflected.size();
  int refr=refracted.size();
  int dim=refl+refr, num=6;
  double chisq=0;

  if(dim<=0){
    cerr<<"no dice:"<<endl<<"1 = "<<one<<endl<<"2 = "<<two<<endl<<"i = "<<plane<<endl<<p<<endl;
    return same;
  }

  myvect r(p);
  // plane.setp(); //  nominal  basis
  plane.setp(p.k); // optimized basis
  double q[num][dim], f[num], x[dim], s[dim];

  for(int i=0; i<=dim; i++){
    photon& tmp = i<refl ? reflected[i] : i<dim ? refracted[i-refl] : p;

    if(verbose) cout << i << ": " << tmp << endl;

    f[0]=tmp.D.dot(plane);
    f[1]=tmp.E.dot(plane.p1);
    f[2]=tmp.E.dot(plane.p2);
    f[3]=tmp.H.x, f[4]=tmp.H.y, f[5]=tmp.H.z;

    if(i<dim) for(int j=0; j<num; j++) q[j][i]=(i<refl?-1:1)*f[j];
    s[i]=tmp.s.dot(plane); // norm();
  }

  {
    gsl_matrix * A = gsl_matrix_alloc(num, dim);
    gsl_vector * B = gsl_vector_alloc(num);

    for(int i=0; i<num; i++){
      for(int j=0; j<dim; j++) gsl_matrix_set(A, i, j, q[i][j]);
      gsl_vector_set(B, i, f[i]);
    }

    gsl_multifit_linear_workspace * W = gsl_multifit_linear_alloc(num, dim);
    gsl_vector * X = gsl_vector_alloc(dim);
    gsl_matrix * C = gsl_matrix_alloc(dim, dim);

    gsl_multifit_linear(A, B, X, C, &chisq, W);

    /*
    size_t rank;
    double tol=1.e-8;
    gsl_multifit_linear_svd(A, B, tol, & rank, X, C, &chisq, W); cout<<"rank="<<rank<<endl;
    */

    for(int i=0; i<dim; i++) x[i]=gsl_vector_get(X, i);

    gsl_vector_free(X);
    gsl_matrix_free(C);
    gsl_multifit_linear_free(W);

    gsl_vector_free(B);
    gsl_matrix_free(A);
  }

  double sum=0;
  for(int i=0; i<dim; i++){ s[i]=fabs(x[i]*x[i]*s[i]/s[dim]); sum+=s[i]; }
  if(verbose) cout<<refl<<" "<<refr<<"  "<<chisq<<" "<<sum<<" "<<p.k.dot(plane)<<" "<<p.s.dot(plane)<<endl;

  if(verbose){
    cout<<"x:"; for(int i=0; i<dim; i++) cout<<" "<<x[i]; cout<<endl;
    cout<<"f:"; for(int i=0; i<dim; i++) cout<<" "<<s[i]; cout<<endl;
  }

  if(orefr){
    sum=0;
    for(int i=0; i<refl; i++) s[i]=0;
    for(int i=refl; i<dim; i++) sum+=s[i];
  }

  double y=sum*xrnd();
  sum=0;

  for(int i=0; i<dim; i++){
    sum+=s[i];
    if(y<sum){
      if(i<refl){
	same=true;
	p=reflected[i];
      }
      else{
	same=false;
	p=refracted[i-refl];
      }
      break;
    }
  }

  if(verbose) cout<<endl<<endl;
  p.set(r);
  return same;
}

void test(){
  verbose=true;

  myvect k(0, 0, 1);
  medium one(1, 0, 0), two(0, 1, 0); // two media definitions
  surface plane(0, 0, 1); // interface between two media

  // one.set_n(1), two.set_n(2);

  photon o, e;
  one.set_k(k, o, e, true);
  if(verbose){
    cout << "o:" << endl << o << endl;
    cout << "e:" << endl << e << endl;
  }

  for(int n=0; n<2; n++){
    photon p=n==0?o:e; // incident photon
    interact(one, two, plane, p);
  }
}

void test2(double p, int num, int tot){
  for(int j=0; j<tot; j++){
    myvect k(0, 0, 1);
    medium one, two; // two media definitions
    surface plane; // interface between two media

    surface flow(sin(p), 0, cos(p)); // direction of ice flow
    flow.setp();

    one = flow.rand_c();
    myvect r(0, 0, 0);
    photon o(r), e(r);
    one.set_k(k, o, e, true);
    photon p=xrnd()<0.5?o:e;

    if(verbose) cout<<endl<<endl;
    two = flow.rand_c();

    for(int i=0; i<num; i++){
      p.advance(1./num);
      plane.rand_i(p.s);
      bool same=interact(one, two, plane, p);
      if(!same) one=two;
      two = flow.rand_c();
    }

    myvect s=p.s;
    s.normalize();
    cout<<s.x<<" "<<s.y<<" "<<s.z<<" "<<p.x<<" "<<p.y<<" "<<p.z<<endl;
  }
}

main(int arg_c, char *arg_a[]){
  {
    char * bmp=getenv("SEED");
    if(bmp!=NULL){
      int seed=atoi(bmp);
      srand48(seed);
      cerr<<"SEED="<<seed<<endl;
    }
  }
  {
    char * bmp=getenv("ORFR");
    if(bmp!=NULL && !(*bmp!=0 && atoi(bmp)==0)){
      orefr=true;
      cerr<<"Only refraction; no reflection"<<endl;
    }
  }

  if(arg_c>1){
    int inum=1000;
    {
      char * bmp=getenv("INUM");
      if(bmp!=NULL) inum=atoi(bmp);
      cerr<<"INUM="<<inum<<endl;
    }
    int jnum=100000;
    {
      char * bmp=getenv("JNUM");
      if(bmp!=NULL) jnum=atoi(bmp);
      cerr<<"JNUM="<<jnum<<endl;
    }

    if(arg_c>2) girdle=atof(arg_a[2])>0;

    test2(atof(arg_a[1])*M_PI/180, inum, jnum);
  }
  else{
    cerr<<"Usage: [SEED=0] [ORFR=0] [INUM=1000] [JNUM=100000] bfr [angle to flow] [girdle]"<<endl;
    test();
  }
}
