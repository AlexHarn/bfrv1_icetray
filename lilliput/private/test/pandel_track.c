/* pandel_track.c  */
/* this functions implements various pandelfunctions for amanda ! */
/* Extracted from rdmc                                            */

#include <math.h>
#include <float.h>

#define RDMC_BIG 1e6                              /* a big value (in meter) */
#define RDMC_SPECIAL_NA -RDMC_BIG
#define RDMC_FINFTY FLT_MAX         /* singkle infinity */
#define C_VACUUM  2.997924e-1
#define N_ICE_G 1.35634  /* group  400 nm according to amanda abs */
#define DIST_MAGIC 100.

#define TD_TAU 556.7
#define TD_LAM 33.29
#define PS_TAU 450.
#define PS_LAM 47.
#define TD_ATT 98.0
#define TD_DIST_P1  0.8395 
#define TD_DIST_P0_CS0 3.094
#define TD_DIST_P0_CS1 -3.946
#define TD_DIST_P0_CS2 4.636
#define TD_SIGMA 15.0

/* attention  constants are in GeV but rdmc uses MeV */
#define TD_PH_EPS_PE0 1.978
#define TD_PH_EPS_PE1 0.2314e-3
#define TD_PH_EPS_ORI_N0   1.1932 
#define TD_PH_EPS_ORI_POW  -0.20084 
#define TD_PH_DIST_A 0.7886
#define TD_PH_DIST_B 0.1851
#define TD_PH_DIST_L 0.5967
#define TD_PH_DIST_E0 1409.

const double rdmc_c_ice_g = C_VACUUM/N_ICE_G; /* m/nsec;  speed in water */

double rdmc_pt_lgtd(double delay, double perp_dist, double cs_ori);
double rdmc_lgamma(double x);  /* logarithm of the gamma-function */
double rdmc_pt_lgtd_norm(double perp_dist, double cs_ori);
double rdmc_gamac(double a, double x); /* incomplete gamma function */
static void gcf(double *gammcf,double a,double x,double *gln);
static void gser(double *gamser,double a,double x,double *gln);

typedef struct {
  double td_tau;     /* tau for tracks */
  double td_lam;     /* lambda for tracks */
  double td_att;     /* Absorption lenght */
  double ps_tau;     /* tau for tracks */
  double ps_lam;     /* lambda for tracks */
  double td_sigma;   /* pmt jitter -> patched functions */
  double td_dist_p1; /* scale for the distance */
  double td_dist_p0_cs0; /* const for distance ped (P0) */
  double td_dist_p0_cs1; /* const for distance ped (P1*cs_ori) */
  double td_dist_p0_cs2; /* const for distance ped (P2*cs_ori^2) */
  double td_ph_eps_pe0;
  double td_ph_eps_pe1;
  double td_ph_eps_ori_n0;
  double td_ph_eps_ori_pow;
  double td_ph_dist_a;
  double td_ph_dist_b;
  double td_ph_dist_l;
  double td_ph_dist_e0;
} rdmc_pandel_par_t;

/* this is the local init variable, initilized the same as h0 */
static rdmc_pandel_par_t pp = {
  TD_TAU,
  TD_LAM,
  TD_ATT,
  PS_TAU, PS_LAM,
  TD_SIGMA,
  TD_DIST_P1,
  TD_DIST_P0_CS0,
  TD_DIST_P0_CS1,
  TD_DIST_P0_CS2,
  TD_PH_EPS_PE0,  TD_PH_EPS_PE1, TD_PH_EPS_ORI_N0, TD_PH_EPS_ORI_POW,
  TD_PH_DIST_A,  TD_PH_DIST_B,  TD_PH_DIST_L, TD_PH_DIST_E0
};

