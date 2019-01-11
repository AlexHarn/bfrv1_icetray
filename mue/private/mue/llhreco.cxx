#include <cmath>
#include <gsl/gsl_errno.h>
#include <gsl/gsl_sf_gamma.h>
#include <gsl/gsl_sf_hyperg.h>
#include <gsl/gsl_multimin.h>
#include <gsl/gsl_roots.h>

#include <fstream>

#include "reader.h"
#include "llhreco.h"
#include "interpol.h"
#include "miniball.h"

using namespace std;

namespace I3FRreco{

  struct cpdf:intfunction {  // holds the convoluted pandel pdf parameterization
    static const int verbose=0;

    double l;     // scattering length
    double la;    // absorption length
    double tau;   // cpandel tau
    double jit;   // cpandel jitter

    double wv;    // light wavelength
    double np;    // phase refractive index
    double ng;    // group refractive index

    int size;                    // size of kurt table
    double dh, hmin, hmax;       // step, min and max depth
    double *abs, *sca;           // arrays of absorption and scattering values
    double *abs_sum, *sca_sum;   // arrays of absorption and scattering depth-integrated values
    double lf, ls;               // multiplicative factor: pandel lambda/scattering length=lf+ls*distance
    double laf, las;             // multiplicative factor: pandel absorption/absorption length=laf+las*d
    bool kurt;                   // whether kurt table is used

    static double udf, thmax;    // see definitions below
    static bool updown, invert;

    static const double le, lr, rlen, elen, th0, dd0, c, lm;
    static const double a1, a00, a01, a02, a2, at, ts1, ts2, pmin, pre;
    static const double dmin, dmax, admin, admax, tmin, tmax, atmin, atmax;
    static const bool fixdir, lcflag, mnball, nnhit;
    static const int om_pad, dnum, tnum;

    void init(double wv){  // initialize ice parameters
      np=1.31950;
      ng=1.35634;
      if(wv<0.3) wv=0.3;
      else if(wv>0.6) wv=0.6;
      this->wv=wv;

      kurt=false;
      if(cpandel_flag) return;

      double A, B, D, E, a, k;
      double Ae, Be, De, Ee, ae, ke;

      bool flag=true;
      ifstream inFile;
      inFile.open("icemodel.par", ifstream::in);

      flag=(!inFile.fail());
      if(flag){
	if(flag) flag=(bool)(inFile >> a >> ae);
	if(flag) flag=(bool)(inFile >> k >> ke);
	if(flag) flag=(bool)(inFile >> A >> Ae);
	if(flag) flag=(bool)(inFile >> B >> Be);
	if(flag) flag=(bool)(inFile >> D >> De);
	if(flag) flag=(bool)(inFile >> E >> Ee);
	if(!flag) cerr << "File icemodel.par found, but is corrupt, falling back to bulk ice" << endl;
	inFile.close();
	if(!flag) return;
      }
      else return;

      vector<double> dp, be, ba, td;
      inFile.open("icemodel.dat", ifstream::in);
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
      else return;

      double wva=wv*1.e3;
      double wv0=400;
      double l_a=pow(wva/wv0, -a);
      double l_k=pow(wva, -k);
      double ABl=A*exp(-B/wva);

      abs = new double[size];
      sca = new double[size];

      double abs_a=0, sca_a=0;
      dh=dp[1]-dp[0];
      if(dh<=0){ cerr << "Ice table does not use increasing depth spacing, falling back to bulk ice" << endl; return; }

      for(int i=0; i<size; i++){
	if(i>0) if(fabs(dp[i]-dp[i-1]-dh)>dh*1.e-10){ cerr << "Ice table does not use uniform depth spacing, falling back to bulk ice" << endl; return; }
	sca[i]=be[i]*l_a;
	abs[i]=(D*ba[i]+E)*l_k+ABl*(1+0.012*td[i]);
	if(sca[i]<=0 || abs[i]<=0){ cerr << "Invalid value of ice parameter, falling back to bulk ice" << endl; return; }
	sca_a+=sca[i]; abs_a+=abs[i];
      }

      sca_a=size/sca_a;
      abs_a=size/abs_a;

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

      double wv2=wv*wv;
      double wv3=wv*wv2;
      double wv4=wv*wv3;
      np=1.55749-1.57988*wv+3.99993*wv2-4.68271*wv3+2.09354*wv4;
      ng=np*(1+0.227106-0.954648*wv+1.42568*wv2-0.711832*wv3);

      cerr << "Using 6-parameter ice model at l=" << wva << "nm: np=" << np << " ng=" << ng
	   << " average sca=" << sca_a << " abs=" << abs_a << " depths=(" << hmin << ";" << hmax << ")" << endl;
      kurt=true;
    }

    bool cpandel_flag;       // true if cpandel is parameterized
    interpol *cpandel_intc;  // holds parameterization table

    double cpandel(double d, double t){  // cpandel parameterization according to G. Japaridze and M. Ribordy
      double result;

      if(cpandel_flag){
	if(d<admin || d>admax || t<atmin || t>atmax) result=0;
	else result=exp(cpandel_intc->interpolate(d, t))-lm;
	if(result<0 || result!=result || result+1==result) result=0;
	if(verbose>10){
	  cpandel_flag=false;
	  double exact=cpandel(d,t);
	  double diff=2*(result-exact)/(result+exact);
	  static double maxdiff=0;
	  if(fabs(diff)>maxdiff) maxdiff=fabs(diff);
	  cout<<"interpolation maxdiff: "<<maxdiff<<" diff: "<<diff<<" = "
	      <<result<<" - "<<exact<<" (d="<<d<<" t="<<t<<")"<<endl;
	  cpandel_flag=true;
	}
	return result+lm;
      }

      double sig=jit;

      double l=(lf+ls*d)*this->l;
      double la=(laf+las*d)*this->la;

      if(d<=0) result=exp(-t*t/(2*sig*sig))/(sqrt(2*M_PI)*sig);
      else{
	double xi=d/l;
	double cm=c/ng;
	double rho=1/tau+cm/la;
	double eta=rho*sig-t/sig;

	gsl_sf_result g0, g1, g2, h1, h2;
	if(gsl_sf_gamma_e(xi, &g0)!=GSL_SUCCESS) g0.val=0;

	double t0=rho*sig*sig;
	if(t>t0+ts2*sig){
	  double aux=sig/(t-t0);
	  result=(1/g0.val)*pow(rho, xi)*pow(t, xi-1)*exp(-rho*t)*
	    exp(rho*t0/2)*pow(1-t0/t, xi-1)*(1+aux*aux*(1-xi)*(2-xi));
	}
	else if(t<t0-ts1*sig*max(xi, 1.)){
	  result=exp(-t*t/(2*sig*sig))/(sqrt(2*M_PI)*sig*pow(1-t/t0, xi));
	}
	else{
	  if(gsl_sf_gamma_e(xi/2, &g1)!=GSL_SUCCESS) g1.val=0;
	  if(gsl_sf_gamma_e((xi+1)/2, &g2)!=GSL_SUCCESS) g2.val=0;
	  if(gsl_sf_hyperg_1F1_e(xi/2, 1./2, eta*eta/2, &h1)!=GSL_SUCCESS) h1.val=0;
	  if(gsl_sf_hyperg_1F1_e((xi+1)/2, 3./2, eta*eta/2, &h2)!=GSL_SUCCESS) h2.val=0;

	  result=((pow(rho*sig, xi)*pow(2, (xi-3)/2))/(sqrt(M_PI)*g0.val))*
	    (exp(-t*t/(2*sig*sig))/sig)*(g1.val*h1.val-M_SQRT2*eta*g2.val*h2.val);
	}
	if(result<0 || result!=result || result+1==result) result=0;
      }

      return result+lm;
    }

    void destroy(){  // empties booked arrays and tables
      if(kurt){
	delete abs;
	delete sca;
	delete abs_sum;
	delete sca_sum;
      }
      if(cpandel_flag){
	cerr << "Removing cpandel parameterization ... ";
	cpandel_flag=false;
	delete cpandel_intc;
	cerr << "done" << endl;
      }
    }

    virtual ~cpdf(){
      destroy();
    }

    double ifunction(double d, double t){  // intfunction interface
      return cpandel(d, t);
    }

    // constructor
    cpdf(double l, double la, double tau, double jit, double lf, double ls, double laf, double las, double wv, bool iflag, const char *rname){
      this->l=l;
      this->ls=ls;
      this->la=la;
      this->las=las;
      this->tau=tau;
      this->jit=jit;
      this->lf=lf;
      this->laf=laf;
      this->rname=rname;
      cpandel_flag=iflag;
      init(wv);
      if(cpandel_flag){
	cerr << "Initializing cpandel parameterization ... ";
	cpandel_flag=false;
	cpandel_intc = new interpol(dnum, dmin, dmax, tnum, tmin, tmax, this);
	cpandel_flag=true;
	cerr << "done" << endl;
      }
    }

    // sets ice parameters for specified depth
    void lx(double x){
      if(!kurt) return;
      int i=(int) floor((x-hmin)/dh); if(i<0) i=0; else if(i>=size) i=size-1;
      this->l=1/sca[i];
      this->la=1/abs[i];
    }

    // calculates average ice parameters between given depths using prescription by G. Japaridze and M. Ribordy
    void ll(double x1, double x2){
      if(!kurt) return;

      double zz, rsca, rabs;
      if(x2<x1){ zz=x1; x1=x2; x2=zz; }
      int i=(int) floor((x1-hmin)/dh); if(i<0) i=0; else if(i>=size) i=size-1;
      if(x1==x2){ rsca=sca[i]; rabs=abs[i]; }
      else{
	int f=(int) floor((x2-hmin)/dh); if(f<0) f=0; else if(f>=size) f=size-1;
	rsca=((sca_sum[f]-sca_sum[i])*dh-((hmin+(f+1)*dh)-x2)*sca[f]+((hmin+(i+1)*dh)-x1)*sca[i])/(x2-x1);
	rabs=((abs_sum[f]-abs_sum[i])*dh-((hmin+(f+1)*dh)-x2)*abs[f]+((hmin+(i+1)*dh)-x1)*abs[i])/(x2-x1);
      }
      this->l=1/rsca;
      this->la=1/rabs;
    }

  };

  const double cpdf::lr=0.6;     // [  fraction of the scattering length  ]  for scattering/timing profile
  const double cpdf::le=0.3;     // [after which initial direction is lost]  for photon density calculation
  const double cpdf::rlen=0;     // addtional effective distance variation with angle of cascade (timing)
  const double cpdf::elen=0;     // addtional effective distance variation with angle of cascade (energy)
  const double cpdf::th0=90;     // initial zenith angle, fixed direction for cascades when rmsk&8
  const double cpdf::dd0=0;      // initial closest approach distance, fixed for cascades when rmsk&8
  const double cpdf::c=0.299792458;
  const double cpdf::lm=0.5e-6;  // noise likelihood

  double cpdf::udf=1.e-5;  // suppression of upgoing tracks for Bayesian reconstruction
  double cpdf::thmax=100;  // zenith angle cut for tracks (used when rmsk&512)
  bool cpdf::updown=true;  // false restricts to downgoing tracks for Bayesian reconstruction
  bool cpdf::invert=false; // true inverts up <--> down

