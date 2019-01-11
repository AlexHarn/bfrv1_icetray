#ifndef ZenithSampler_H
#define ZenithSampler_H
/**
 *  copyright  (C) 2005
 *  the IceCube collaboration
 *  $Id: $
 *
 *  @version $Revision: $
 *  @date    $Date: $
 *  @author  Kotoyo Hoshina <hoshina@icecube.wisc.edu>
 *
 * @brief emulating flat sampling in zenith
 */
#include <vector>
#include <map>
#include <iostream>
#include <icetray/I3Logging.h>
#include <icetray/I3PointerTypedefs.h>
//#include <neutrino-generator/utils/Polynominal1DFunc.h>

namespace ZenithSampler {

/**
 * SimpleSlopeSampler
 *
 * change distribution from uniform (left) to weighted (right)
 *
 *  2/L +       +          +       +
 *      |       |          |       | (2-al)/L
 *      |       |          |      /|
 *      |       |          |     / |
 *      |       |          |    /  |
 *  1/L |-------|          |   /   |
 *      |       |          |  /    |
 *      |       |          | /     |
 *      |       |     al/L |/      |
 *      |       |          |       |
 * -----+-------+----   ---+-------+----> x
 *     min     max        min     max
 *      |_______|          |_______|
 *          L                  L
 *
 * al ... alpha (0.1 < al < 1.9)
 * min ... 
 * max ... 
 * if alpha is 1.0, the zenith distribution 
 * remains unchanged (= uniform).
 *
 * ---------
 * equation:
 * ---------
 * original distribution is 
 * y = 1/L (constant).
 *
 * To modify this flat function to 1dim function,
 * set origin at mincosth then
 *
 * diff_y = ((2-al)/L - al/L);
 * diff_x = L;
 * y = ((2-al)/L - al/L) / L * x + al/L
 *   = (2 - 2*al) / L^2 * x + al / L
 *
 * ---------
 * integral:
 * ---------
 *
 * integral of y from min to max is (by definition) normalized.
 * yinteg = [(1-al)/L^2 * x^2 + al/L * x] min to max = 1.0
 *
 * with a random number r (0 < r < 1) get x from 
 * following equation
 *
 * (1-al) /L^2 * x^2 + al/L * x = r
 * multiply L
 * (1-al) /L * x^2 + al * x - r*L = 0
 *
 * x = (-al +- sqrt(al^2 + 4*(1-al)/L*r*L)) / (2*(1-al)/L)
 *   = L / (2-2al) * (-al +- sqrt(al^2 + 4*(1-al)*r)
 *
 * -------------
 * weight at x :
 * -------------
 * the nominal weight is flat (y = 1/L).
 * then weight will be 
 * weight = (1/L) / y = 1 / (y*L)
 *
 * @return vector<double> result
 * result[0] is x_value, result[1] is weight
 */
  std::vector<double> SimpleSlopeSampler(double alpha,
                          double min, double max, 
                          double random);

/**
 class for polynominal 1D function
 y = ax + b
 */
  class Polynominal1DFunc 
  {
    //private:
    public:
      double a_; // inclination
      double b_; // intersection
      double xmin_;
      double xmax_;
      double intgmin_;
      double intgmax_;
      double totalintg_;
      int    funcid_;
      SET_LOGGER("ZenithSampler");

    public:
      Polynominal1DFunc(double a=0, double b=0, int funcid = -1) :
         a_(a), b_(b), xmin_(0), xmax_(0),
         intgmin_(0), intgmax_(0), totalintg_(0),
         funcid_(funcid) {}

      inline double Evaluate(double x) { return a_ * x + b_; }

      inline double Integral(double x) { return 0.5 * a_ * x*x + b_ * x; }

      /**
       sample x for constant function (y = b_)
       because rand is sampled in integral of the function
       need to find answer x for
       rand = b_ * x
       */ 
      double Sample_x_pol1(double rand) ;

      /**
       sample x for pol1 function (y = a_*x + b_)
       because rand is sampled in integral of the function
       need to find answer x for
       rand = 0.5*a_*x**2 + b_*x
       */ 
      double Sample_x_pol2(double rand) ;


      double Sample_x(double rand) {
         if (a_ == 0) return Sample_x_pol1(rand);
         return Sample_x_pol2(rand);
      }

      void SetXmin(double xmin) {
         xmin_ = xmin;
         intgmin_ = Integral(xmin_);
         log_trace("funcid %d, xmin %f, intgmin %f" ,funcid_, xmin_, intgmin_);
      }

      void SetXmax(double xmax) {
         xmax_ = xmax;
         intgmax_ = Integral(xmax_);
         log_trace("funcid %d, xmax %f, intgmax %f" , funcid_, xmax_, intgmax_);
      }

      double GetTotalIntegral() {
         totalintg_ = intgmax_ - intgmin_;
         log_trace("funcid %d, intgmin %f, intgmax %f, total %f", funcid_, intgmin_, intgmax_, totalintg_);
         return totalintg_;
      }

      double GetMinIntegral() { return intgmin_; }
      int    GetFuncID() { return funcid_; }

      void DebugPrint() {
         log_debug("funcid %d, lowerx = %f, lowerintg =%f, upperx = %f, upperintg = %f, totintg %f", funcid_, xmin_, intgmin_, xmax_, intgmax_,totalintg_);
      }
  };

/**
 * FlatZenithEmulator
 *
 * This function emulates flat zenith sampling
 * in cos(zen) space.
 * It divides zenith range (-1 to 1) in several sections, 
 * and applies same idea with SimpleSlopeSampler 
 * with different slopes in each section. 
 * The resulting sampling distribution is very close to 
 * sampling flat in zenith, but still possible to weight 
 * back to flat in cos(zenith), which is what we observe 
 * with diffuse flux.
 */
  class FlatZenithEmulator
  {
    public:
      FlatZenithEmulator()
      :mincos_(-1), maxcos_(1), nfuncs_(0) 
      {
         xths_.clear();
         yvals_.clear();
         funcmap_.clear();
      }

      void Initialize(double mincos, double maxcos);  
      std::vector<double> Sampling(double r);

    private:
      double mincos_;
      double maxcos_;
      unsigned int nfuncs_;
      std::vector<double> xths_;  // x_thresholds
      std::vector<double> yvals_; // y at xth
      std::map<double, Polynominal1DFunc> funcmap_;

      SET_LOGGER("ZenithSampler");

  };

  I3_POINTER_TYPEDEFS(FlatZenithEmulator);
}

#endif 
