/**
 * copyright  (C) 2004
 * the icecube collaboration
 * @file I3TensorOfInertia.h
 * Version $Id$
 * @version $Revision: 1.3 $
 * @date $Date$
 * @author Sean Grullon <grullon@icecube.wisc.edu>
 */

#ifndef I3TENSOROFINERTIA_H_INCLUDED
#define I3TENSOROFINERTIA_H_INCLUDED

#include "icetray/I3ConditionalModule.h"
#include "icetray/I3Logging.h"
#include "dataclasses/geometry/I3Geometry.h"
#include "dataclasses/physics/I3RecoPulse.h"
#include "dataclasses/I3Map.h"


/**
 * This is an IceTray module to implement the Inertia tensor fit first guess algorithm.
 * It works by calculating a Tensor of Inertia from the hit optical modules in the 
 * event, using the amplitudes as virtual masses.  From the tensor the eigenvales
 * and eigenvectors can be calculated, the smallest eigenvalue corresponds 
 * to the longest axis, which approximates the direction of the track.  
 *
 * The tensor of inertia algorithm treats the pulse amplitidues of the PMTs as virtual 
 * masses, with each PMT The amplitudes of the OMs at position r_i from the center of 
 * gravity of the hits.  The center of gravity of the virtual mass distribution is given by 
 * \f[
 * \vec{COG}= \sum_{i=1}^N a_i^w*\vec{r_i}
 * \f]
 * where a_i is the ith PMT amplitude.  The Inertia Tensor itself is given by the usual 
 * formula with PMT amplitudes replacing the masses: 
 * \f[
 * I^{k,l}=\sum_{i=1}^N a_i^w*(\delta^{kl}*\vec{r_i}^2-r_i^k*r_i^j).
 * \f]
 *
 * The amplitude weight w can be set arbitrarily, with 0 and 1 the two most common values.  
 * (1 itself is the default value) The smallest eigenvalue  of the inertia tensor 
 * corresponds to the longest axis, which approximates the track if the smallest 
 * eigenvalue is much smaller than the other two eigenvalues.
 */

class I3TensorOfInertia : public I3ConditionalModule
{
public:

  I3TensorOfInertia(const I3Context& ctx);

  
  ~I3TensorOfInertia(){}
   
  void Configure();
  
 /**
 * Excecute the Inertia Tensor fit first guess reconstruction on the event
 * in the provided frame.  The algorithm is adapted from the one used in Siegmund.
 */ 

  void Physics(I3FramePtr frame);

       
  private:
  
  //I3TensorOfInertia(); 
  
  I3TensorOfInertia(const I3TensorOfInertia& source);
    
    /* 
     *  The Assignment operator here is overloaded to  copy the attributes of the 
     * target to the calling object 
     */ 
  I3TensorOfInertia& operator=(const I3TensorOfInertia& source);
  
  

  /* Here are the private variables the InertiaTensorFit module needs.  */
  
  // Name to assign to fit
  std::string moduleName_;
  
    // selector to use to select OMResponses
  std::string inputSelection_;
  
    // DataReadout to use
  std::string inputReadout_;
    
  // Minimum number of hits I need to fit the event
  int minHits_;
  
    // Amplitude Weight applied to the virtual masses 
    // needed for the InertiaTensor fit algorithm.
  double ampWeight_;

  // allow log level configuration via log4cplus.conf
  SET_LOGGER( "I3TensorOfInertia" );
  
};  

#endif //I3TENSOROFINERTIA_H