  const bool cpdf::fixdir=true;  // true restricts to downgoing showers
  const bool cpdf::lcflag=true;  // true recalculates hit probabilities taking LC into account
  const bool cpdf::mnball=true;  // include all DOMs in the miniball into the p_nohit calculation
  const bool cpdf::nnhit=false;  // true disables accounting for the no hit probability
  const int cpdf::om_pad=1;      // number of DOMs above and below the ones hit included into the no hit probability

  const double cpdf::a1=0.84;    // parameters from Ch. Wiebusch reco paper
  const double cpdf::a00=3.1;
  const double cpdf::a01=-3.9;
  const double cpdf::a02=4.6;
  const double cpdf::a2=0.3;
  const double cpdf::at=1.;

  const double cpdf::ts1=5;        // range of validity or the full cpandel formula
  const double cpdf::ts2=25;       // ... use asymptotic formulae outside the range
  const double cpdf::pmin=1.e-50;  // minimum phit probability
  const double cpdf::pre=1.e-2;    // approximate 1/sqrt of smaller arguments

  const int cpdf::dnum=1000;       // interpolation bounds and steps
  const double cpdf::dmin=-150;
  const double cpdf::dmax=1050;
  const double cpdf::admin=-100;
  const double cpdf::admax=1000;

  const int cpdf::tnum=1000;
  const double cpdf::tmin=-150;
  const double cpdf::tmax=3050;
  const double cpdf::atmin=-100;
  const double cpdf::atmax=3000;

  // cpdf(47.0, 98.0, 450., 15., 1.0, 0.0, 1.0, 0.0, 0.380, false, ...) cascade llhf by M. Kowalski
  // cpdf(25.0, 98.0, 198., 15., 1.2, 0.0, 1.0, 0.0, 0.380, false, ...) cpandel re-fit by M. Ribordy

  static cpdf *ppf[2];
  static bool ppstf=false;

  void pp_start(int h){  // initializes cpandel structures
    if(ppstf) return;
    gsl_set_error_handler_off();
    if(get_inic()>0 || get_icsn()>0){
      /*
	int m=get_mmin();
	int m1=m/100;
	int m2=m%100;
	double l=0.1+0.1*m1;
	double la=.05+0.05*m2;
	cout<<m<<" "<<m1<<" "<<m2<<" "<<l<<" "<<la<<endl;
      */

      switch(h){
      case 1:  // data
	if(1&get_rmsk()) ppf[0] = new cpdf(21.8, 105., 1.e10, 4.0, 2.0, 0.0, 0.5, 0.0, 0.380, getinterpo()>0, "data_track");
	if(2&get_rmsk()) ppf[1] = new cpdf(20.0, 101., 1.e10, 4.0, 0.35, 0.006, 0.055, 0.001, 0.380, getinterpo()>0, "data_cascade");
	//constant l,la: ppf[1] = new cpdf(20.0, 101., 1.e10, 4.0, 1.80, 0.000, 0.300, 0.000, 0.380, getinterpo()>0, "data_cascade");
	break;
      case 2:  // monte carlo
	if(1&get_rmsk()) ppf[0] = new cpdf(33.3, 98.0, 557., 15., 1.0, 0.0, 1.0, 0.0, 0.380, getinterpo()>0, "mc_track");
	if(2&get_rmsk()) ppf[1] = new cpdf(47.0, 98.0, 450., 15., 1.0, 0.0, 1.0, 0.0, 0.380, getinterpo()>0, "mc_cascade");
	break;
      }
    }
    ppstf=true;
  }

  void pp_stop(){  // empties cpandel structures
    if(!ppstf) return;
    ppstf=false;
    if(get_inic()>0 || get_icsn()>0){
      for(int i=0; i<2; i++) if((i+1)&get_rmsk()) delete ppf[i];
    }
  }

  void pp_setpars(double udf, double thmax, bool invert){
    cpdf::udf=udf;
    cpdf::updown=udf!=1.0;
    cpdf::invert=invert;
    cpdf::thmax=thmax;
  }

  static int freco, vnum, vnux;
  static bool rspe, tfix=false, rfx=true;
  static double rfx_th=0, rfx_ph=0, rfx_ct=0;

  // track time residual
  double tfit(double t, double z, double th, double dd, double ddp){
    double ng=ppf[freco-1]->ng;
    double na=(ppf[freco-1]->np*ppf[freco-1]->ng-1)/sqrt(ppf[freco-1]->np*ppf[freco-1]->np-1);
    double sinth=sin(th), costh=cos(th);
    double sqrtn=sqrt(dd*dd+z*z*sinth*sinth);
    return t-(z*costh+na*sqrtn-ng*fabs(ddp))/cpdf::c;
  }

  // cascade time residual
  double cfit(double t, double z, double dd, double ddp){
    double cm=cpdf::c/ppf[freco-1]->ng;
    double sqrtn=sqrt(dd*dd+z*z);
    return t-(sqrtn-fabs(ddp))/cm;
  }

  static multe *evts;
  static double lle, dlle, nllr;

  double vector_get(const void *par, const gsl_vector *x, int i){
    if(i<vnum) return gsl_vector_get(x, i);
    else return ((double *)par)[i];
  }

  double (*llh)(const gsl_vector *, gsl_vector *, void *, bool);

  static bool extf=true;
  map<unsigned long long, double> qdoms;
  bool parinic=false;

  struct vtemp{
    double t0;
    double z0;
    double th;
    double dd;
    double ph;
    double az;
    double nn;
    double x0;
    double y0;
    short dth;
    short ddf;

    vtemp(){
      nn=0;
      dth=1;
    }

    void norm(int i){
      switch(i){
      case 1:
	if(th<0){
	  th=-th;
	  dth*=-1;
	}
	if(th>=360) th-=360*floor(th/360); // while(th>360) th-=360;
	if(th>180){
	  th=360-th;
	  dth*=-1;
	}
	if(!cpdf::updown) if(freco==1 && rspe) if(th<90){
	  th=180-th;
	  dth*=-1;
	}
	if(cpdf::invert) th=180-th;
	if(ph<0 || ph>=360) ph-=360*floor(ph/360);
	if(az<0 || az>=360) az-=360*floor(az/360);
	if(nn<0) nn=-nn;
	break;
      case 2:
	ddf=0==get_strn()&&dd<0?-1:1;
	dd*=dd;
	nn*=nn;
	break;
      default:
	cerr<<"Internal Error: Unknown option to vtemp.norm()"<<endl;
      }
    }

    void norm(){
      for(int i=1; i<=2; i++) norm(i);
    }

  };

  vtemp& gettmp_inic(const vtemp &par, const gsl_vector *x){
    static vtemp vtmp;
    int i=0;
    vtmp=par;
    vtmp.t0=gsl_vector_get(x, i++);
    vtmp.z0=gsl_vector_get(x, i++);
    if((freco==1&&(!tfix)) || ((freco==2)&&(!rspe))) vtmp.th=gsl_vector_get(x, i++);
    if((freco==1) || ((freco==2)&&(0==get_strn()))) vtmp.dd=gsl_vector_get(x, i++);
    if(((freco==1)&&(0==get_strn())) || ((freco==2)&&(0==get_strn()))) vtmp.ph=gsl_vector_get(x, i++);
    if((freco==2)&&(!rspe)) vtmp.az=gsl_vector_get(x, i++);
    if(vnum==vnux) vtmp.nn=gsl_vector_get(x, i++);
    if(i!=vnum){ cerr<<"Internal Error: vnum parameter mismatch!"<<endl; exit(-1); }
    return vtmp;
  }

  int settmp_inic(gsl_vector *& x, const vtemp &vtmp){
    double par[7];
    int max=0;
    par[max++]=vtmp.t0;
    par[max++]=vtmp.z0;
    if(((freco==1)&&(!tfix)) || ((freco==2)&&(!rspe))) par[max++]=vtmp.th;
    if((freco==1) || ((freco==2)&&(0==get_strn()))) par[max++]=vtmp.dd;
    if(((freco==1)&&(0==get_strn())) || ((freco==2)&&(0==get_strn()))) par[max++]=vtmp.ph;
    if((freco==2)&&(!rspe)) par[max++]=vtmp.az;
    if(vnum==vnux) par[max++]=vtmp.nn;
    if(x==NULL) x = gsl_vector_alloc(max);
    for(int i=0; i<max; i++) gsl_vector_set(x, i, par[i]);
    return max;
  }

  struct otemp{
    double tc;
    double t0;
    double th;
    double ph;
    double x0;
    double y0;
    double z0;
    double a0;
    double k0;
    short dth;
    short dir;

    otemp(){
      dth=1;
    }

    void norm(int i){
      switch(i){
      case 1:
	if(th<-180 || th>180){ th-=360*floor(th/360); if(th>180) th-=360; }
	if(th<0){
	  th=-th;
	  dth*=-1;
	  ph=ph+180;
	}

	if(cpdf::fixdir){
	  dir=1;
	  if(th<90){
	    th=180-th;
	    dth*=-1;
	  }
	}
	else dir=th<90?-1:1;
	if(ph<0 || ph>=360) ph-=360*floor(ph/360);
	break;
      case 2:
	a0=1+tanh(a0);
	k0=0.30264+0.3*tanh(k0);
	break;
      default:
	cerr<<"Internal Error: Unknown option to otemp.norm()"<<endl;
      }
    }

    void norm(){
      for(int i=1; i<=2; i++) norm(i);
    }

  };

  otemp& gettmp_itop(const otemp &par, const gsl_vector *x){
    static otemp otmp;
    int i=0;
    otmp=par;
    otmp.t0 = gsl_vector_get(x, i++);
    otmp.th = gsl_vector_get(x, i++);
    otmp.ph = gsl_vector_get(x, i++);
    if(vnum>3) otmp.x0 = gsl_vector_get(x, i++);
    if(vnum>4) otmp.y0 = gsl_vector_get(x, i++);
    if(vnum>5) otmp.a0 = gsl_vector_get(x, i++);
    if(vnum>6) otmp.k0 = gsl_vector_get(x, i++);
    return otmp;
  }

  int settmp_itop(gsl_vector *& x, const otemp &otmp){
    if(x==NULL) x = gsl_vector_alloc(vnum);
    int max=0;
    gsl_vector_set(x, max++, otmp.t0);
    gsl_vector_set(x, max++, otmp.th);
    gsl_vector_set(x, max++, otmp.ph);
    if(vnum>3) gsl_vector_set(x, max++, otmp.x0);
    if(vnum>4) gsl_vector_set(x, max++, otmp.y0);
    if(vnum>5) gsl_vector_set(x, max++, otmp.a0);
    if(vnum>6) gsl_vector_set(x, max++, otmp.k0);
    return max;
  }

  bool reco_reset=true;