typedef struct {
 double td_lgtau;
 double td_tau_slew;
 double lg_td_tau_slew;
 double td_tau_scale;
 double td_sigma_d_2square;
 double td_t0;
 double td_t0_dsq4;
 double td_t0_dsq3;
 double td_t0_dsq2;
 double td_t0_dsq1;
 double td_ph_dist_lge0;
} rdmc_pandel_special_t;

static volatile rdmc_pandel_special_t pt_s=
{
  RDMC_SPECIAL_NA,
  RDMC_SPECIAL_NA,  /* 1+ cw*tau/att */
  RDMC_SPECIAL_NA,
  RDMC_SPECIAL_NA, /* 1/tau + cw/x0 */
  RDMC_SPECIAL_NA, 
  RDMC_SPECIAL_NA,
  RDMC_SPECIAL_NA,
  RDMC_SPECIAL_NA,
  RDMC_SPECIAL_NA,
  RDMC_SPECIAL_NA,
  RDMC_SPECIAL_NA
};

static volatile rdmc_pandel_special_t pp_s=
{
  RDMC_SPECIAL_NA,
  RDMC_SPECIAL_NA,  /* 1+ cw*tau/att */
  RDMC_SPECIAL_NA,
  RDMC_SPECIAL_NA, /* 1/tau + cw/x0 */
  RDMC_SPECIAL_NA, 
  RDMC_SPECIAL_NA,
  RDMC_SPECIAL_NA,
  RDMC_SPECIAL_NA,
  RDMC_SPECIAL_NA,
  RDMC_SPECIAL_NA,
  RDMC_SPECIAL_NA
};

/* global flag to mark if the structures are initilized */
static  int  do_init=1;

static  void td_init_special(void){
/* now the track stuff */
  pt_s.td_lgtau = log(pp.td_tau);
#if RDMC_OLD_REFIDX
  pt_s.td_tau_slew = 1.0 + pp.td_tau*c_water/pp.td_att;
#else
  pt_s.td_tau_slew = 1.0 + pp.td_tau*rdmc_c_ice_g/pp.td_att;
#endif
  pt_s.lg_td_tau_slew = log(pt_s.td_tau_slew);
#if RDMC_OLD_REFIDX
  pt_s.td_tau_scale = 1.0/pp.td_tau + c_water/pp.td_att;
#else
  pt_s.td_tau_scale = 1.0/pp.td_tau + rdmc_c_ice_g/pp.td_att;
#endif
  pt_s.td_t0=sqrt(2.0*M_PI)*pp.td_sigma;
  pt_s.td_sigma_d_2square = 1./(2.*pp.td_sigma*pp.td_sigma);

  pt_s.td_t0_dsq1=1./pt_s.td_t0;
  pt_s.td_t0_dsq2=pt_s.td_t0_dsq1/pt_s.td_t0;
  pt_s.td_t0_dsq3=pt_s.td_t0_dsq2/pt_s.td_t0;
  pt_s.td_t0_dsq4=pt_s.td_t0_dsq3/pt_s.td_t0;

  pt_s.td_ph_dist_lge0=log(pp.td_ph_dist_e0);

/* now the point source stuff */
  pp_s.td_lgtau = log(pp.ps_tau);
#if RDMC_OLD_REFIDX
  pp_s.td_tau_slew = 1.0 + pp.ps_tau*c_water/pp.td_att;
#else
  pp_s.td_tau_slew = 1.0 + pp.ps_tau*rdmc_c_ice_g/pp.td_att;
#endif
  pp_s.lg_td_tau_slew = log(pp_s.td_tau_slew);
#if RDMC_OLD_REFIDX
  pp_s.td_tau_scale = 1.0/pp.ps_tau + c_water/pp.td_att;
#else
  pp_s.td_tau_scale = 1.0/pp.ps_tau +rdmc_c_ice_g/pp.td_att;
#endif

  pp_s.td_t0=sqrt(2.0*M_PI)*pp.td_sigma;
  pp_s.td_sigma_d_2square = 1./(2.*pp.td_sigma*pp.td_sigma);

  pp_s.td_t0_dsq1=1./pp_s.td_t0;
  pp_s.td_t0_dsq2=pp_s.td_t0_dsq1/pp_s.td_t0;
  pp_s.td_t0_dsq3=pp_s.td_t0_dsq2/pp_s.td_t0;
  pp_s.td_t0_dsq4=pp_s.td_t0_dsq3/pp_s.td_t0;

  pp_s.td_ph_dist_lge0=log(pp.td_ph_dist_e0);

  do_init=0;
  return;
}

