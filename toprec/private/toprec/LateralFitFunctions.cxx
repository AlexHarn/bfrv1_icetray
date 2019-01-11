/**
 * Copyright (C) 2007
 * The IceCube collaboration
 * ID: $Id$
 *
 * @file LateralFitFunctions.cxx
 * @version $Rev$
 * @date $Date$
 * @author $Author$
 */

#include <vector>
#include <gsl/gsl_sf_gamma.h>
#include "toprec/LateralFitFunctions.h"

using namespace std;

namespace LateralFitFunctions {

/********************************************************************/

double top_ldf_nkg(double r, double *par){
  // the logarithm of the nkg function, so we can fit logvem
  // moliere radius calculated for 700 hPa with formula in
  // arXiv:hep-ph/0407020 v2 Jul 2004
  return par[2] + (par[3]-2.)*log10(r/R0_PARAM) + (par[3]-4.5)*log10((r+128.)/(R0_PARAM+128.));
}

/********************************************************************/

double top_ldf_dlp(double r, double *par) {
  //if (r/I3Units::m < 30) r = 30*I3Units::m;
  /*if (r/I3Units::m < 30) {
    double local_x = log10(r/R0_PARAM);
    double local_xR = log10(30*I3Units::m/R0_PARAM);
    return par[2] + KAPPA*local_xR*local_xR - (par[3] + 2*KAPPA*local_xR)*local_x;
    } else {*/
    double local_x = log10(r/R0_PARAM);
    return par[2] - par[3]*local_x - KAPPA*local_x*local_x;
    //}
}

/********************************************************************/

double top_ldf_sigma(double r, double logq) {
  // parametrisation of sigma (from study of tank-to-tank fluctuations)

  // Function: 3 powerlaws with a kink at logq = kink
  // sigma = A*(q/beta)^alpha = A/c * exp(alpha*logq)
  // with c = beta^alpha

  // left radius r as a parameter in order to be able to implement an
  // explicit radius dependency

//  double A[3] = {0.3538, 0.389, 0.071};
//  double alpha[3] = {0.044, -0.346, 0.191};
//  double kink[2] = {0.105, 1.3776};
//
//  int i = 0;
//  if (logq>kink[0]) {
//    i = 1;
//    if (logq>kink[1])
//      i = 2;
//  }
//
//  return A[i] * pow(10, alpha[i] * logq);

  double a[2]     = {-.5519, -.078};
  double b[3]     = {-.373, -.658, .158};
  double trans[2] = {.340, 2.077};
  
  if(logq>trans[1]) logq=trans[1]; 
  if (logq<trans[0]) {
    return pow(10, a[0] + a[1] * logq);
  }
  else return pow(10, b[0] + b[1] * logq + b[2]*logq*logq);

}

/*----------------- CURVATURE FUNCTIONS ---------------------------*/
// The "gausspar" family of curvature functions has been moved to recclasses.
// Use I3LaputopParams::ExpectedShowerFrontDelay() and
//     I3LaputopParams::ExpectedShowerFrontDelayError()
//        ...to access them from toprec.

// The experimental "kislat" function remains here:

/********************************************************************/
double top_curv_kislat(double r, double *par){
  // this must be a function that is never NAN!
  return 20. * (exp(-r*r/(170.*170.)) - 1.) - 4.9e-4*r*r; // from FK
}

/********************************************************************/

double top_curv_kislat_llh(double r, double deltaT, double *par){
  // for details on the likelihood function, see Fabian Kislat's Diploma Thesis

  if ((r != r) || (deltaT != deltaT)) return NAN;        // check for NANs, because gsl_sf_gamma doesn't like them

  // the parametrisations of the likelihood:
  const double sigma1_par[3] = {0.516, 2.36e-2, 1.73e-4};
  const double sigma2_par[3] = {6.8, 0.003, 1.00e-3};
  const double Crel_par = 1./86.0;

  double local_curv = top_curv_kislat(r, par);
  double local_delta_t = deltaT - local_curv;
  double local_sigma1 = sigma1_par[0] + sigma1_par[1]*r + sigma1_par[2]*r*r;
  double local_sigma2 = sigma2_par[0] + sigma2_par[1]*r + sigma2_par[2]*r*r;

  double local_norm = log(local_sigma1 * (sqrt(2*M_PI) + Crel_par * 
					   (pow(2., 0.5*local_sigma1/local_sigma2) +
					    gsl_sf_gamma(0.5*local_sigma1/local_sigma2)
					    )));

  return local_norm + 0.5*exp(-local_delta_t/local_sigma1) - 
         log(exp(-0.5*local_delta_t/local_sigma1) + 
	     Crel_par*exp(-0.5*local_delta_t/local_sigma2));
}

/********************************************************************/

double estimate_energy(double lsref, double szenith, double conversion_radius){
  double fg_energy = estimate_firstguess_energy(lsref, szenith, conversion_radius);
  // here the full wisdom of simulation can be deposited...
#if 0
  /* old 2nd order correction by Stefan Klepser */
  double log10_corrected_energy = log10(fg_energy) - (0.69805 - 0.298603 *szenith
                                                   - 0.535927 *szenith*szenith
                                                   + 0.395615 *pow(szenith, 3)
                                                   - 0.238035 *pow(szenith, 4)
                                                   + 0.102972 *pow(szenith, 5));
#endif // 0
  /* new 2nd order correction by Fabian Kislat, after corrections to
     the simulation code have been made. */
  double log10_corrected_energy = log10(fg_energy) - (  0.9797 - 1.635 * szenith
						      + 0.5483 * szenith*szenith );

  return pow(10, log10_corrected_energy); 
}

/********************************************************************/

double estimate_firstguess_energy(double lsref, double szenith, double conversion_radius){ 
  vector<double> e_par = E_parameters(conversion_radius);

  // first estimation as described in Internal Report 200702001
  double log10_E_first = e_par[0]+e_par[1]*szenith-sqrt(e_par[2]+e_par[3]*szenith-e_par[4]*lsref);
  // second order correction, described in Stefans thesis when finished 
  return pow(10, log10_E_first);
} 

/********************************************************************/

// I know, this may look like a completely magical thing, but if I'd
// predefine all the constants in some header file, it would be chaos
// instead, so what to do?
// All numbers are extracted from a corsika study that is partly described
// in the internal report repository (see Jan 07).

vector<double> E_parameters(double ref_radius){

  double rRef = log10(ref_radius/I3Units::m);
  
  vector<double> lr;
  
  vector<vector<double> > p;
  vector<double> p0;
  vector<double> p1;
  vector<double> p2;
  vector<double> p3;
  vector<double> p4;
  
  vector<double> out;
  
  lr.push_back(50.);
  p0.push_back(20.2778);
  p1.push_back(10.4785);
  p2.push_back(376.253);
  p3.push_back(629.015);
  p4.push_back(58.955);
  
  lr.push_back(80.);
  p0.push_back(22.713);
  p1.push_back(11.0473);
  p2.push_back(428.443);
  p3.push_back(742.533);
  p4.push_back(66.0993);
  
  lr.push_back(100.);
  p0.push_back(24.0269);
  p1.push_back(11.3414);
  p2.push_back(459.884);
  p3.push_back(805.717);
  p4.push_back(69.9528);
  
  lr.push_back(200.);
  p0.push_back(28.8654);
  p1.push_back(12.3592);
  p2.push_back(596.026);
  p3.push_back(1049.47);
  p4.push_back(84.1653);
  
  lr.push_back(400.);
  p0.push_back(35.0482);
  p1.push_back(13.5397);
  p2.push_back(817.707);
  p3.push_back(1384.59);
  p4.push_back(102.434);
  
  lr.push_back(700.);
  p0.push_back(41.1952);
  p1.push_back(14.6112);
  p2.push_back(1092.41);
  p3.push_back(1741.93);
  p4.push_back(120.769);
  
  lr.push_back(1000.);
  p0.push_back(45.7182);
  p1.push_back(15.348);
  p2.push_back(1329.71);
  p3.push_back(2019.19);
  p4.push_back(134.389);
  
  p.push_back(p0);
  p.push_back(p1);
  p.push_back(p2);
  p.push_back(p3);
  p.push_back(p4);
  
  int regime = 1;
  
  for(unsigned int i=1; i<lr.size()-1; i++) {
    if(rRef>log10(lr[i])) regime++;
  }
  
  for(int i=0; i<5; i++){
    double slope = log10(p[i][regime]/p[i][regime-1])/(log10(lr[regime]/lr[regime-1]));
    double constant = log10(p[i][regime]) - slope*log10(lr[regime]);
    out.push_back(pow(10, slope*rRef + constant));
  }
  
  return out;
}

} // end namespace
  