  void reco_e(vtemp&, preco *, double);
  void reco_e(otemp&, preco *, double);
  void reco_e(multe& e, preco& r, double n){
    freco = abs(r.type);
    evts=&e;

    vtemp vtmp;
    vtmp.z0=r.z0;
    vtmp.th=r.th;
    vtmp.dd=fabs(r.dd);
    vtmp.ph=r.ph;
    vtmp.az=r.az;
    vtmp.x0=r.x0;
    vtmp.y0=r.y0;
    vtmp.ddf=r.ddf;
    vtmp.norm(1);

    reco_reset=true;
    reco_e(vtmp, &r, n);
  }


  // track and cascade log likelihood function
  double illh(const gsl_vector *x, gsl_vector *df, void *par, bool flag){
    vtemp vtmp=gettmp_inic(*(vtemp *) par, x);
    vtmp.norm();

    double t0=vtmp.t0;
    double z0=vtmp.z0;
    double th=vtmp.th*M_PI/180;
    double ph=vtmp.ph*M_PI/180;
    double az=vtmp.az*M_PI/180;
    double dd=vtmp.dd;
    double nn=vtmp.nn;
    double x0=vtmp.x0;
    double y0=vtmp.y0;
    short ddf=vtmp.ddf;
    double sinth=sin(th);
    double costh=cos(th);

    double ddp=dd;
    double sinph=sin(ph);
    double cosph=cos(ph);

    double coschr=1/ppf[freco-1]->np;
    double sinchr=sqrt(1-coschr*coschr);

    double result=0, zsum=0, qsum=0;

    for(vector<event>::const_iterator it = evts->events.begin(); it!=evts->events.end(); ++it){
      map<unsigned long long, mydom>::const_iterator cdom = idoms().find(it->id);
      const mydom& dom = cdom->second;
      int om = dom.om;
      if(om>=1 && om<=60) if(0==get_strn() || dom.str==get_strn()) if(it->Q>=0 && it->fi<=0){
	double dt=((long long)(it->gt-evts->t0))/10.-t0;
	double dz=dom.z-z0;
	double fit=0, lx, sz, sqrtd=0, d=0, coseta=0, dez=0;
	switch(freco){
	case 1:
	  if(0==get_strn()){
	    double dx=dom.x-x0;
	    double dy=dom.y-y0;
	    double dT=dx*cosph+dy*sinph;
	    dx-=-ddp*sinph*ddf+dT*cosph;
	    dy-=ddp*cosph*ddf+dT*sinph;
	    dd=sqrt(dx*dx+dy*dy);
	    if(sinth!=0){
	      dT/=sinth;
	      dt-=dT/cpdf::c;
	      dz-=costh*dT;
	    }
	  }
	  sz=dz*sinth;
	  sqrtd=sqrt(dd*dd+sz*sz);
	  coseta=coschr*costh+sinchr*sinth*(sqrtd>cpdf::pre?sz/sqrtd:1);
	  d=cpdf::a00+cpdf::a01*coseta+cpdf::a02*coseta*coseta+cpdf::a1*sqrtd;
	  fit=tfit(dt, dz, th, dd, ddp);
	  if(!ppf[freco-1]->cpandel_flag) dez=coseta*(sqrtd/sinchr); else dez=0;
	  break;
	case 2:
	  double cospsi=0;
	  if(0==get_strn()){
	    double dx=dom.x-x0;
	    double dy=dom.y-y0;
	    dx-=ddp*cosph;
	    dy-=ddp*sinph;
	    dd=sqrt(dx*dx+dy*dy);
	    if(!rspe){
	      double sinaz=sin(az);
	      double cosaz=cos(az);
	      double nx=sinth*cosaz, ny=sinth*sinaz;
	      cospsi=nx*dx+ny*dy;
	    }
	  }
	  if(!rspe){
	    ppf[freco-1]->lx(-z0);
	    lx=ppf[freco-1]->l*cpdf::lr+cpdf::rlen;
	  }
	  else lx=0;
	  sqrtd=sqrt(dz*dz+dd*dd);
	  if(!rspe) cospsi=(cospsi+costh*dz)/sqrtd;
	  coseta=(sqrtd>cpdf::pre?dz/sqrtd:1);
	  d=max(0., cpdf::a00+cpdf::a01*coseta+cpdf::a02*coseta*coseta+cpdf::a1*sqrtd-lx*cospsi);
	  fit=cfit(dt, dz, dd, ddp);
	  if(!ppf[freco-1]->cpandel_flag) dez=dz; else dez=0;
	  break;
	}
	ppf[freco-1]->ll(-dom.z, -dom.z+dez);
	double cpn=ppf[freco-1]->cpandel(d, fit);
	double msk=(get_rmsk()&128?it->Q:get_rmsk()&32?(qdoms[cdom->first]>0?it->Q/qdoms[cdom->first]:0):1);
	if(cpdf::verbose>2) cout << cpn << endl;
	result-=msk*log(cpn);
	if(rfx){ zsum-=msk; qsum+=it->Q; }

	if(parinic) cout<<"parinic "<<(it->Q)<<" "<<d<<" "<<fit<<" "<<sqrtd<<" "<<coseta<<" "<<vtmp.th<<" "<<dd<<" "
			<<dt<<" "<<dz<<" "<<dez<<" "<<(ppf[freco-1]->l)<<" "<<(ppf[freco-1]->la)<<" "<<cpn<<endl;
      }
    }

    double dif=rfx_ct;
    if(extf && freco==1 && rspe){
      if(rfx){
	double cosrth=cos(rfx_th*M_PI/180);
	double sinrth=sin(rfx_th*M_PI/180);
	double cosrph=cos(rfx_ph*M_PI/180);
	double sinrph=sin(rfx_ph*M_PI/180);
	dif=costh*cosrth+sinth*sinrth*(cosph*cosrph+sinph*sinrph);
	if(dif>rfx_ct) result=zsum*log(cpdf::lm);
      }
      else result-=log(costh<-cpdf::udf?-costh:cpdf::udf)*nllr;
    }

    if(extf && vnum==vnux){
      if(freco==1 && rspe && rfx && dif>rfx_ct) result+=1.e5*qsum*nllr;
      else{
	reco_e(vtmp, NULL, nn);
	result-=lle*nllr;
      }
    }

    if(cpdf::verbose>1) printf("%g %g %g %g %g %g\n", result, t0, z0, vtmp.th, dd, nn);
    if(flag) for(int i=0; i<vnum; i++) gsl_vector_set(df, i, 0);

    return result;
  }

  static int itopreco=2;
  static bool itopdok0=false;
  static double tAmean, tArms;
  static const double pepp(const mydom& dom){
    return dom.vem;
  }

  // surface particle density
  double pdens(const mydom& dom, double q){
    return q/pepp(dom);
  }

  bool paritop=false;

  // chi^2 function for icetop shower time residuals
  double tllh(const gsl_vector *x, gsl_vector *df, void *par, bool flag){
    double dfr[vnum];
    if(flag) for(int i=0; i<vnum; i++) dfr[i]=0;

    otemp otmp=gettmp_itop(*(otemp *) par, x);
    otmp.norm();

    double result=0;
    double t = otmp.t0;
    double th = otmp.th*M_PI/180;
    double ph = otmp.ph*M_PI/180;
    double x0 = otmp.x0;
    double y0 = otmp.y0;
    double z0 = otmp.z0;
    double age = otmp.a0;
    double k0=otmp.k0;
    const double r0=75, r100=100;

    if(cpdf::verbose>2) cout<<"tllh: "<<t<<" "<<otmp.th<<" "<<otmp.ph<<" "<<x0<<" "<<y0<<" "<<age<<" ";

    double sinth=sin(th);
    double costh=cos(th);
    double sinph=sin(ph);
    double cosph=cos(ph);
    double nx=sinth*cosph;
    double ny=sinth*sinph;
    double nz=costh;

    double q2=0, qf=0, f2=0;

    for(vector<event>::const_iterator it = evts->events.begin(); it!=evts->events.end(); ++it){
      map<unsigned long long, mydom>::const_iterator cdom = idoms().find(it->id);
      const mydom& dom = cdom->second;
      int om = dom.om;
      if(om>=61 && om<=64) if(it->Q>=0 && it->fi<=0){
	double dt=((long long)(it->gt-evts->t0))/10.-otmp.tc-t;
	double dx=dom.x-x0;
	double dy=dom.y-y0;
	double dz=dom.z-z0;

	double res=dt-(dx*nx+dy*ny+dz*nz)/cpdf::c;
	double msk=(get_rmsk()&64?it->Q:1);

	if(vnum>3){
	  if(!(0<it->Q && it->Q<dom.st)){
	    if(cpdf::verbose>0) cout<<"Warning (reco): charge above saturation: "<<it->Q<<" > "<<dom.st<<endl;
	    continue;
	  }

	  double d=nx*(dom.x-x0)+ny*(dom.y-y0)+nz*(dom.z-z0);
	  double dx=x0+d*nx-dom.x;
	  double dy=y0+d*ny-dom.y;
	  double dz=z0+d*nz-dom.z;
	  double dd=sqrt(dx*dx+dy*dy+dz*dz);
	  double s=pdens(dom, it->Q);

	  if(paritop){ cout<<"paritop "<<dd<<" "<<res<<" "<<s<<" "; }

	  double delta, sigma, qsig;
	  assert (itopreco>=0 and itopreco<=2);
	  switch(itopreco){  //----------> geometrical reconstruction parameters
	  case 0:  // extended SPASE parameterization
	    {
	      double z=s/(2.7/4);
	      delta=pow(z, -0.29)*(dd<69?0.365*dd:dd<120?0.423*dd-4.002:dd<250?46.758:0.187032*dd);
	      sigma=max(3.62-1.18*log(z)/log(10.), 0.1)+(dd<50?7.3e-4*dd*dd:0.073*dd-1.825);
	    }
	    break;
	  case 1:  // new IceTop parameterization
	  case 2:
	    {
	      double z=s/(2.7/4);
	      double pfi[5]={-3.10505, 0.948017, 51.9268, 0.350253, 129.385};
	      double a=39.;
	      double x=dd<a?a:dd;
	      delta=pow(z, -0.29)*pow(10, pfi[0]+pfi[1]*exp(-x*x/(2*pfi[2]*pfi[2]))+pfi[3]*exp(-x*x/(2*pfi[4]*pfi[4])))*x*x;
	      if(dd<a) delta*=dd/a;
	      sigma=1.15*(max(3.62-1.18*log(z)/log(10.), 0.1)+(dd<50?7.3e-4*dd*dd:0.073*dd-1.825));
	    }
            break;
	  }
	  res-=delta;
	  res/=sigma;

	  if(dd<cpdf::at) dd=cpdf::at;
	  double fr;

	  switch(itopreco){  //----------> shower particle density
	  case 0:
	  case 1:  // NKG approximation
	    {
	      fr=pow(dd/r0, age-2.0)*pow(1+dd/r0, age-4.5);
	      // muons: pow(dd/80, -0.4)*exp(-dd/80)
	    }
	    break;
	  case 2:  // SAF approximation (Stefan Arne Fabian)
	    {
	      double b=(3.399-age)/0.838;
	      double rm=pow(10, -b/(2*k0));
	      double x=max(dd, rm)/r100;
	      fr=pow(x, -b-k0*log(x)/log(10.));
	    }
	    break;
	  }

	  switch(itopreco){  //----------> density variation
	  case 0:
	    {
	      double z=s/(2.7/4);
	      qsig=z>0.54?1.26*pow(z, 0.62):1.45*pow(z, 0.85);
	      qsig/=z;
	    }
	    break;
	  case 1:
	    {
	      double z=s/(2.7/4);
	      qsig=1.e-2+0.77/(0.002*pow(z, -5)+pow(z, -0.22));
	    }
	    break;
	  case 2:
	    if(false){  // SAF approximation
	      if(s<0.874) qsig=0.401*pow(s, 0.149);
	      else if(s<21.7) qsig=0.363*pow(s, -0.573);
	      else qsig=0.075*pow(s, 0.062);
	    }
	    else{
	      double y=log(s)/log(10.);
	      qsig=0.459*exp((y-0.794)*(y-0.794)/2.3);
	    }
	    break;
	  }

	  if(paritop){ cout<<delta<<" "<<sigma<<" "<<qsig<<" "<<fr<<endl; }

	  {
	    qsig*=qsig;
	    double lnq=log(s/fr);
	    q2+=lnq*lnq/qsig;
	    qf+=lnq/qsig;
	    f2+=1/qsig;
	  }
	}

	if(vnum<=3) if(flag){
	  dfr[0]+=msk*(2*res*(-1));
	  dfr[1]+=msk*(2*res*(-1/cpdf::c)*(dx*costh*cosph+dy*costh*sinph+dz*(-sinth)));
	  dfr[2]+=msk*(2*res*(-1/cpdf::c)*(dx*sinth*(-sinph)+dy*sinth*cosph));
	}
	result+=msk*res*res;
      }
    }

    if(vnum<=3) if(flag){
      dfr[1]*=otmp.dth*M_PI/180;
      dfr[2]*=M_PI/180;
    }

    if(vnum>3){
      if(f2>0){
	tAmean=qf/f2;
	tArms=q2-2*tAmean*qf+tAmean*tAmean*f2;
	result+=tArms;
      }
      else{
	tAmean=0;
	tArms=0;
      }
      if(paritop){ cout<<"paritop "<<exp(tAmean)<<" "<<age<<endl; }

      if(cpdf::verbose>2) cout<<tAmean<<" "<<tArms<<" ";
    }
    if(cpdf::verbose>2) cout<<age<<" "<<result<<endl;

    if(flag) for(int i=0; i<vnum; i++) gsl_vector_set(df, i, dfr[i]);
    return result;
  }

