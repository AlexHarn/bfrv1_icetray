#ifndef IPDF_TMinuitMinimizer_H
#define IPDF_TMinuitMinimizer_H

/**
 *
 * (c) 2005
 * the IceCube Collaboration
 * $Id$
 *
 * @file TMinuitMinimizer.h
 * @version $Revision: 1.3 $
 * @date $Date$
 * @author Simon Robbins
 */

#include "TMinuit.h"

#include "dataclasses/I3Position.h"
#include "dataclasses/I3Direction.h"
#include <cassert>

namespace IPDF {

typedef double minimization_result;

  /**
   * @brief Factors out interface to TMinuit
   *
   * This is only a simple example, which should be extended in 
   * a generic likelihood reconstruction.
   *
   * @todo Could do with some TLC, docs and re-factoring
   */
template<int NDimensions>
class TMinuitMinimizer {
public:

  TMinuitMinimizer()
  : minuit_(NDimensions), converged_(false), result_(-9.e99) {
  }

  virtual ~TMinuitMinimizer() { }

  template<typename Likelihood,
	   typename Response,
	   typename Hypothesis>
  class MinimizationFunctor {
  public:
    MinimizationFunctor(
	const Likelihood& likelihood,
	const Response& response,
	const Hypothesis& seed)
	: likelihood_(likelihood), response_(response), seed_(seed)
	{ }

    minimization_result operator()(double params[]) {
      double like = likelihood_.getLogLikelihood(response_,
					   this->makeHypothesis(params));
      assert(finite(like) && "Non finite likelhood from ipdf");
      return (finite(like) ? -like : std::numeric_limits<double>::min());
    }

    Hypothesis makeHypothesis(const double params[]) const {
      assert(finite(params[0]));
      assert(finite(params[1]));
      assert(finite(params[2]));
      assert(finite(params[3]));
      assert(finite(params[4]));

      // Theta-phi coords used internally by ipdf
      I3Position  posn(params[0],params[1],params[2]);
      I3Direction dirn; dirn.SetThetaPhi(params[3],params[4]);
      //std::cout<<Hypothesis(posn,dirn)<<", theta "<<params[3]std::endl;
      return Hypothesis(posn,dirn,seed_.getEnergy(),seed_.getTZero());
    }
  private:
    const Likelihood& likelihood_;
    const Response&   response_;
    const Hypothesis& seed_;
  };

  template<typename Likelihood,
	   typename Response,
	   typename Hypothesis>
  Hypothesis minimize(
      const Likelihood& likelihood,
      const Response& response,
      const Hypothesis& seed) {

    MinimizationFunctor<Likelihood,Response,Hypothesis>
	minime(likelihood,response,seed);
    double iparams[NDimensions];
    this->minimize(minime,this->makeParams(seed,iparams));

    return minime.makeHypothesis(iparams);
  }

  template<typename Hypothesis>
  double* makeParams(const Hypothesis& hypothesis,double iparams[]) const {
    iparams[0] = hypothesis.getPointX();
    iparams[1] = hypothesis.getPointY();
    iparams[2] = hypothesis.getPointZ();
    iparams[3] = hypothesis.getTheta();
    iparams[4] = hypothesis.getPhi();
    return iparams;
  }
  
  /// Minimize a function implemented as a Functor
  template <class MinimizedFunction>
  void minimize(MinimizedFunction& f,
		double initial_params[]) {
      *(GetStaticPtr<MinimizedFunction>()) = &f;

      minuit_.SetFCN(fcn<MinimizedFunction>);

      // The step size for each parameter
      double step_size[NDimensions];
      for(unsigned int idim=0; idim<NDimensions ; ++idim) {
	step_size[idim] = 0.1;
      }

      double arglist[10];
      int ierflg = 0;
      
      for(unsigned int idim=0; idim<NDimensions; ++idim) {
	minuit_.mnparm(idim,"Example",initial_params[idim],step_size[idim],0,0,ierflg);
	if(ierflg == 4)
	  log_warn("Error setting parameter %d",idim);
      }
      
      minuit_.SetPrintLevel(-2);
//      minuit_.mnexcm("SET NOW",arglist,1,ierflg);
//      minuit_.SetErrorDef(0.5);
      // Have had problems with strategy two, so use default:
//      arglist[0] = 2;
//      minuit_.mnexcm("SET STR",arglist,1,ierflg);
      
      arglist[0]= 50000;

      minuit_.mnexcm("MIGRAD",arglist,1,ierflg);
      log_debug("ierflg %d:",ierflg);

      double err;
      for(unsigned int idim=0; idim<NDimensions; ++idim) {
	double x;
	minuit_.GetParameter(idim,x,err);
	initial_params[idim] = x;
      }

      *(GetStaticPtr<MinimizedFunction>()) = 0;

      result_ = f(initial_params);
      if(ierflg != 0) {
	log_error("TMinimizer failed to converge. %d", ierflg);
	converged_ = false;
      } else {
	log_debug("TMinimizer minimized. %d", ierflg);
	converged_ = true;
      }
    }

    bool converged() const { return converged_; }
    minimization_result result() { return result_; }

private:
  template <class MinimizedFunction>
  static MinimizedFunction** GetStaticPtr() {
    static MinimizedFunction* minimized;
    return &minimized;
  }

  TMinuit minuit_;
  bool converged_;
  minimization_result result_;

  /**
   * This is the global function that we're going to pass to TMinuit,
   * one for each kind of functor we're going to use.
   *
   * See the TMinuit docs for what all this stuff is.  It's pretty 
   * standard stuff.
   */
  template <class MinimizedFunction>
  static void  fcn(int& npar, double* gin, double& f, double*par, int iflag)
    {
      MinimizedFunction* function = *(GetStaticPtr<MinimizedFunction>());
      assert(function);
      assert(npar == NDimensions);
      double params[NDimensions];
      for(unsigned int idim=0; idim<NDimensions; ++idim) {
	params[idim] = par[idim];
      }
      f = (*function)(params);
      return ;
    }
  
};

} // namespace IPDF

#endif // IPDF_TMinuitMinimizer_H