/***************************************************************************/
static double td_dist(double perp_dist, double cs_ori){
  static double r=-RDMC_BIG;
#if SPEED_UP
  static double ori=-RDMC_BIG,pd=-RDMC_BIG;
  if ((perp_dist == pd) && (ori== cs_ori) )
    return r;
  else{
    pd=perp_dist;
    ori=cs_ori;
  }
#endif
  r= pp.td_dist_p1*perp_dist
    + pp.td_dist_p0_cs0
      + pp.td_dist_p0_cs1*cs_ori
    + pp.td_dist_p0_cs2*cs_ori*cs_ori;
  return r;
}
/***************************************************************************/
static double ps_dist(double perp_dist, double cs_ori, double cs_axis){
  return perp_dist;
}
/***************************************************************************/
/* now the implementation of the track time-delay  */
/***************************************************************************/
double rdmc_pt_td(double delay, double perp_dist, double cs_ori){
  static double result=0.;

#if SPEED_UP
  static double pd=-RDMC_BIG,ori=-RDMC_BIG,td=-RDMC_BIG;
  if ( (delay == td) && (pd == perp_dist) && (cs_ori == ori))
    return result;
  else{
    td = delay;
    pd = perp_dist;
    ori = cs_ori;
  }
#endif

  result = rdmc_pt_lgtd(delay, perp_dist, cs_ori);
  if (result <= -RDMC_FINFTY)
    result = 0.;
  else
    result = exp(result);

  return result;
}


double rdmc_pt_lgtd(double delay, double perp_dist, double cs_ori){
  static double result=-RDMC_FINFTY;
  static double dist,rdist,lam;
#if SPEED_UP
  static double pd=-RDMC_BIG,ori=-RDMC_BIG,td=-RDMC_BIG;
#endif

  if ((delay <= 0.) || (perp_dist <= 0.) || (cs_ori >1.) || (cs_ori < -1.))
    return -RDMC_FINFTY;

  if (do_init)
    td_init_special();  

#if SPEED_UP
  if ( (delay == td) && (pd == perp_dist) && (cs_ori == ori))
    return result;
  else{
    td = delay;
    pd = perp_dist;
    ori = cs_ori;
  }
#endif
  lam = pp.td_lam;
  dist = td_dist(perp_dist,cs_ori);
  rdist = dist/lam;
  result = - delay*pt_s.td_tau_scale - dist/pp.td_att         /* exponential*/ 
    + log(delay)*(rdist-1.0) - pt_s.td_lgtau*rdist           /* potentials */ 
    - rdmc_lgamma(rdist)                              /* Gamma function */
    - rdmc_pt_lgtd_norm(perp_dist,cs_ori);           /* normalisation */

#if 0
    printf("\t\t %f %f %f %f\n", delay*td_tau_scale,dist/td_att,
    + log(delay)*(rdist-1.0) , td_lgtau*rdist);
#endif

  return result;
}