  // interface functions for gsl minimizers

  double llh_f(const gsl_vector *x, void *par){
    return (*llh)(x, NULL, par, false);
  }

  void llh_df(const gsl_vector *x, void *par, gsl_vector *df){
    (*llh)(x, df, par, true);
  }

  void llh_fdf(const gsl_vector *x, void *par, double *f, gsl_vector *df){
    *f=(*llh)(x, df, par, true);
  }

  // holds parameters for error bounds root finder
  struct epar{
    int num;
    double val;
    double sto;
    void *par;
    gsl_vector *x;
  };

  // interface function for error bounds gsl root finder
  double llh_fx(double x, void *par){
    epar &epar_fx = * (epar *) par;
    gsl_vector *y = gsl_vector_alloc(vnum);
    for(int i=0; i<vnum; i++) gsl_vector_set(y, i, i==epar_fx.num?x:gsl_vector_get(epar_fx.x, i));
    double result=llh_f(y, epar_fx.par)/nllr-epar_fx.val;
    gsl_vector_free(y);
    epar_fx.sto=result;
    return result;
  }

  // finds error bounds on reconstruction parameters
  void errors(void *par, double val, gsl_vector *x, gsl_vector *xmin, gsl_vector *xmax){
    if(cpdf::verbose>2) cout<<"entering errors"<<endl;
    epar epar_fx;
    epar_fx.val=val;
    epar_fx.par=par;
    epar_fx.x=x;

    for(int i=0; i<vnum; i++) for(int side=0; side<2; side++){
      int status;
      int iter = 0, max_iter = 100;
      const gsl_root_fsolver_type *T;
      gsl_root_fsolver *s;
      double r = 0;
      double x_lo, x_hi, xi;

      xi=gsl_vector_get(x, i);
      x_lo=side==0?gsl_vector_get(xmin, i):xi;
      x_hi=side==0?xi:gsl_vector_get(xmax, i);

      epar_fx.num=i;

      double f_lo=llh_fx(x_lo, &epar_fx);
      double f_hi=llh_fx(x_hi, &epar_fx);
      if(f_lo*f_hi>0){
	if(cpdf::verbose>3) cout<<"Failed to bracket root ("<<i<<", "<<(side==0?"down":"up")
				<<"): f("<<x_lo<<")="<<f_lo<<" f("<<x_hi<<")="<<f_hi<<endl;
	continue;
      }

      gsl_function F;

      F.function = &llh_fx;
      F.params = &epar_fx;

      switch(3){
      case 1: T = gsl_root_fsolver_bisection; break;
      case 2: T = gsl_root_fsolver_falsepos; break;
      case 3: T = gsl_root_fsolver_brent; break;
      }
      s = gsl_root_fsolver_alloc (T);
      gsl_root_fsolver_set (s, &F, x_lo, x_hi);

      if(cpdf::verbose>3){
	printf ("using %s method\n", gsl_root_fsolver_name (s));
	printf ("%5s [%9s, %9s] %9s %10s %9s\n", "iter", "lower", "upper", "root", "solution", "range");
      }

      double sto_old=0;
      do{
	iter++;
	gsl_root_fsolver_iterate (s);
	r = gsl_root_fsolver_root (s);

	status = gsl_root_test_residual (epar_fx.sto-sto_old, 1.e-4);
	if(status == GSL_SUCCESS) break;
	sto_old=epar_fx.sto;

	x_lo = gsl_root_fsolver_x_lower (s);
	x_hi = gsl_root_fsolver_x_upper (s);
	status = gsl_root_test_interval (x_lo, x_hi, 0, 1.e-9);
	if(status == GSL_SUCCESS) break;

	status = gsl_root_test_residual (epar_fx.sto, 0.01);
	if(cpdf::verbose>3){
	  if (status == GSL_SUCCESS) printf ("Converged:\n");
	  printf ("%5d [%g, %g] %g %g %g\n", iter, x_lo, x_hi, r, xi, x_hi - x_lo);
	  f_lo=llh_fx(x_lo, &epar_fx);
	  f_hi=llh_fx(x_hi, &epar_fx);
	  cout<<"\tf_lo="<<f_lo<<" f_hi="<<f_hi<<endl;
	}
      } while (status == GSL_CONTINUE && iter < max_iter);

      side==0?gsl_vector_set(xmin, i, r):gsl_vector_set(xmax, i, r);

      gsl_root_fsolver_free (s);
    }
  }

  void order(double &x, double &y){
    if(x>y){
      double aux=x;
      x=y;
      y=aux;
    }
  }

