
#include "ipdf/Pandel/ConvolutedHyperg.h"
#include "ipdf/Utilities/PdfMath.h"

#include <math.h>
#include <gsl/gsl_sf_hyperg.h>

double gslConvoluted1F1Diff(const double xi, const double eta) {

  const double xi1 = 0.5*(xi+1);
  const double xi2 = 0.5*xi;
  const double eta2 = 0.5*eta*eta;

  const double f1 = PdfMath::HyperGeometric1F1(xi2,0.5,eta2);
  const double f2 = PdfMath::HyperGeometric1F1(xi1,1.5,eta2);
  return f1/PdfMath::Gamma(xi1) - eta*M_SQRT2*f2/PdfMath::Gamma(xi2);
}

// We don't need more than 200 terms to converge over our specified domain
#define N_PRECOMP_COEFF 200

namespace {

class PrecomputedBCoefficients {

  public:

    PrecomputedBCoefficients() {

      b05_[0] = b15_[0] = 1.L;
      for (int i = 1; i < N_PRECOMP_COEFF; ++i) {
        b05_[i] = 1.L / (i * (0.5L + (i - 1)));
        b15_[i] = 1.L / (i * (1.5L + (i - 1)));
      }
    }

    double b05_[200];
    double b15_[200];

};

const PrecomputedBCoefficients cf;

double SQRTPI_INV = 1./sqrt(M_PI);

}


double gslConvolutedU(const double xi, const double eta) {

  return SQRTPI_INV*gsl_sf_hyperg_U(0.5*xi, 0.5, 0.5*eta*eta);
}

/*
 *  1F1(a,b,z) = sum_n=0^infty (a)n * z^n / ( (b)n * n! )
 *  Compute the power series using a source of precomputed
 *  denominators (b)n * n!
 */
double fastHyperg1F1(const double a, const double z, const double* b) {

  double val = 1.;
  double term = 1.;
  double an = a;
  const double* end = b + N_PRECOMP_COEFF;
  // We're starting at n = 1 term, so advance b pointer
  for (++b; b < end; ++b) {
    term *= an * z * (*b);
    an += 1.;
    val += term;
    if (fabs(term)*1e17 < val) {
      return val;
    }
  }

  // Out of range.  The series would have converged by now over the
  // specified range.
  return val;
}

/*
 *  Compute the hypergeometric portion of the Gaussian convoluted
 *  Pandel PDF
 *
 *  1F1(0.5*xi,0.5,0.5*eta*eta)/gamma(0.5*(xi+1)) -
 *              sqrt2*eta*1F1(0.5*(xi+1),1.5,0.5*eta*eta)/gamma(0.5*xi)
 *  for 0.5*eta*eta < 100 and xi < 5 using the hypergeometric power series
 *  with the denominator precomputed.  This is >10x faster than GSL.
 */
double fastConvolutedHyperg(const double xi, const double eta) {

  double xi1 = 0.5*(xi+1);
  double xi2 = 0.5*xi;
  const double eta2 = eta*eta*0.5;

  const double f1 = fastHyperg1F1(xi2,eta2,cf.b05_);
  const double f2 = fastHyperg1F1(xi1,eta2,cf.b15_);
  return f1*exp(-lgamma(xi1)) - eta*M_SQRT2*f2*exp(-lgamma(xi2));
}
