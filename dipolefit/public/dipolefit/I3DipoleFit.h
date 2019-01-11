/**
 * copyright  (C) 2004
 * the icecube collaboration
 *  $Id$
  *
 * @file I3DipoleFit.h
 * @version $Revision: 1.2 $
 * @date $Date$
 * @author troy
 */

#ifndef I3DIPOLEFIT_H
#define I3DIPOLEFIT_H

#include "icetray/I3ConditionalModule.h"

/**
 * @brief IceTray module to implement the dipole fit first guess routine
 */
class I3DipoleFit : public I3ConditionalModule
{
 public:

  /**
   * Builds an instance of this class
   * @param ctx the context with which this module's built
   */
  I3DipoleFit(const I3Context& ctx);
  
  /**
   * Destroys an instance of this class
   */
  ~I3DipoleFit();

  /**
   * This module takes a configuration parameter and so it must be configured
   */
  void Configure();

  /**
   * Physics method operates on the event (the physics frame), once per frame.
   */
  void Physics(I3FramePtr frame);

 private:
  // default, assignment, and copy constructor declared private
  I3DipoleFit();
  I3DipoleFit(const I3DipoleFit&);
  I3DipoleFit& operator=(const I3DipoleFit&);

  /**
   * Parameter: ame to assign to the fit that is produced.
   */
  std::string fitName_;

  /**
   * Parameter: name of the input RecoPulseSeriesMap to use for reconstruction.
   */
  std::string inputRecoPulses_;

  /**
   * Parameter: minimum number of hits needed to fit the event.
   */
  int minHits_;

  /**
   * Parameter: step size in the dipole calculation.
   * 
   * The dipole fit will (after sorting the hits to time) determine the
   * 'dipole' direction as:
   *     sum(i) weight(i,i+N) * (pos(i+N) - pos(i) )
   *     divided by the sum of the weights.
   * With the DipoleStep option you set the value of N.
   */
  int dipoleStep_;

  /**
   * Parameter: power of hits weights.
   * 
   * Hits are weighted with the amplitude raised to this power. Typically 0. 
   * (for all hits weight=1) or 1. (weight=amplitude).
   */
  double ampWeightPower_;

  SET_LOGGER("I3DipoleFit");

};


#endif 