/***************************************************************************/
double rdmc_pt_td_int(double delay, double perp_dist, double cs_ori){
  double dist,rdist,lam;
#if 0
  double ldist;
#endif
  static double result=0.0;
#if SPEED_UP
  static double pd=-RDMC_BIG,ori=-RDMC_BIG,td=-RDMC_BIG;
#endif

  if (do_init)
    td_init_special();

  if ((delay <=0.) || (perp_dist <= 0.) || (cs_ori >1.) || (cs_ori < -1.))
    return 0.;

#if SPEED_UP
  if ( (delay == td) && (pd == perp_dist) && (cs_ori == ori))
    return result;
  else{
    td = delay;
    pd = perp_dist;
    ori = cs_ori;
  }
#endif


  lam = pp.td_lam;
  dist = td_dist(perp_dist,cs_ori);
  rdist = dist/lam;

#if 0 /* we want it normalized */
  ldist = dist/pp.td_att;
  result = - rdist*pt_s.lg_td_tau_slew - ldist;
  return  rdmc_gamac(rdist,delay*pt_s.td_tau_scale)*exp(result);
#endif

  result =  rdmc_gamac(rdist,delay*pt_s.td_tau_scale);
  return result;
}

/***************************************************************************/
double rdmc_pt_td_diff(double delay, double perp_dist, double cs_ori){
  static double result=0.0;
  double dist,rdist,tmp,lam;
#if SPEED_UP
  static double pd=-RDMC_BIG,ori=-RDMC_BIG,td=-RDMC_BIG;
#endif

  if (do_init)
    td_init_special();

  if ((delay <= 0.) || (perp_dist <= 0.) || (cs_ori >1.) || (cs_ori < -1.))
    return 0.;

#if SPEED_UP
  if ( (delay == td) && (pd == perp_dist) && (cs_ori == ori))
    return result;
  else{
    td = delay;
    pd = perp_dist;
    ori = cs_ori;
  }
#endif


  lam = pp.td_lam;
  dist = td_dist(perp_dist,cs_ori);
  rdist = dist/lam;

  tmp= (rdist-1.)/delay - (pt_s.td_tau_scale);
  result = tmp*rdmc_pt_td(delay, perp_dist, cs_ori);

  return result;
}

double rdmc_pt_lgtd_norm(double perp_dist, double cs_ori){
  static double dist,rdist,ldist,lam;
#if SPEED_UP
  static double pd=-RDMC_BIG,ori=-RDMC_BIG;
#endif
  static double result=1.;

  if ((perp_dist <= 0.) || (cs_ori >1.) || (cs_ori < -1.))
    return 0.;

  if (do_init)
    td_init_special();

#if SPEED_UP
  if ( (pd== perp_dist) &&  (ori == cs_ori) ){
    return result;
  } else{
    pd =perp_dist;
    ori = cs_ori;
  }
#endif

  lam = pp.td_lam;
  dist = td_dist(perp_dist,cs_ori);
  rdist = dist/lam;
  ldist = dist/pp.td_att;

  result = -ldist -rdist*pt_s.lg_td_tau_slew;
  return result;
}

double rdmc_pt_lgtd_patched(double delay, double perp_dist, double cs_ori){
  double result=0.0;

  if (do_init)
    td_init_special();

  if ((perp_dist <= 0.) || (cs_ori >1.) || (cs_ori < -1.))
    return -RDMC_FINFTY;

  if (delay >= pt_s.td_t0){ /* pandel */
    result = rdmc_pt_lgtd(delay, perp_dist, cs_ori);
  }else{
    if (perp_dist > DIST_MAGIC){
      result = rdmc_pt_lgtd(delay, perp_dist, cs_ori);
    }else if (delay <= 0){ /* Gauss */
      double ipt=rdmc_pt_td_int(pt_s.td_t0, perp_dist, cs_ori);
      double pt=rdmc_pt_td(pt_s.td_t0, perp_dist, cs_ori);
      double dpt=rdmc_pt_td_diff(pt_s.td_t0, perp_dist, cs_ori);
      double a0=ipt*pt_s.td_t0_dsq1 - 0.5*pt+dpt*pt_s.td_t0/12.;
      double norm=log(a0);
      result = norm - delay*delay*pt_s.td_sigma_d_2square;
    } else{ /* P3 */
      double ipt=rdmc_pt_td_int(pt_s.td_t0, perp_dist, cs_ori);
      double pt=rdmc_pt_td(pt_s.td_t0, perp_dist, cs_ori);
      double dpt=rdmc_pt_td_diff(pt_s.td_t0, perp_dist, cs_ori);
      double a0=ipt*pt_s.td_t0_dsq1 - 0.5*pt+dpt*pt_s.td_t0/12.;
      double a2 = -3.*ipt*pt_s.td_t0_dsq3 - 1.25*dpt*pt_s.td_t0_dsq1 + 4.5*pt*pt_s.td_t0_dsq2;
      double a3=2*ipt*pt_s.td_t0_dsq4 + 7./6.*dpt*pt_s.td_t0_dsq2 - 3 * pt*pt_s.td_t0_dsq3;
      result = log(a0 + a2*delay*delay + a3*delay*delay*delay);
    }
  }
  return result;
}

