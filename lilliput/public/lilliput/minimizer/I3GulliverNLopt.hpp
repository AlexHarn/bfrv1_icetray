/**
 * @file I3GulliverNLopt.hpp
 * @brief Gulliver interface to the minimizers of the NLopt library
 *
 * Copyright (C) 2016 the IceCube Collaboration <http://www.icecube.wisc.edu>
 *
 * $Id: I3GSLSimplex.h 102172 2013-04-07 16:29:18Z boersma $
 *
 * @version $Revision: 102172 $
 * @date $Date: 2013-04-07 18:29:18 +0200 (Sun, 07 Apr 2013) $
 * @author Kevin Meagher
 */

#ifndef I3GULLIVERNLOPT_H_INCLUDED
#define I3GULLIVERNLOPT_H_INCLUDED

#include "icetray/I3ServiceBase.h"
#include "gulliver/I3MinimizerBase.h"

/**
 * @class I3GulliverNLopt
 * @brief Gulliver interface to the minimizers of the NLopt library
 *        http://ab-initio.mit.edu/wiki/index.php/NLopt
 *
 */
class I3GulliverNLopt :  public I3ServiceBase,  public I3MinimizerBase
{
public:

  I3GulliverNLopt(const I3Context& context);

  I3GulliverNLopt(const std::string &name,
		  const std::string &algorithm,
		  double tol, int maxi);

  virtual ~I3GulliverNLopt(){}

  void Configure();

  I3MinimizerResult Minimize(I3GulliverBase &gulliver_instance,
                 const std::vector<I3FitParameterInitSpecs> &parspecs ) ;

  /// get the tolerance 
  double GetTolerance() const { return tolerance_; }

  /// set the tolerance 
  void SetTolerance(double newtol ){ tolerance_ = newtol; }

  /// get the maximum number of minimizer iterations
  unsigned int GetMaxIterations() const { return maxIterations_; }

  /// set the maximum number of minimizer iterations
  void SetMaxIterations(unsigned int newmaxi ){ maxIterations_ = newmaxi; }

  /// name to use for log_messages
  const std::string GetName() const {
    return I3ServiceBase::GetName();
  }

  /// tell gulliver weather we use gradients
  bool UsesGradient(){return uses_gradient_;}

  /// tell global or local
  bool IsGlobal(){return global_;}

  /// macro which sets the name to use to configure logging
  SET_LOGGER( "I3GulliverNLopt" );

  ///dictionary of possible minimization algorithms to use
  static const std::map<std::string,int> algorithms;
  
private:

  void set_algorithm(const std::string&);
  
  /// tolerance (of function value)
  double tolerance_;

  /// maximum number of simplex iterations
  int maxIterations_;

  /// algorithm to use
  int algorithm_;

  /// store weather we use gradients
  bool uses_gradient_;

  /// store weather global or local
  bool global_;
  
};

#endif