  // performs reconstruction
  void reconstruct(multe& e, bool do_reco){
    evts=&e;
    bool doreco = ppstf && do_reco;
    reco_reset=true;

    if(get_inic()>0 || get_icsn()>0){
      int icsn=0, inice=0, inice_ijs=0, inice_num=0;
      double zapp=0, zamp=0, x0=0, y0=0;
      bool tflag=false;

      map<unsigned long long, int> mltp;
      map<int, int> str;
      for(vector<event>::const_iterator it = evts->events.begin(); it!=evts->events.end(); ++it){
	map<unsigned long long, mydom>::const_iterator cdom = idoms().find(it->id);
	const mydom& dom = cdom->second;
	int om = dom.om;
	if(om>=1 && om<=60) if(0==get_strn() || dom.str==get_strn()) if(it->Q>=0 && it->fi<=0){
	  if(it->ijs==0){ inice_ijs++; mltp[it->id]++; str[dom.str]++; }
	  inice++;
	  if(doreco){
	    x0+=dom.x*it->Q;
	    y0+=dom.y*it->Q;
	    zamp+=it->Q; zapp+=dom.z*it->Q;
	    if(get_rmsk()&32) qdoms[cdom->first]+=it->Q;
	  }
	}
      }
      icsn=str.size();
      e.icsn=icsn;
      inice_num=mltp.size();
      e.inic=inice_num;
      if(doreco && inice_num>=get_inic() && icsn>=get_icsn()){
	x0/=zamp;
	y0/=zamp;
	zapp/=zamp;
	llh=&illh;
	nllr=(get_rmsk()&128?zamp:get_rmsk()&32?inice_num:inice)/zamp;

	for(freco=1; freco<=2; freco++) if(freco & get_rmsk()){
	  int i;
	  double llh, llh_e;
	  vtemp vres;

	  double t0app=0, z0app=0, diff=-1;
	  for(vector<event>::const_iterator it = evts->events.begin(); it!=evts->events.end(); ++it){
	    map<unsigned long long, mydom>::const_iterator cdom = idoms().find(it->id);
	    const mydom& dom = cdom->second;
	    int om = dom.om;
	    if(om>=1 && om<=60) if(0==get_strn() || dom.str==get_strn()) if(it->Q>=0 && it->fi<=0){
	      double z_om = dom.z;
	      if(diff<0 || fabs(zapp-z_om)<diff){
		z0app=z_om;
		t0app=((long long)(it->gt-evts->t0))/10.;
		diff=fabs(zapp-z_om);
	      }
	    }
	  }
	  if(freco==2){
	    double cm=cpdf::c/ppf[freco-1]->ng;
	    double z1aux=0, z2aux=0, q1aux=0, q2aux=0;
	    for(vector<event>::const_iterator it = evts->events.begin(); it!=evts->events.end(); ++it){
	      map<unsigned long long, mydom>::const_iterator cdom = idoms().find(it->id);
	      const mydom& dom = cdom->second;
	      int om = dom.om;
	      if(om>=1 && om<=60) if(0==get_strn() || dom.str==get_strn()) if(it->Q>=0 && it->fi<=0){
		double z_om = dom.z;
		if(z_om>z0app){
		  z1aux+=it->Q*(z_om-cm*(((long long)(it->gt-evts->t0))/10.));
		  q1aux+=it->Q;
		}
		else if(z_om<z0app){
		  z2aux+=it->Q*(z_om+cm*(((long long)(it->gt-evts->t0))/10.));
		  q2aux+=it->Q;
		}
	      }
	    }
	    if(q1aux>0 && q2aux>0){
	      z1aux/=q1aux;
	      z2aux/=q2aux;
	      double zaux=(z1aux+z2aux)/2;
	      t0app-=fabs(z0app-zaux)/cm;
	      z0app=zaux;
	    }
	  }

	  vtemp vpar;
	  preco new_reco;
	  // new_reco.gt=evts->t0;

	  rspe=get_rmsk()&8;
	  int vnus=rspe && 256 & get_rmsk() ?1:0;
	  if(freco==1) rfx=vnus>0;  // enables complementary space search (delete to default to bayesian reconstruction)

	  for(int s=0; s<=vnus; s++){
	    if(vnus>0) rspe=!rspe;

	    vpar.t0=t0app;
	    vpar.z0=z0app;
	    vpar.th=cpdf::th0;
	    vpar.dd=cpdf::dd0;
	    vpar.ph=0;
	    vpar.az=0;
	    vpar.nn=10;
	    vpar.x0=x0;
	    vpar.y0=y0;

	    vnum=4; vnux=5;
	    gsl_vector *x=NULL;
	    vnum=settmp_inic(x, vpar);
	    vnux=vnum+1;

	    {
	      int nmx=freco==1&&vnum>2?0!=get_strn()?60:12:1;
	      double llhs=0, t0aux=0, thaux=0, phaux=0;
	      if(tfix) nmx=1;

	      for(int n=!cpdf::updown && freco==1 && rspe ?nmx/2:0; n<nmx; n++){
		double thn=nmx>1?(0.5+n)*180./nmx:vpar.th;
		double t0min=0, thmin=thn, phmin=0, llh;

		if(0!=get_strn()){
		  t0min=t0app;
		  for(vector<event>::const_iterator it = evts->events.begin(); it!=evts->events.end(); ++it){
		    map<unsigned long long, mydom>::const_iterator cdom = idoms().find(it->id);
		    const mydom& dom = cdom->second;
		    int om = dom.om;
		    if(om>=1 && om<=60) if(0==get_strn() || dom.str==get_strn()) if(it->Q>=0 && it->fi<=0){
		      double dt=((long long)(it->gt-evts->t0))/10.;
		      double dz=dom.z-z0app;
		      double tm=freco==1?tfit(dt, dz, (cpdf::invert?180-thn:thn)*M_PI/180, 0, 0):cfit(dt, dz, 0, 0);
		      if(tm<t0min) t0min=tm;
		    }
		  }
		  vpar.t0=t0min;
		  vpar.th=thn;
		  settmp_inic(x, vpar);

		  llh=llh_f(x, &vpar);
		}

		else{
		  int nmy=24;
		  double llhs=0, t0aux=0, thaux=0, phaux=0, thnn=thn;

		  for(int m=0; m<nmy; m++){
		    if(nmx>1) thn=thnn+(m-(nmy-1)/2.)*(180./nmx)/nmy;

		    double sinth=sin(thn*M_PI/180);
		    double costh=cos(thn*M_PI/180);

		    double phn=m*360./nmy;
		    double sinph=sin(phn*M_PI/180);
		    double cosph=cos(phn*M_PI/180);

		    double t0min=t0app;
		    for(vector<event>::const_iterator it = evts->events.begin(); it!=evts->events.end(); ++it){
		      map<unsigned long long, mydom>::const_iterator cdom = idoms().find(it->id);
		      const mydom& dom = cdom->second;
		      int om = dom.om;
		      if(om>=1 && om<=60) if(0==get_strn() || dom.str==get_strn()) if(it->Q>=0 && it->fi<=0){
			double dt=((long long)(it->gt-evts->t0))/10.;
			double dz=dom.z-z0app;

			short ddf=1;
			double dd=0, ddp=0;
			if(freco==1){
			  double dx=dom.x-x0;
			  double dy=dom.y-y0;
			  double dT=dx*cosph+dy*sinph;
			  dx-=-ddp*sinph*ddf+dT*cosph;
			  dy-=ddp*cosph*ddf+dT*sinph;
			  dd=sqrt(dx*dx+dy*dy);
			  if(sinth!=0){
			    dT/=sinth;
			    dt-=dT/cpdf::c;
			    dz-=costh*dT;
			  }
			}
			else if(freco==2){
			  double dx=dom.x-x0;
			  double dy=dom.y-y0;
			  dx-=ddp*cosph;
			  dy-=ddp*sinph;
			  dd=sqrt(dx*dx+dy*dy);
			}

			double tm=freco==1?tfit(dt, dz, (cpdf::invert?180-thn:thn)*M_PI/180, dd, ddp):cfit(dt, dz, dd, ddp);
			if(tm<t0min) t0min=tm;
		      }
		    }
		    vpar.t0=t0min;
		    vpar.th=thn;
		    vpar.ph=phn;
		    settmp_inic(x, vpar);

		    double llh=llh_f(x, &vpar);

		    if(llhs==0 || llh<llhs){
		      llhs=llh;
		      t0aux=t0min;
		      thaux=thn;
		      phaux=phn;
		    }
		    // cout<<thn<<" "<<phn<<" "<<llh<<endl;
		  }
		  llh=llhs;
		  t0min=t0aux;
		  thmin=thaux;
		  phmin=phaux;
		}
		// cout<<thn<<" "<<llh<<endl;

		if(llhs==0 || llh<llhs){
		  llhs=llh;
		  t0aux=t0min;
		  thaux=thmin;
		  phaux=phmin;
		}
	      }

	      // cout<<"approx "<<thaux<<endl;
	      vpar.t0=t0aux;
	      vpar.th=thaux;
	      if(0==get_strn()) vpar.ph=phaux;
	      settmp_inic(x, vpar);
	    }

	    int vnui=get_rmsk()&16?1:0;
	    for(int j=0; j<=vnui; j++){
	      if(j==0){ if(512 & get_rmsk()) if(freco==1 && ((!rspe) || ((!rfx) && cpdf::updown))) if(vpar.th>cpdf::thmax){ tflag=true; e.inic=0; break; } }
	      else if(j==1){
		gsl_vector *y;
		y = gsl_vector_alloc(vnux);
		for(i=0; i<vnum; i++) gsl_vector_set(y, i, gsl_vector_get(x, i));
		gsl_vector_free(x);

		gsl_vector_set(y, vnum, sqrt(new_reco.n0));
		vnum=vnux;
		x = y;
	      }

	      {
		const gsl_multimin_fminimizer_type *T;
		gsl_multimin_fminimizer *s;

		vtemp vstep;
		vstep.t0=-100;
		vstep.z0=100;
		vstep.th=180;
		vstep.dd=3;
		vstep.ph=180;
		vstep.az=180;
		vstep.nn=j>0?sqrt(new_reco.n0)/2:3;
		vstep.x0=x0;
		vstep.y0=y0;

		gsl_vector *step;
		step = gsl_vector_alloc(vnum);
		settmp_inic(step, vstep);

		gsl_multimin_function reco_f;

		reco_f.f = &llh_f;
		reco_f.n = vnum;
		reco_f.params = &vpar;

		T = gsl_multimin_fminimizer_nmsimplex;
		s = gsl_multimin_fminimizer_alloc(T, vnum);

		gsl_multimin_fminimizer_set(s, &reco_f, x, step);
		double dist;

		for(i=0; i<1000; i++){
		  int status = gsl_multimin_fminimizer_iterate(s);
		  if(status) break;

		  dist=gsl_multimin_fminimizer_size(s);
		  status = gsl_multimin_test_size(dist, 1.e-4);

		  if(status == GSL_SUCCESS) break;
		}

		if(j==-1){
		  parinic=true;
		  llh_f(s->x, &vpar);
		  parinic=false;
		}
		if(cpdf::verbose>0) printf("Minimum found at:  ");

		for(i=0; i<vnum; i++) gsl_vector_set(x, i, gsl_vector_get(s->x, i));

		llh=s->fval;
		vres=gettmp_inic(vpar, x);

		if(cpdf::verbose>0) printf("%d f=%g t0=%g z0=%g th=%g dd=%g   %g\n", i, llh, vres.t0, vres.z0, vres.th, vres.dd, dist);

		gsl_multimin_fminimizer_free(s);
		gsl_vector_free(step);
	      }

	      llh_e=llh/nllr;
	      if(vnum==vnux){
		extf=false;
		llh=llh_f(x, &vpar);
		extf=true;
	      }

	      double cm=cpdf::c/ppf[freco-1]->ng;
	      vres.norm(1);
	      settmp_inic(x, vres);

	      vtemp vmin, vmax;
	      if(256 & get_rmsk()){
		vmin.t0=vres.t0-1.e3;
		vmax.t0=vres.t0+1.e3;
		vmin.z0=vres.z0-1.e3;
		vmax.z0=vres.z0+1.e3;
		vmin.th=!cpdf::updown && freco==1 && rspe ?90:0;
		vmax.th=180;
		vmin.dd=vres.dd<0?vres.dd-1.e2:0;
		vmax.dd=vres.dd>0?vres.dd+1.e2:0;
		vmin.nn=vres.nn/10;
		vmax.nn=vres.nn*10;
		vmin.ph=vres.ph-60;
		vmax.ph=vres.ph+60;
		vmin.az=vres.az-60;
		vmax.az=vres.az+60;

		gsl_vector *gmin=NULL, *gmax=NULL;
		settmp_inic(gmin, vmin);
		settmp_inic(gmax, vmax);

		errors(&vres, llh_e+0.5, x, gmin, gmax);

		vmin=gettmp_inic(vres, gmin);
		vmax=gettmp_inic(vres, gmax);

		gsl_vector_free(gmin);
		gsl_vector_free(gmax);
	      }

	      new_reco.type=freco;
	      new_reco.hits=inice;
	      new_reco.wforms=inice_ijs;
	      new_reco.hitdoms=inice_num;
	      new_reco.strings=icsn;
	      new_reco.rllh=llh/(zamp*nllr);

	      vres.norm(2);
	      new_reco.t0=vres.t0-fabs(vres.dd)/cm;
	      new_reco.z0=vres.z0;
	      new_reco.th=vres.th;
	      new_reco.dd=vres.dd*(freco==1?vres.ddf:1);
	      new_reco.ph=vres.ph;
	      new_reco.az=vres.az;
	      new_reco.x0=vres.x0;
	      new_reco.y0=vres.y0;
	      new_reco.ddf=vres.ddf;
	      new_reco.dir=vres.th<90?-1:1;

	      if(512 & get_rmsk()) if((freco==1) && ((!rspe) || ((!rfx) && cpdf::updown))) if(new_reco.th>cpdf::thmax){ tflag=true; e.inic=0; break; }

	      reco_e(vres, &new_reco, j>0?vres.nn:0);
	      if(j>0) new_reco.n0=vres.nn; else vres.nn=new_reco.n0;

	      if(j==-1){
		parinic=true;
		reco_e(vres, &new_reco, new_reco.n0);
		parinic=false;
	      }

	      new_reco.en=vnum==vnux?1:0;
	      new_reco.sp=rspe?1:0;

	      if(256 & get_rmsk()){
		vmin.norm(2);
		vmax.norm(2);
		gsl_vector *gaux;

		gaux=NULL; settmp_inic(gaux, vmin);
		vmin=gettmp_inic(vres, gaux);
		gsl_vector_free(gaux);

		gaux=NULL; settmp_inic(gaux, vmax);
		vmax=gettmp_inic(vres, gaux);
		gsl_vector_free(gaux);

		new_reco.t0_min=vmin.t0-fabs(vres.dd)/cm;
		new_reco.t0_max=vmax.t0-fabs(vres.dd)/cm;
		new_reco.z0_min=vmin.z0;
		new_reco.z0_max=vmax.z0;
		new_reco.th_min=vmin.th;
		new_reco.th_max=vmax.th;
		new_reco.dd_min=vmin.dd*(freco==1?vres.ddf:1);
		new_reco.dd_max=vmax.dd*(freco==1?vres.ddf:1);
		order(new_reco.dd_min, new_reco.dd_max);
		new_reco.n0_min=vmin.nn;
		new_reco.n0_max=vmax.nn;
		new_reco.ph_min=vmin.ph;
		new_reco.ph_max=vmax.ph;
		new_reco.az_min=vmin.az;
		new_reco.az_max=vmax.az;
		evts->tracks.push_back(new_reco);
	      }
	    }

	    if(vnus>0 && s<vnus) vpar=gettmp_inic(vpar, x);  // not used
	    gsl_vector_free(x);

	    if(rfx) if(vnus){ rfx_th=s?0:new_reco.th; rfx_ph=s?0:new_reco.ph; }
	    if(tflag) break;
	  }
	  if(tflag) break;

	  // cout<<"photons="<<nn<<endl;
	  if(!(256 & get_rmsk())) evts->tracks.push_back(new_reco);
	}
      }
    }

    if(get_itop()>0 || get_itsn()>0){
      int itsn=0, icetop=0, icetop_ijs=0, icetop_num=0;
      double zamp=0;
      double x0=0, y0=0, z0=0, t0=0;

      map<unsigned long long, int> mltp;
      map<int, int> str;
      for(vector<event>::const_iterator it = evts->events.begin(); it!=evts->events.end(); ++it){
	map<unsigned long long, mydom>::const_iterator cdom = idoms().find(it->id);
	const mydom& dom = cdom->second;
	int om = dom.om;
	if(om>=61 && om<=64) if(it->Q>=0 && it->fi<=0){
	  if(it->ijs==0){ icetop_ijs++; mltp[it->id]++; str[dom.str]++; }
	  icetop++;
	  if(doreco){
	    zamp+=it->Q;
	    t0+=(((long long)(it->gt-evts->t0))/10.)*it->Q;
	    x0+=dom.x*it->Q;
	    y0+=dom.y*it->Q;
	    z0+=dom.z*it->Q;
	  }
	}
      }
      itsn=str.size();
      e.itsn=itsn;
      icetop_num=mltp.size();
      e.itop=icetop_num;
      if(doreco && icetop_num>=get_itop() && itsn>=get_itsn() && 4 & get_rmsk()){
	freco=3;
	vnum=3;
	nllr=(get_rmsk()&64?zamp:icetop)/icetop;

	int i;
	t0/=zamp;
	x0/=zamp;
	y0/=zamp;
	z0/=zamp;
	llh=&tllh;

	double llh, llh_e;
	otemp ores;

	otemp opar;
	opar.tc=t0;
	opar.x0=x0;
	opar.y0=y0;
	opar.z0=z0;
	opar.t0=0;
	opar.th=0;
	opar.ph=0;
	opar.a0=0;
	opar.k0=0;

	gsl_vector *x=NULL;
	settmp_itop(x, opar);

	int status;
	int vnui=get_rmsk()&2048?(itopreco==2&&itopdok0?3:2):0;
	if(get_rmsk()&1024) vnui=0;

	for(int j=0; j<=vnui; j++){
	  if(j>0){
	    gettmp_itop(ores, x);
	    gsl_vector_free(x);
	    x=NULL;
	    vnum=4+j;
	    settmp_itop(x, ores);
	  }

	  if(vnum<=3){
	    const gsl_multimin_fdfminimizer_type *T;
	    gsl_multimin_fdfminimizer *s;

	    gsl_multimin_function_fdf reco_f;

	    reco_f.f = &llh_f;
	    reco_f.df = &llh_df;
	    reco_f.fdf = &llh_fdf;
	    reco_f.n = vnum;
	    reco_f.params = &opar;

	    switch(1){
	    case 1: T = gsl_multimin_fdfminimizer_conjugate_fr; break;
	    case 2: T = gsl_multimin_fdfminimizer_conjugate_pr; break;
	    case 3: T = gsl_multimin_fdfminimizer_vector_bfgs; break;
	    case 4: T = gsl_multimin_fdfminimizer_steepest_descent; break;
	    }
	    s = gsl_multimin_fdfminimizer_alloc(T, vnum);

	    gsl_multimin_fdfminimizer_set(s, &reco_f, x, 10, 1.e-4);

	    for(i=0; i<200; i++){
	      status = gsl_multimin_fdfminimizer_iterate(s);
	      if(status) break;
	      status = gsl_multimin_test_gradient(s->gradient, 1.e-5);
	      if(i>=10 && status == GSL_SUCCESS) break;
	    }

	    if(cpdf::verbose>0) printf("Minimum found at:  ");

	    for(i=0; i<vnum; i++) gsl_vector_set(x, i, gsl_vector_get(s->x, i));

	    llh=s->f;
	    ores=gettmp_itop(opar, x);

	    if(cpdf::verbose>0) printf("%d f=%g t0=%g th=%g ph=%g   %g %g %g\n", i, llh, ores.t0, ores.th, ores.ph,
				       gsl_vector_get(s->gradient, 0), gsl_vector_get(s->gradient, 1),
				       gsl_vector_get(s->gradient, 2));

	    gsl_multimin_fdfminimizer_free(s);
	  }
	  else{
	    const gsl_multimin_fminimizer_type *T;
	    gsl_multimin_fminimizer *s;

	    double vstep[7]={10, 10, 10, 10, 10, 1, 1};
	    gsl_vector *step;
	    step = gsl_vector_alloc(vnum);
	    for(i=0; i<vnum; i++) gsl_vector_set(step, i, vstep[i]);

	    gsl_multimin_function reco_f;

	    reco_f.f = &llh_f;
	    reco_f.n = vnum;
	    reco_f.params = &opar;

	    T = gsl_multimin_fminimizer_nmsimplex;
	    s = gsl_multimin_fminimizer_alloc(T, vnum);

	    gsl_multimin_fminimizer_set(s, &reco_f, x, step);
	    double dist;

	    for(i=0; i<1000; i++){
	      int status = gsl_multimin_fminimizer_iterate(s);
	      if(status) break;

	      dist=gsl_multimin_fminimizer_size(s);
	      status = gsl_multimin_test_size(dist, 1.e-4);

	      if(status == GSL_SUCCESS) break;
	    }
	    gsl_vector_free(step);

	    if(j==-1){
	      paritop=true;
	      llh_f(s->x, &opar);
	      paritop=false;
	    }

	    if(cpdf::verbose>0) printf("Minimum found at:  ");

	    for(i=0; i<vnum; i++) gsl_vector_set(x, i, gsl_vector_get(s->x, i));

	    llh=s->fval;
	    ores=gettmp_itop(opar, x);

	    gsl_multimin_fminimizer_free(s);
	  }
	  llh_e=llh/nllr;

	  ores.norm(1);
	  settmp_itop(x, ores);

	  double th=ores.th*M_PI/180;
	  double ph=ores.ph*M_PI/180;
	  double sinth=sin(th);
	  double costh=cos(th);
	  double sinph=sin(ph);
	  double cosph=cos(ph);
	  double nx=sinth*cosph;
	  double ny=sinth*sinph;
	  double nz=costh;
	  double cdel=ores.dir*(ores.x0*nx+ores.y0*ny+ores.z0*nz)/cpdf::c;

	  preco new_reco;
	  // new_reco.gt=evts->t0;
	  new_reco.type=freco;
	  new_reco.hits=icetop;
	  new_reco.wforms=icetop_ijs;
	  new_reco.hitdoms=icetop_num;
	  new_reco.strings=itsn;
	  new_reco.rllh=sqrt(llh/(icetop*nllr));
	  new_reco.t0=ores.tc+ores.t0;
	  new_reco.dd=ores.tc+ores.t0-cdel;
	  new_reco.th=ores.th;
	  new_reco.ph=ores.ph;
	  new_reco.x0=ores.x0;
	  new_reco.y0=ores.y0;
	  new_reco.z0=ores.z0;
	  new_reco.dir=ores.dir;

	  reco_e(ores, &new_reco, 0);
	  if(j>0){
	    new_reco.n0=exp(tAmean);
	    new_reco.ne=tArms;
	  }

	  if(256 & get_rmsk()){
	    otemp omin, omax;

	    omin.t0=ores.t0-1.e3;
	    omax.t0=ores.t0+1.e3;
	    omin.th=max(ores.th-20, 0.);
	    omax.th=min(ores.th+20, 180.);
	    omin.ph=ores.ph-60;
	    omax.ph=ores.ph+60;
	    omin.x0=ores.x0-1.e4;
	    omax.x0=ores.x0+1.e4;
	    omin.y0=ores.y0-1.e4;
	    omax.y0=ores.y0+1.e4;
	    omin.a0=ores.a0-1.e3;
	    omax.a0=ores.a0+1.e3;
	    omin.k0=ores.k0-1.e3;
	    omax.k0=ores.k0+1.e3;

	    gsl_vector *gmin=NULL, *gmax=NULL;
	    settmp_itop(gmin, omin);
	    settmp_itop(gmax, omax);

	    errors(&ores, llh_e*exp(0.5), x, gmin, gmax);

	    omin=gettmp_itop(ores, gmin);
	    omax=gettmp_itop(ores, gmax);

	    gsl_vector_free(gmin);
	    gsl_vector_free(gmax);

	    gsl_vector *gaux;

	    gaux=NULL; settmp_itop(gaux, omin);
	    omin=gettmp_itop(ores, gaux);
	    gsl_vector_free(gaux);

	    gaux=NULL; settmp_itop(gaux, omax);
	    omax=gettmp_itop(ores, gaux);
	    gsl_vector_free(gaux);

	    omin.norm(2);
	    omax.norm(2);

	    new_reco.t0_min=ores.tc+omin.t0;
	    new_reco.t0_max=ores.tc+omax.t0;
	    new_reco.z0_min=omin.z0;
	    new_reco.z0_max=omax.z0;
	    new_reco.dd_min=ores.tc+omin.t0-cdel;
	    new_reco.dd_max=ores.tc+omax.t0-cdel;
	    new_reco.th_min=omin.th;
	    new_reco.th_max=omax.th;
	    new_reco.ph_min=omin.ph;
	    new_reco.ph_max=omax.ph;
	    new_reco.x0_min=omin.x0;
	    new_reco.x0_max=omax.x0;
	    new_reco.y0_min=omin.y0;
	    new_reco.y0_max=omax.y0;
	    new_reco.n0_min=omin.a0;
	    new_reco.n0_max=omax.a0;
	    new_reco.az_min=omin.k0;
	    new_reco.az_max=omax.k0;
	  }

	  ores.norm(2);
	  new_reco.nl=ores.a0;
	  new_reco.az=ores.k0;

	  new_reco.en=j;
	  new_reco.sp=get_rmsk()&1024?1:0;
	  if(256 & get_rmsk() || j==vnui) evts->tracks.push_back(new_reco);
	}
	gsl_vector_free(x);
      }
    }

    if(doreco) if(get_rmsk()&32) qdoms.erase(qdoms.begin(), qdoms.end());
    return;
  }

