/**
 *  copyright  (C) 2005
 *  the IceCube collaboration
 *  $Id: $
 *
 *  @version $Revision: $
 *  @date    $Date: $
 *  @author  Kotoyo Hoshina <hoshina@icecube.wisc.edu>
 *
 *  @brief   Calculator IMPLEMENTATION FILE
 */

#include <neutrino-generator/utils/ZenithSampler.h>
#include <algorithm>
#include <iostream>
#include <cmath>


namespace ZenithSampler {

std::vector<double> SimpleSlopeSampler(double alpha,
                           double min,
                           double max,
                           double random)
{
  double xval = -2;
  double weight = 1.0;
  std::vector<double> result;
  result.push_back(xval);
  result.push_back(weight);

  // check zenith
  double l = max - min;
  if (l == 0) {
     // flat distribution 
     result[0] = max;
     result[1] = 1.0; // no weight
     return result;
  } else if (l < 0 ) {
     log_error("max is smaller than min");
  } 

  // check alpha
  if (alpha < 0.1 || alpha > 1.9) {
     log_warn("your alpha %f is out of range. I use alpha = 1.0 instead.", alpha);
     alpha = 1.0;
  }
  if (alpha == 1.0) { 
     // this is uniform weight !
     result[0] = (random * l) + min;
     result[1] = 1.0; // no weight
     return result;
  }

  // calc x 
  // a = (1-alpha) / l
  // b = alpha
  // c = -r*l
  double sqrt_term = sqrt(alpha*alpha + 4*(1-alpha)*random);
  double x1 = (l / (2 - 2*alpha)) * (-alpha - sqrt_term);
  double x2 = (l / (2 - 2*alpha)) * (-alpha + sqrt_term);
  double x = -2;
  if (x1 >= 0 && x1 <= max - min) { 
     x = x1;
  } else if (x2 >= 0 && x2 <= max - min) {
     x = x2;
  } else {
     log_error("no valid x is found??");
  }

  //set xval
  result[0] = min + x;

  // calc weight
  double yl = (2 - 2*alpha)/l * x + alpha;
  result[1] = 1.0 / yl;

  return result;
}

//_________________________________________________________________
double Polynominal1DFunc::Sample_x_pol1(double rand)
{
  if (rand > intgmax_) {
     log_error("rand %f must be in (%f,%f)", rand, intgmin_, intgmax_);
  }
  return rand / b_;
}

//_________________________________________________________________
double Polynominal1DFunc::Sample_x_pol2(double rand)
{
  log_debug("rand %f range (%f,%f)" , rand, intgmin_, intgmax_);
  if (rand > intgmax_) {
     log_fatal("rand %f must be in (%f,%f)" , rand, intgmin_, intgmax_);
  }

  // a = 0.5*a_
  // b = b_
  // c = -rand
  double d = b_*b_ + 4 * 0.5 * a_ * rand;
  log_debug("rand %f, intgmin %f, intgmax %f, d = %f", rand, intgmin_, intgmax_, d);
  double sqrt_term = sqrt(d);
  double x1 = (-b_ + sqrt_term) / a_;
  double x2 = (-b_ - sqrt_term) / a_;

  double x = -2;
  if ((x1 >= xmin_) && (x1 <= xmax_)) {
     x = x1;
  } else if ((x2 >= xmin_) && (x2 <= xmax_)) {
     x = x2;
  } else {
     log_error("something happended, x1 = %f, x2 = %f",  x1, x2);
  }
  return x;
}

//_________________________________________________________________
void FlatZenithEmulator::Initialize(double coszenmin, double coszenmax)
{

   mincos_ = coszenmin;
   maxcos_ = coszenmax;

   if (mincos_ < -1 or maxcos_ > 1) {
      log_error("mincos or maxcos out of range: mincos = %f maxcos = %f" , mincos_, maxcos_);
   }

   if (mincos_ > maxcos_) {
      log_error("maxcos %f must be larger than mincos %f" , maxcos_, mincos_);
   }


   // define nodes
   std::vector<double> xths1;
   std::vector<double> yvals1;
   xths1.push_back(-1.);    yvals1.push_back(30.);   // 0
   xths1.push_back(-0.998); yvals1.push_back(12);    // 1
   xths1.push_back(-0.995); yvals1.push_back(6.);    // 2
   xths1.push_back(-0.985); yvals1.push_back(4.);    // 3
   xths1.push_back(-0.97);  yvals1.push_back(2.5);   // 4
   xths1.push_back(-0.93);  yvals1.push_back(1.8);   // 5
   xths1.push_back(-0.85);  yvals1.push_back(1.25);  // 6
   xths1.push_back(-0.8);   yvals1.push_back(1.1);   // 7 
   xths1.push_back(-0.75);  yvals1.push_back(1.0);   // 8
   xths1.push_back(-0.5);   yvals1.push_back(0.75);  // 9
   xths1.push_back(-0.2);   yvals1.push_back(0.7);   // 10

   xths_.clear();
   yvals_.clear();
   unsigned int nhalfnodes = xths1.size();
   for (unsigned int i=0; i<nhalfnodes; ++i) {
      xths_.push_back(xths1[i]); 
      yvals_.push_back(yvals1[i]); 
   }

   // nodes are symmetry w.r.t coszen = 0
   for (unsigned int i=0; i<nhalfnodes; ++i) {
      xths_.push_back(-1*xths1[nhalfnodes-1 -i]);
      yvals_.push_back(yvals1[nhalfnodes-1 -i]);
   }

   for (unsigned int i=0; i<xths_.size(); ++i) {
      log_debug("i %d, xths_[i] = %f yvals_[i] = %f", i, xths_[i], yvals_[i]);
   }
   // number of functions are 1 smaller than size of
   // nodes(xths_)
   nfuncs_ = xths_.size() - 1;

   // generate functions and
   // set minimum and maximum in each function

   std::vector<Polynominal1DFunc> funcs;
   bool is_mincos_set = false;
   bool is_maxcos_set = false;
   for (unsigned int i = 0; i<nfuncs_; ++i) {
      log_debug("try to scan func i = %d, nfuncs = %d", i, nfuncs_);
      // y - yvals[i] = a(x - xths[i])
      // y = ax + yvals[i] - a*xths[i]
      double a = (yvals_[i+1] - yvals_[i]) / (xths_[i+1] - xths_[i]);
      double b = yvals_[i] - a * xths_[i];
      Polynominal1DFunc f1(a, b, i);

      // see upper threshold
      double upper_xth = xths_[i+1];
      double lower_xth = xths_[i];

      if (mincos_ < upper_xth) {
         // at least, mincos_ must be lower than upper_xths
         // to activate the function.

         if (is_mincos_set) {
            f1.SetXmin(lower_xth);
         } else {
            f1.SetXmin(mincos_);
            is_mincos_set = true;
         }

         if (maxcos_ < upper_xth) {
            f1.SetXmax(maxcos_);
            is_maxcos_set = true;
         } else if (is_mincos_set) {
            f1.SetXmax(upper_xth);
         }

         log_debug("store function id i = %d, a = %f, b = %f, minx = %f, maxx = %f, mincos = %f, maxcos = %f", i, f1.a_, f1.b_, f1.xmin_, f1.xmax_, mincos_, maxcos_);
         funcs.push_back(f1);
         if (is_maxcos_set) {
            break;
         }
      }
   }

   // update nfuncs
   nfuncs_ = funcs.size();

   // total integrals of functions
   double intgtotal = 0;

   // filling map with accumulated integral as key
   Polynominal1DFunc f0(0, 0);
   funcmap_.clear();
   funcmap_[0.] = f0;
   for (unsigned int i=0; i<nfuncs_; ++i) {
      double intg = funcs[i].GetTotalIntegral();
      funcs[i].DebugPrint();
      intgtotal += intg;
      funcmap_[intgtotal] = funcs[i];
   }

   std::map<double, Polynominal1DFunc>::iterator i;
   for (i=funcmap_.begin(); i != funcmap_.end(); ++i) {
       double total = i->first;
       Polynominal1DFunc f = i->second;
       log_debug("total section %f, lowedge %f, highedge %f, funcid %d", total, f.xmin_, f.xmax_, f.funcid_);
   }
} 

//_________________________________________________________________
std::vector<double> FlatZenithEmulator::Sampling(double r)
{
   // first: accumulated integral
   std::map<double,Polynominal1DFunc>::reverse_iterator
            riter = funcmap_.rbegin();
   double intgtotal = riter->first;

   if (intgtotal <= 0) {
       log_fatal("integral <= 0, can't sample zenith !");
   }

   // get random number to select area.
   double random = r * intgtotal;
   log_debug("r = %f, intgtotal = %e, random %f" ,r, intgtotal, random);

   std::map<double, Polynominal1DFunc>::iterator i;
   i = funcmap_.lower_bound(random);
   if (i == funcmap_.begin()) {
      // avoid first dummy
      i++;
   }

   Polynominal1DFunc thefunc = i->second;

   log_debug("falls area %d",  thefunc.GetFuncID());

   // shift random number in the range of the function 
   double accumintg = (--i)->first;
   double minintg = thefunc.GetMinIntegral();
   //log_debug("orig. random %f, shifted random %f, accumintg %f, minintg %f" , random, random-accumintg+minintg, accumintg, thefunc.GetMinIntegral());
   log_debug("orig. random %f, shifted random %f, accumintg %f, minintg %f" , random, random-accumintg+minintg, accumintg, thefunc.GetMinIntegral());

   random -= accumintg;
   random += minintg;
   double x = thefunc.Sample_x(random);
   double y = thefunc.Evaluate(x) / intgtotal; // normalized
   log_debug("x = %f, y = %f", x, y);

   // target distribution is 1/(maxcos - mincos).
   // get ratio as weight.
   double weight = (1. / (maxcos_ - mincos_)) / y;

   std::vector<double> result;
   result.push_back(x);
   result.push_back(weight);
   result.push_back(double(thefunc.GetFuncID()));
   return result;
}

}

