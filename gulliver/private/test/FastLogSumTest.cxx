/**
    copyright  (C) 2007
    the icecube collaboration
    $Id$

    @version $Revision$
    @date $Date$
    @author boersma
*/

// std includes
#include <cmath>
#include <float.h>
#include <vector>
#include <iostream>

// unit test main
#include <I3Test.h>

// the class to be tested
#include "gulliver/utilities/FastLogSum.h"

// Several tests are run with x_i=exp(i)
// to use the full range but not let the x_i exceed the range of
// normalized double precision floats, we get the limits from
// "float.h". Nmax=708, in practice.
namespace FLSTestConstants {
    const int LogDblMax( int(floor(log(DBL_MAX)) ) );
    const int LogDblMin( int(-floor(-log(DBL_MIN)) ) );
    const int Nmax = (LogDblMax>-LogDblMin) ? -LogDblMin : LogDblMax;
    const unsigned int Nlongtest = 1000000;
};

using namespace FLSTestConstants;

TEST_GROUP(FastLogSum)

// make a sum of logs which equals zero by construction
// x_i = exp(i) with i running from -N to +N
// testing the sum both with Add(x_i) and FastAdd(x_i)
TEST(SumEqualsZero)
{

  std::cout << std::endl;
  FastLogSum fls;
  FastLogSum ffls;
  double checksum = 0;
  ENSURE_DISTANCE(fls.GetLogSum(), checksum, 1e-5,
          "With zero terms, the 'sum' of logs should be 0." );

  // diagnostic noise (enable with -s option to phys-services-test)
  std::cout << std::endl;

  for ( int i=-Nmax; i<=Nmax; ++i ){
      double x_i = exp(1.0*i);
      fls.Add(x_i);
      ffls.Add(x_i);
      checksum += log(x_i);
      std::cout << "checksum=" << checksum << std::endl;
      std::cout << "fls.GetLogSum()=" << fls.GetLogSum() << std::endl;
      ENSURE_DISTANCE(fls.GetLogSum(), checksum, 1e-5,
              "sum of logs should be equal to checksum." );
      ENSURE_DISTANCE(ffls.GetLogSum(), checksum, 1e-5,
              "fast sum of logs should be equal to checksum." );
  }

  std::cout << std::endl;

  ENSURE_DISTANCE(checksum, 0, 1e-5,
          "Product of x_i is 1, so the sum of logs should be 0." );
  ENSURE_DISTANCE(fls.GetLogSum(), 0, 1e-5,
          "Product of x_i is 1, so the sum of logs should be 0." );
  ENSURE_DISTANCE(ffls.GetLogSum(), 0, 1e-5,
          "Product of x_i is 1, so the fast sum of logs should be 0." );
}

// Make a sum with many millions of terms, quasirandomly jumping through
// all accessible orders of magnitude.
// * Avoiding zero, denormalized numbers NAN and INF.
// * Testing the sum both with Add(x_i) and FastAdd(x_i).
// * Testing the sum both with Add(x_i) and FastAdd(x_i).
// * Compare result with regular (non-analytic) log sum.
TEST(FreakingBigLongSum)
{
  FastLogSum fls;
  FastLogSum ffls;
  double checksum = 0.0;
  double myrand = 1.0;
  ieee754_double &randbits = reinterpret_cast<ieee754_double&>( myrand );
  for ( unsigned int itest = 0; itest < Nlongtest; ++itest ){
    randbits.ieee.exponent = 1 + (itest*79) % 2045;
    randbits.ieee.mantissa0 = itest*13;
    checksum += log( myrand );
    fls.Add( myrand );
    ffls.FastAdd( myrand );
  }
  ENSURE_DISTANCE(fls.GetLogSum(), checksum, 1e-5,
          "Fast log sum should be equal to slow log sum." );
  ENSURE_DISTANCE(ffls.GetLogSum(), checksum, 1e-5,
          "Fast fast log sum should be equal to slow log sum." );
}

// test sums for which prod_i x_i would be way above or below
// the regular floating point range
// * Testing the sum both with Add(x_i) and FastAdd(x_i).
// * Compare with analytically calculated log sum.
// * Also test that Reset() works correctly.
TEST(SumIsVeryBigOrVerySmall)
{
  FastLogSum fls_big;
  FastLogSum ffls_big;
  FastLogSum fls_small;
  FastLogSum ffls_small;
  double x_i = 1;
  for ( int i=1; i<=Nmax; ++i ){
      x_i *= M_E;
      fls_big.Add(x_i);
      ffls_big.FastAdd(x_i);
      fls_small.Add(1/x_i);
      ffls_small.FastAdd(1/x_i);
  }
  ENSURE_DISTANCE(fls_big.GetLogSum(), 0.5*Nmax*(Nmax+1), 1e-2,
          "Adding very large logs didn't sum to the correct value." );
  ENSURE_DISTANCE(ffls_big.GetLogSum(), 0.5*Nmax*(Nmax+1), 1e-2,
          "Adding very large logs didn't sum to the correct value." );
  ENSURE_DISTANCE(fls_small.GetLogSum(), -0.5*Nmax*(Nmax+1), 1e-2,
          "Adding very small logs didn't sum to the correct value." );
  ENSURE_DISTANCE(ffls_small.GetLogSum(), -0.5*Nmax*(Nmax+1), 1e-2,
          "Adding very small logs didn't sum to the correct value." );

  fls_big.Reset();
  ENSURE_DISTANCE(fls_big.GetLogSum(), 0, 1e-5,
          "After a Reset(), the sum of logs should be 0." );
  ffls_big.Reset();
  ENSURE_DISTANCE(ffls_big.GetLogSum(), 0, 1e-5,
          "After a Reset(), the sum of logs should be 0." );
  fls_small.Reset();
  ENSURE_DISTANCE(fls_small.GetLogSum(), 0, 1e-5,
          "After a Reset(), the sum of logs should be 0." );
  ffls_small.Reset();
  ENSURE_DISTANCE(ffls_small.GetLogSum(), 0, 1e-5,
          "After a Reset(), the sum of logs should be 0." );
}