  // holds angular sensitivity parameterization of P. Miocinovich
  struct sens{
    double ave;
    double ped_pmt[11];

    double s(double coseta){
      double y=1, sum=ped_pmt[0];
      for(int i=1; i<=10; i++){ y*=coseta; sum+=ped_pmt[i]*y; }
      return sum;
    }

    sens(){
      static double ped_pmt[11]={0.447487, 0.45995, -0.0644593, 0.777876, 0.325881, -1.07997, -0.17585, -0.178416, -0.469926, 0.524479, 0.444801};
      for(int i=0;i<11;i++) this->ped_pmt[i]=ped_pmt[i];
      double sum=ped_pmt[0];
      for(int i=2; i<=10; i+=2){ sum+=ped_pmt[i]/(i+1); }
      ave=sum; // average sensitivity
    }
  } asens;

  struct spq{
    double p, q, r;
  };

  // calculates energy likelihood (phit*pnohit)
  void reco_e(vtemp& vtmp, preco *r, double n){
    double result=0, dresult=0;
    int empty=0, xempty=0;

    if((get_inic()>0 || get_icsn()>0) && ( ((freco==1) && (1&get_rmsk())) || ((freco==2) && (2&get_rmsk())) )){
      double z0=vtmp.z0;
      double th=vtmp.th*M_PI/180;
      double ph=vtmp.ph*M_PI/180;
      double az=vtmp.az*M_PI/180;
      double dd=vtmp.dd;
      double x0=vtmp.x0;
      double y0=vtmp.y0;

      short ddf=vtmp.ddf;

      double sinth=sin(th);
      double costh=cos(th);

      double coschr=1/ppf[freco-1]->np;
      double sinchr=sqrt(1-coschr*coschr);

      double ddp=dd;
      double sinph=sin(ph);
      double cosph=cos(ph);

      static Miniball<3> ball;
      static map<int, map<int, double> > photons;
      static map<int, pair<int, int> > ranges, qranges;
      double Etot=0, Ntot=0, E2tot=0, ENtot=0, N2tot=0, Ltot=0;

      if(reco_reset){
	if(cpdf::mnball) ball=Miniball<3>();
	ranges=map<int, pair<int, int> >();
	qranges=map<int, pair<int, int> >();
	photons=map<int, map<int, double> >();

	for(vector<event>::const_iterator it = evts->events.begin(); it!=evts->events.end(); ++it){
	  map<unsigned long long, mydom>::const_iterator cdom = idoms().find(it->id);
	  const mydom& dom = cdom->second;
	  int om = dom.om;
	  int str = dom.str;
	  if(om>=1 && om<=60) if(0==get_strn() || str==get_strn()) if(it->Q>=0 && it->fi<=0){
	    if(photons[str].find(om)==photons[str].end()){
	      if(cpdf::mnball){
		Point<3> p;
		p[0]=dom.x;
		p[1]=dom.y;
		p[2]=dom.z;
		ball.check_in(p);
	      }
	      /*
	      if(it->wf->V>VSAT){
		photons[str][om]+=dom.st;
		if(cpdf::verbose>0) cout<<"Warning: voltage of "<<(it->wf->V)<<" > "<<VSAT<<" Volts was detected in the last ATWD bin. Saturation amount of charge "<<dom.st<<" added"<<endl;
	      }
	      */
	    }
	    photons[str][om]+=it->Q;
	  }
	}
	if(cpdf::mnball){
	  ball.build();
	  Point<3> c=ball.center();
	  double cx=c[0], cy=c[1], cz=c[2];
	  double r2=ball.squared_radius();
	  for(map<unsigned long long, mydom>::const_iterator cdom = idoms().begin(); cdom!=idoms().end(); ++cdom){
	    const mydom& dom = cdom->second;
	    if(0==get_strn() || dom.str==get_strn()) if(dom.hv>0){
	      double dx=dom.x-cx, dy=dom.y-cy, dz=dom.z-cz;
	      if(dx*dx+dy*dy+dz*dz<r2) photons[dom.str][dom.om]+=0;
	    }
	  }
	}

	for(map<int, map<int, double> >::const_iterator it_str = photons.begin(); it_str!=photons.end(); ++it_str){

	  int om_imin=0, om_imax=0, om_qmin=0, om_qmax=0, om_min, om_max, str=it_str->first;

	  for(map<int, double>::const_iterator it = photons[str].begin(); it!=photons[str].end(); ++it){
	    int om = it->first;

	    if(om_imin==0 || om<om_imin) om_imin=om;
	    if(om_imax==0 || om_imax<om) om_imax=om;
	    if(it->second>0){
	      if(om_qmin==0 || om<om_qmin) om_qmin=om;
	      if(om_qmax==0 || om_qmax<om) om_qmax=om;
	    }
	  }

	  om_min=max(om_imin-cpdf::om_pad, 1);
	  om_max=min(om_imax+cpdf::om_pad, min(mystrings().find(str)->second, 60));
	  ranges[str]=make_pair(om_min, om_max);
	  qranges[str]=make_pair(om_qmin, om_qmax);
	}
	reco_reset=false;
      }

      for(map<int, map<int, double> >::const_iterator it_str = photons.begin(); it_str!=photons.end(); ++it_str){
	map<int,spq> pq;
	int str=it_str->first;
	int om_min=ranges[str].first, om_max=ranges[str].second;
	int om_imin=qranges[str].first, om_imax=qranges[str].second;

	for(int om=om_min; om<=om_max; om++){{
	  mykey key;
	  key.om=om;
	  key.str=str;
	  map<mykey, unsigned long long>::const_iterator it_key=mykeys().find(key);
	  if(it_key==mykeys().end()){
	    if(cpdf::verbose>0) cout<<"Warning (reco): Geometry inconsistency detected"<<endl;
	    continue;
	  }
	  map<unsigned long long, mydom>::const_iterator cdom = idoms().find(it_key->second);
	  const mydom& dom = cdom->second;
	  if(dom.hv<=0) continue;

	  double q=0;
	  {
	    map<int, double>::const_iterator it=photons[str].find(om);
	    if(it!=photons[str].end()) q=it->second;
	    else if(cpdf::nnhit) continue;
	  }

	  int om = dom.om;
	  double dz=dom.z-z0;
	  double da, db, lx, sz, sqrtd=0, d=0, coseta=0, dez=0;
	  switch(freco){
	  case 1:
	    if(0==get_strn()){
	      double dx=dom.x-x0;
	      double dy=dom.y-y0;
	      double dT=dx*cosph+dy*sinph;
	      dx-=-ddp*sinph*ddf+dT*cosph;
	      dy-=ddp*cosph*ddf+dT*sinph;
	      dd=sqrt(dx*dx+dy*dy);
	      if(sinth!=0){
		dT/=sinth;
		dz-=costh*dT;
	      }
	    }
	    sz=dz*sinth;
	    sqrtd=sqrt(dd*dd+sz*sz);
	    coseta=coschr*costh+sinchr*sinth*(sqrtd>cpdf::pre?sz/sqrtd:1);
	    d=sqrtd+cpdf::a2*(cpdf::a00+cpdf::a01*coseta+cpdf::a02*coseta*coseta);
	    if(!ppf[freco-1]->cpandel_flag) dez=coseta*(sqrtd/sinchr); else dez=0;
	    break;
	  case 2:
	    double cospsi=0;
	    if(0==get_strn()){
	      double dx=dom.x-x0;
	      double dy=dom.y-y0;
	      dx-=ddp*cosph;
	      dy-=ddp*sinph;
	      dd=sqrt(dx*dx+dy*dy);
	      if(!rspe){
		double sinaz=sin(az);
		double cosaz=cos(az);
		double nx=sinth*cosaz, ny=sinth*sinaz;
		cospsi=nx*dx+ny*dy;
	      }
	    }
	    if(!rspe){
	      ppf[freco-1]->lx(-z0);
	      lx=ppf[freco-1]->l*cpdf::le+cpdf::elen;
	    }
	    else lx=0;
	    sqrtd=sqrt(dz*dz+dd*dd);
	    if(!rspe) cospsi=(cospsi+costh*dz)/sqrtd;
	    coseta=(sqrtd>cpdf::pre?dz/sqrtd:1);
	    d=max(0., sqrtd+cpdf::a2*(cpdf::a00+cpdf::a01*coseta+cpdf::a02*coseta*coseta)-lx*cospsi);
	    if(!ppf[freco-1]->cpandel_flag) dez=dz; else dez=0;
	    break;
	  }
	  switch(1){
	  case 1: da=d; db=d; break;
	  default: da=sqrtd; db=sqrtd; break;
	  }
	  if(db<cpdf::at) db=cpdf::at;
	  ppf[freco-1]->ll(-dom.z, -dom.z+dez);
	  double p=0;
	  double le=ppf[freco-1]->l;
	  double la=ppf[freco-1]->la;
	  double lp=sqrt(le*la/3);
	  double coschr=1/ppf[freco-1]->np;
	  double sinchr=sqrt(1-coschr*coschr);
	  double lg, alf=0;
	  switch(freco){
	  case 1:
	    lg=(4*le/3)*exp(da*(1/lp-1/(la*sinchr)));
	    if(lg+1!=lg){
	      lg*=lg*(2/M_PI*lp);
	      p=da>0?exp(-da/(la*sinchr))/(2*M_PI*sqrt(lg*db)*tanh(sqrt(db/lg))):0;
	    }
	    else p=0;
	    alf=1;
	    break;
	  case 2:
	    lg=la/le;
	    alf=(sqrt(3*lg)-1)/lg;
	    lg=(da/le)*alf;
	    p=da>0?(exp(-da/la)/(4*M_PI*db*db))*(lg/sinh(lg)):0;
	    alf=3/(8*alf);
	    break;
	  }

	  double y=sinh(d/(M_PI*le)), sens=asens.s(coseta), far=alf*4*asens.ave;
	  sens=y+1!=y?(sens+far*y)/(1+y):far;
	  p*=sens;

	  spq spqf;
	  spqf.p=p;
	  spqf.q=q;
	  spqf.r=sqrtd*(dz<0?-1:1);
	  pq.insert(make_pair(om, spqf));
	}}

	for(int om=om_min; om<=om_max; om++){
	  mykey key;
	  key.om=om;
	  key.str=str;
	  map<mykey, unsigned long long>::const_iterator it_key=mykeys().find(key);
	  if(it_key==mykeys().end()) continue;
	  map<unsigned long long, mydom>::const_iterator cdom = idoms().find(it_key->second);
	  const mydom& dom = cdom->second;
	  if(dom.hv<=0) continue;
	  if(cpdf::nnhit) if(photons.find(it_key->second)==photons.end()) continue;

	  spq& spqf=pq.find(om)->second;
	  double p=spqf.p, q=spqf.q, r=spqf.r, ppd=1;

	  if(cpdf::lcflag) if(n>0){
	    double p_ud=pq[om-1].p+pq[om+1].p;
	    if(p_ud>0){
	      p_ud*=n;
	      double aux=p_ud<1.e-5?p_ud:1-exp(-p_ud);
	      p*=aux;
	      ppd+=(1-aux)*p_ud/aux;
	    }
	  }

	  p=max(p, cpdf::pmin);
	  if(p<=0){
	    if(cpdf::verbose>0) cout<<"Warning (reco): probability is not positive: "<<p<<endl;
	    continue;
	  }
	  if(q>dom.st){
	    if(cpdf::verbose>0) cout<<"Warning (reco): charge above saturation: "<<q<<" > "<<dom.st<<endl;
	    continue;
	  }
	  if(parinic) cout<<"parinie "<<r<<" "<<q<<" "<<p<<endl;

	  double er=0.1;  // 10% systematic error belt. Calculation only valid for er<<1. Set to 0 to disable
	  Etot+=q;
	  Ntot+=p;
	  E2tot+=q*q;
	  ENtot+=q*p;
	  N2tot+=p*p;
	  if(n>0){
	    if(er>0){
	      double yux=(q-n*p)*er;
	      double aux=abs(yux);
	      double f, g;
	      if(aux<1.e-5){
		f=yux*yux/6;
		g=yux/3;
	      }
	      else if(aux>10){
		f=aux-log(2*aux);
		g=(aux-1)/yux;
	      }
	      else{
		f=log(sinh(yux)/yux);
		g=1/tanh(yux)-1/yux;
	      }
	      result+=q*log(n*p)-n*p+f;
	      dresult+=ppd*(q/n-p-er*p*g);
	    }
	    else{
	      result+=q*log(n*p)-n*p;
	      dresult+=ppd*(q/n-p);
	    }
	  }
	  else{
	    if(q==0 && om_imin<=om && om<=om_imax){
	      empty++;
	      if(empty>xempty) xempty=empty;
	    }
	    else empty=0;
	    // cout<<"per DOM ("<<str<<","<<om<<") "<<r<<" "<<(q/p)<<endl;
	  }
	}
      }

      if(Ntot>0){
	Ltot=-result/Etot;
	Etot/=Ntot;
	E2tot=sqrt(E2tot-2*Etot*ENtot+Etot*Etot*N2tot)/Ntot;
      }
      else{
	Ltot=0;
	Etot=0;
	E2tot=0;
      }
      if(parinic) cout<<"parinie "<<Etot<<endl;

      if(r!=NULL){
	r->n0=Etot;
	r->ne=E2tot;
	r->nl=n>0?Ltot:xempty;
      }
    }

    lle=result;
    dlle=dresult;
  }

