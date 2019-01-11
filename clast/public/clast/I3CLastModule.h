#ifndef _I3_CLAST_MODULE_H_
#define _I3_CLAST_MODULE_H_

/**
 * copyright  (C) 2004
 * the icecube collaboration
 * @file I3CLastModule.h
 * Version $Id: I3CLast.h 42854 2008-02-29 22:52:12Z joanna $
 * @version $Revision: 1.3 $
 * @date $Date: 2008-02-29 15:52:12 -0700 (Fri, 29 Feb 2008) $
 * @author Pat Toale (derived from TensorOfInteria code by Sean Grullon)
 */

// header files
#include "icetray/I3ConditionalModule.h"
#include "icetray/I3Logging.h"
#include "dataclasses/geometry/I3Geometry.h"
#include "dataclasses/physics/I3RecoPulse.h"
#include "dataclasses/I3Map.h"


/**
 *
 * @class I3CLastModule
 * @brief Seed Estimator for Cascade
 *
 * The main module for generating a complete seed for cascade analysis
 * uses the tensor of inertia reconstruction to estimate a direction,
 * the COG to estimate a vertex position, and the cfirst-ish light-cone
 * algorithm to estimate the start time (though, unlike cfirst, this
 * code will always return a start time since, if the cfirst algorithm
 * fails, it will return the hit time of the first hit in the event.)
 *
 */

class I3CLastModule : public I3ConditionalModule
{
 public:

  I3CLastModule(const I3Context& ctx);

  ~I3CLastModule(){}
   
  void Configure();
  
  /**
   * Excecute the CLast first guess reconstruction on the event
   * in the provided frame.
   */ 
  void Physics(I3FramePtr frame);

       
 private:
  
  I3CLastModule(const I3CLastModule& source);
    
  /* 
   *  The Assignment operator here is overloaded to  copy the attributes of the 
   * target to the calling object 
   */ 
  I3CLastModule& operator=(const I3CLastModule& source);
    
  /* Here are the private variables the InertiaTensorFit module needs.  */
  
  // Name to assign to fit
  std::string moduleName_;
  
  // bool flag to select the detector type for which the energy estimate is made
  bool   AmandaMode_;
  
  // DataReadout to use
  std::string inputReadout_;
    
  // Minimum number of hits I need to fit the event
  int minHits_;
  
  // Amplitude Weight applied to the virtual masses 
  // needed for the InertiaTensor fit algorithm.
  double ampWeight_;
  
  // Amplitude option: treatment of small pulses 
  int ampOpt_;

  // Parameters for the 'cfirst' time algorithm
  double directHitRadius_;
  double directHitWindow_;
  int directHitThreshold_;

  // Parameters for energy estimator from I3 hits
  double e0_;
  double e1_;
  double e2_;
  double e3_;
  double e4_;
  double e5_;

  // Parameters for energy estimator from AM hits
  double a0_;
  double a1_;
  double a2_;
  double a3_;
  double a4_;
  double a5_;

  // allow log level configuration via log4cplus.conf
  SET_LOGGER( "I3CLastModule" );
  
};  // end of the class definition.

#endif /* _I3_CLAST_MODULE_H_ */