// Testing that corrupt input does not result in a regular answer
// if any of the x_i is zero, infinity or NAN, then the result is
// undefined and GetLogSum() should return NAN. We check that this
// is the case, and also that the NAN-ness is not inadvertently undone
// when adding e.g. multiple negative numbers. (When the sum of the logs
// would be computed as the log of the product then including two
// negative x_i factors would result in a positive product and a regular
// log value; that would be bad: if the input is bogus then the output
// MUST be bogus, too.).
// * Testing the sum only with Add(x_i), with FastAdd(x_i) this would fail.
// * Testing that Reset() still works.
TEST(ParryCorruptInput)
{
  FastLogSum unbribable;
  FastLogSum untouchable;
  FastLogSum inflammable;
  FastLogSum impregnable;

  // first strike: hide one corrupt number in a long sum
  double x_i = 1;
  for ( int i=0; i<=Nmax; ++i ){
      x_i *= 0.047*(1+(i%23));
      if ( i==13 ){
          unbribable.Add(-x_i); // negative
          untouchable.Add(HUGE_VAL*HUGE_VAL); // infitinity
          inflammable.Add(-HUGE_VAL*HUGE_VAL); // negative infitinity
          impregnable.Add(NAN); // NAN
      } else {
          unbribable.Add(x_i);
          untouchable.Add(x_i);
          inflammable.Add(x_i);
          impregnable.Add(x_i);
      }
  }
  ENSURE( std::isnan(unbribable.GetLogSum()),
          "When one or more of the x_i is negative, the logsum should be NAN." );
  ENSURE( std::isnan(untouchable.GetLogSum()),
          "When one or more of the x_i is INF, the logsum should be NAN." );
  ENSURE( std::isnan(inflammable.GetLogSum()),
          "When one or more of the x_i is -INF, the logsum should be NAN." );
  ENSURE( std::isnan(impregnable.GetLogSum()),
          "When one or more of the x_i is NAN, the logsum should be NAN." );

  // second strike: add more corrupt numbers to the long sums, one by one
  std::vector<double> garbage;
  garbage.push_back(-10);
  garbage.push_back(-100);
  garbage.push_back(0);
  garbage.push_back(HUGE_VAL*HUGE_VAL);
  garbage.push_back(NAN);
  for ( std::vector<double>::iterator ipoison = garbage.begin();
                                      ipoison != garbage.end();
                                    ++ipoison ){
      unbribable.Add(*ipoison);
      ENSURE( std::isnan(unbribable.GetLogSum()),
              "When one or more of the x_i is negative/NAN/INF, the logsum should be NAN." );
      untouchable.Add(*ipoison);
      ENSURE( std::isnan(untouchable.GetLogSum()),
              "When one or more of the x_i is negative/NAN/INF, the logsum should be NAN." );
      inflammable.Add(*ipoison);
      ENSURE( std::isnan(inflammable.GetLogSum()),
              "When one or more of the x_i is negative/NAN/INF, the logsum should be NAN." );
      impregnable.Add(*ipoison);
      ENSURE( std::isnan(impregnable.GetLogSum()),
              "When one or more of the x_i is negative/NAN/INF, the logsum should be NAN." );
  }

  unbribable.Reset();
  ENSURE_DISTANCE(unbribable.GetLogSum(), 0, 1e-5,
          "After a Reset(), the sum of logs should be 0." );
  untouchable.Reset();
  ENSURE_DISTANCE(untouchable.GetLogSum(), 0, 1e-5,
          "After a Reset(), the sum of logs should be 0." );
  impregnable.Reset();
  ENSURE_DISTANCE(impregnable.GetLogSum(), 0, 1e-5,
          "After a Reset(), the sum of logs should be 0." );

}

TEST(TmpTestUnionVsCast)
{
    const int imax = 1000*1000;

    // method 1
    double logsum1=0.;

    // method 2
    double prod=1.;
    ieee754_double& ieee_bits = reinterpret_cast<ieee754_double&>(prod);
    int powersum2 = 0;

    // method 3
    ieee754_double ieee_prod;
    ieee_prod.d=1.;
    int powersum3 = 0;

    for (int i=0; i<imax; i++){
        double x = (1.+i)/(imax-i);
        // method 1
        logsum1 += log(x);
        // method 2
        prod *= x;
        // on some platforms (gcc 4.4) this will give warnings and/or wrong results
        powersum2 += int(ieee_bits.ieee.exponent) - int(IEEE754_DOUBLE_BIAS);
        ieee_bits.ieee.exponent = IEEE754_DOUBLE_BIAS;
        // method 3
        // this should always work and be as fast as method 2
        ieee_prod.d *= x;
        powersum3 += int(ieee_prod.ieee.exponent) - int(IEEE754_DOUBLE_BIAS);
        ieee_prod.ieee.exponent = IEEE754_DOUBLE_BIAS;
    }
    double logsum2 = powersum2*M_LN2+log(ieee_bits.d);
    double logsum3 = powersum3*M_LN2+log(ieee_prod.d);

    ENSURE_DISTANCE(logsum1,0.,1e-8,"sum of logs should be zero")
    ENSURE_DISTANCE(logsum2,0.,1e-10,"sum of logs should be zero")
    ENSURE_DISTANCE(logsum3,0.,1e-10,"sum of logs should be zero")
}