  void reco_e(otemp& otmp, preco *r, double n){
    if((get_itop()>0 || get_itsn()>0) && (freco==3 && 4&get_rmsk())){
      double Etot=0, Ntot=0, E2tot=0, ENtot=0, N2tot=0;

      for(vector<event>::const_iterator it = evts->events.begin(); it!=evts->events.end(); ++it){
	map<unsigned long long, mydom>::const_iterator cdom = idoms().find(it->id);
	const mydom& dom = cdom->second;
	int om = dom.om;
	if(om>=61 && om<=64) if(it->Q>=0 && it->fi<=0){
	  double q=it->Q;
	  double vef=pepp(dom);
	  Etot+=q/vef;
	  Ntot++;
	  E2tot+=q*q/(vef*vef);
	}
      }

      if(Ntot>0){
	ENtot=Etot;
	N2tot=Ntot;
	Etot/=Ntot;
	E2tot=sqrt(E2tot-2*Etot*ENtot+Etot*Etot*N2tot)/Ntot;
      }
      else{
	Etot=0;
	E2tot=0;
      }

      if(r!=NULL){
	r->n0=Etot;
	r->ne=E2tot;
	r->nl=0;
      }
    }
  }

  void inimctrack(multe& evts, preco& track){
    if(track.type!=-1) return;
    double xs=0, ys=0, zs=0, qs=0;
    for(vector<event>::const_iterator it = evts.events.begin(); it!=evts.events.end(); ++it){
      map<unsigned long long, mydom>::const_iterator cdom = idoms().find(it->id);
      const mydom& dom = cdom->second;
      int om = dom.om;
      if(om>=1 && om<=60) if(0==get_strn() || dom.str==get_strn()) if(it->Q>=0 && it->fi<=0){
	xs+=dom.x*it->Q;
	ys+=dom.y*it->Q;
	zs+=dom.z*it->Q;
	qs+=it->Q;
      }
    }
    if(qs>0){
      xs/=qs;
      ys/=qs;
#ifdef MMCRECO
      zs/=qs;
#endif
    }

    double sinth=sin(track.th*M_PI/180);
    double costh=cos(track.th*M_PI/180);
    double sinph=sin(track.ph*M_PI/180);
    double cosph=cos(track.ph*M_PI/180);

    double nx=sinth*cosph;
    double ny=sinth*sinph;
    double nz=costh; 

    double t0=track.t0;
    double x0=track.x0;
    double y0=track.y0;
    double z0=track.z0;

    double tn, dx, dy, zn, dn, n2;
    n2=nx*nx+ny*ny;

    double sol=cpdf::c;
    tn=t0+(n2>0?(nx*(xs-x0)+ny*(ys-y0))/n2:0)/sol;
    dx=x0+nx*(tn-t0)*sol-xs;
    dy=y0+ny*(tn-t0)*sol-ys;
    zn=z0+nz*(tn-t0)*sol;
    dn=sqrt(dx*dx+dy*dy);

    track.t0=tn;
    track.z0=zn;
    track.x0=xs;
    track.y0=ys;

    track.ddf=nx*dy-ny*dx>0?1:-1;
    track.dd=dn*track.ddf;

    reco_e(evts, track, 0);

#ifdef MMCRECO
    tn=t0+(nx*(xs-x0)+ny*(ys-y0)+nz*(zs-z0))/sol;
    eloss& e=track.els;
    if(e.Ei>0){
      double totE=0, sumE=0;
      for(vector<double>::const_iterator it=e.t.begin(), ie=e.E.begin(); it!=e.t.end(); it++, ie++){
	totE+=*ie;
	if(*it<tn) sumE+=*ie;
      }
      double sl0=e.tf>e.ti?((e.Ef-e.Ei+totE)/(e.ti-e.tf)):0;
      double slmax=(0.21+8.8e-3*log(e.Ei)/log(10.))*sol;  // for ice only at Ecut=500 MeV
      track.nl=e.Ei-sumE-min(sl0, slmax)*max(tn-e.ti, 0.);
    }
    else track.nl=0;
#else
    track.nl=0;
#endif
  }

