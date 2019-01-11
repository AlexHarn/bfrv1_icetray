#ifndef PdfMath_H
#define PdfMath_H

/**
    copyright  (C) 2004
    the icecube collaboration
    $Id$

    @file PdfMath.h
    @version $Revision: 1.3 $
    @date $Date$
    @author S Robbins <robbins@physik.uni-wuppertal.de>
 */

//#include "config.h"

#include <cmath> // for finite() lgamma()
#include <float.h> // for FLT_MAX

#include <gsl/gsl_math.h> // ensures BSD math constants, e.g. M_PI 
#include <gsl/gsl_sf_gamma.h>
#include <gsl/gsl_sf_hyperg.h>

/// @brief Useful mathematical functions for the IPDF project.  

namespace PdfMath {
  extern const double FINFTY /* = FLT_MAX */;
  inline double Gamma(double t) {
    return gsl_sf_gamma(t);
  }

  inline double LogGamma(double t) {
    return  gsl_sf_lngamma (t);
    // standar mathlib could be faster    return lgamma(t);
  }

  inline double HyperGeometric1F1(double a, double b, double x) {
    return gsl_sf_hyperg_1F1(a,b,x);
  }

  inline double IncGammaQ(double a, double t) {
    return  gsl_sf_gamma_inc_Q(a,t);
  }

  inline double IncGammaP(double a, double t) {
    return  gsl_sf_gamma_inc_P(a,t);
  }

  inline double IncGamma(double a, double t) {
    return  gsl_sf_gamma_inc(a,t);
  }

  /**************************************************************************/
  /*       nint(double a)                                                  */
  /* converts a to the nearest integer                                     */
  /* but also takes care on the sign                                       */
  /**************************************************************************/
  inline long Nint(double a){
   return  (a >= 0.) ?  (long) (a + 0.5 ) : (long) (a - 0.5);
  }

  /**
   * @brief Computes a positive integer power faster than pow(double,double)
   */
  inline double FastPow(double f,unsigned int ui){
    double result = 1.;
    while ( ui ){
      if ( ui & 1 ){
        result *= f;
      }
      f*=f;
      ui/=2;
    }
    return result;
  }
}

#endif // PdfMath_H
