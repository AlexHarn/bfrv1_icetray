
#ifndef IPDF_PANDEL_ConvolutedHyperg_H
#define IPDF_PANDEL_ConvolutedHyperg_H

/*
 *  Compute the hypergeometric portion of the Gaussian convoluted
 *  Pandel PDF
 *
 *  1F1(0.5*xi,0.5,0.5*eta*eta)/gamma(0.5*(xi+1)) -
 *              sqrt2*eta*1F1(0.5*(xi+1),1.5,0.5*eta*eta)/gamma(0.5*xi)
 */

/*
 *  Compute using GSL by directly taking the difference of 1F1 terms.
 *
 *  Valid over -30 < eta < 1.5; xi < 5
 */
double gslConvoluted1F1Diff(const double xi, const double eta);

/*
 *  Compute using GSL using Tricomi's function U(a, b, z)
 *
 *  Valid over 0 < eta < 50; xi < 5
 */
double gslConvolutedU(const double xi, const double eta);


/*
 *  Fast implementation using the hypergeometric power
 *  series with the denominator precomputed.  This is >10x faster than GSL.
 *  Error is <5e-15 for eta < 0, xi > 0.1.  Error rapidly increases for
 *  eta > 0 due to poor conditioning of the 1F1 representation of U(a,b,z).
 *  This is also true for any GSL implementation.
 *
 *  Valid over -14.2 < eta < 1.5; 0.1 < xi < 5
 */
double fastConvolutedHyperg(const double xi, const double eta);

#endif // IPDF_PANDEL_ConvolutedHyperg_H