  // prints reconstruction results
  ostream& operator<<(ostream& out, const preco& it){
    bool s=true;  // print x0, y0 at z0 along the track or cascade instead of COG x, y
    switch(it.type){
    case 1:
      out << "track: rllh=" << it.rllh << " (" << it.strings << "," << it.hitdoms << "," << it.wforms << "," << it.hits
	  << ") t0=" << it.t0 << " x0=" << it.x0-(s?it.dd*sin(it.ph*M_PI/180):0)
	  << " y0=" << it.y0+(s?it.dd*cos(it.ph*M_PI/180):0) << " z0=" << it.z0
	  << " th=" << it.th << " ph=" << it.ph << " dd=" << it.dd << " n=" << it.n0 << "+-" << it.ne
	  << " nl=" << it.nl << "  sp=" << it.sp << " en=" << it.en << " dir=" << it.dir << endl;
      if(256 & get_rmsk()){
	// if(get_text()==7) out << "!";
	out
	  << "\terrors:"
	  << " t0=(" << it.t0_min << "," << it.t0_max << ")" << " z0=(" << it.z0_min << "," << it.z0_max << ")"
	  << " th=(" << it.th_min << "," << it.th_max << ")" << " ph=(" << it.ph_min << "," << it.ph_max << ")"
	  << " dd=(" << it.dd_min << "," << it.dd_max << ")" << " n0=(" << it.n0_min << "," << it.n0_max << ")" << endl;
      }
      break;
    case 2:
      out << "cascade: rllh=" << it.rllh << " (" << it.strings << "," << it.hitdoms << "," << it.wforms << "," << it.hits
	  << ") t0=" << it.t0 << " x0=" << it.x0+(s?it.dd*cos(it.ph*M_PI/180):0)
	  << " y0=" << it.y0+(s?it.dd*sin(it.ph*M_PI/180):0) << " z0=" << it.z0
	  << " th=" << it.th << " az=" << it.az << " ph=" << it.ph << " dd=" << it.dd
	  << " n=" << it.n0 << "+-" << it.ne << " nl=" << it.nl
	  << "  sp=" << it.sp << " en=" << it.en << " dir=" << it.dir << endl;
      if(256 & get_rmsk()){
	// if(get_text()==7) out << "!";
	out
	  << "\terrors:"
	  << " t0=(" << it.t0_min << "," << it.t0_max << ")" << " z0=(" << it.z0_min << "," << it.z0_max << ")"
	  << " th=(" << it.th_min << "," << it.th_max << ")" << " az=(" << it.az_min << "," << it.az_max << ")"
	  << " ph=(" << it.ph_min << "," << it.ph_max << ")" << " dd=(" << it.dd_min << "," << it.dd_max << ")"
	  << " n0=(" << it.n0_min << "," << it.n0_max << ")" << endl;
      }
      break;
    case 3:
      out << "shower: rchi=" << it.rllh << " (" << it.strings << "," << it.hitdoms << "," << it.wforms << "," << it.hits
	  << ") ta=" << it.t0 << " t0=" << it.dd << " th=" << it.th << " ph=" << it.ph
	  << " core x=" << it.x0 << " y=" << it.y0 << " z=" << it.z0 << " n=" << it.n0 << "+-" << it.ne
	  << "  sp=" << it.sp << " en=" << it.en << " age=" << it.nl << " k=" << it.az << " dir=" << it.dir << endl;
      if(256 & get_rmsk()){
	// if(get_text()==7) out << "!";
	out
	  << "\terrors:" << " t0=(" << it.t0_min << "," << it.t0_max << ")"
	  << " th=(" << it.th_min << "," << it.th_max << ")" << " ph=(" << it.ph_min << "," << it.ph_max << ")"
	  << " x0=(" << it.x0_min << "," << it.x0_max << ")" << " y0=(" << it.y0_min << "," << it.y0_max << ")"
	  << " age=(" << it.n0_min << "," << it.n0_max << ")" << " k=(" << it.az_min << "," << it.az_max << ")" << endl;
      }
      break;
    case -1:
      out << "muon: e=" << it.rllh << " (" << it.strings << "," << it.hitdoms << "," << it.wforms << "," << it.hits
	  << ") t0=" << it.t0 << " x0=" << it.x0-(s?it.dd*sin(it.ph*M_PI/180):0)
	  << " y0=" << it.y0+(s?it.dd*cos(it.ph*M_PI/180):0) << " z0=" << it.z0
	  << " th=" << it.th << " ph=" << it.ph << " dd=" << it.dd << " n=" << it.n0 << "+-" << it.ne << " E=" << it.nl << endl;
      break;
    case 0:
      out << "primary: type=" << it.sp << " E=" << it.rllh << " t0=" << it.t0
	  << " x0=" << it.x0 << " y0=" << it.y0 << " z0=" << it.z0 << " th=" << it.th << " ph=" << it.ph
	  << " W=" << it.az << " B=" << it.az_min << " H=" << it.az_max << endl;
      break;
    default: out << "unknown reconstruction type" << endl;
    }
    return out;
  }

}