double rdmc_pp_lgtd_norm(double perp_dist, double cs_ori,double cs_axis){
  static double dist,rdist,ldist,lam;
#if SPEED_UP
  static double pd=-RDMC_BIG,ori=-RDMC_BIG,axis=-RDMC_BIG;
#endif
  static double result=1.;

  if ((perp_dist <= 0.) || (cs_ori >1.) || (cs_ori < -1.) || (cs_axis >1.) || (cs_axis < -1.))
    return 0.;

  if (do_init)
    td_init_special();

#if SPEED_UP
  if ( (pd== perp_dist) &&  (ori == cs_ori)&& (cs_axis == axis) ){
    return result;
  } else{
    pd =perp_dist;
    ori = cs_ori;
    axis = cs_axis;
  }
#endif

  lam = pp.ps_lam;
  dist = ps_dist(perp_dist,cs_ori,cs_axis);
  rdist = dist/lam;
  ldist = dist/pp.td_att;

  result = -ldist -rdist*pp_s.lg_td_tau_slew;
  return result;
}

double rdmc_lgamma(double xx){
  return lgamma(xx);
}

double rdmc_gamac(double a, double x){ /* incomplete gamma function */
  double gamser,gammcf,gln;

  if (x < 0.0 || a <= 0.0)
    return 0.;

  if (x < (a+1.0)) {
    gser(&gamser,a,x,&gln);
    return gamser;
  } else {
    gcf(&gammcf,a,x,&gln);
    return 1.0-gammcf;
  }
}

#define ITMAX 100
#define EPS 3.0e-7

void gcf(double *gammcf,double a,double x,double *gln){
  int n;
  double gold=0.0,g,fac=1.0,b1=1.0;
  double b0=0.0,anf,ana,an,a1,a0=1.0;
  *gln=rdmc_lgamma(a);
  a1=x;
  for (n=1;n<=ITMAX;n++) {
    an=(double) n;
    ana=an-a;
    a0=(a1+a0*ana)*fac;
    b0=(b1+b0*ana)*fac;
    anf=an*fac;
    a1=x*a0+anf*a1;
    b1=x*b0+anf*b1;
    if (a1) {
      fac=1.0/a1;
      g=b1*fac;
      if (fabs((g-gold)/g) < EPS) {
        *gammcf=exp(-x+a*log(x)-(*gln))*g;
        return;
      }
      gold=g;
    }
  }
}

static void gser(double *gamser,double a,double x,double *gln)
{
  int n;
  double sum,del,ap;

  *gln=rdmc_lgamma(a);
  if (x <= 0.0) {
    *gamser=0.0;
    return;
  } else {
    ap=a;
    del=sum=1.0/a;
    for (n=1;n<=ITMAX;n++) {
      ap += 1.0;
      del *= x/ap;
      sum += del;
      if (fabs(del) < fabs(sum)*EPS) {
        *gamser=sum*exp(-x+a*log(x)-(*gln));
        return;
      }
    }
    return;
  }
}

