/**
 * copyright  (C) 2004
 * the icecube collaboration
 *  $Id$
  *
 * @file I3LineFit.h
 * @version $Revision: 1.2 $
 * @date $Date$
 * @author deyoung
 */

#ifndef I3LINEFIT_H_INCLUDED
#define I3LINEFIT_H_INCLUDED

#include "icetray/I3ConditionalModule.h"

/**
 * @brief IceTray module to implement the linefit first guess routine
 */
class I3LineFit : public I3ConditionalModule
{
 public:

  /**
   * Constructor:  builds an instance of the module, with the
   * context provided by IceTray. 
   */ 
  I3LineFit(const I3Context& ctx);
  
  /**
   * Destructor: deletes the module
   */ 
  ~I3LineFit();
  
  /**
   * This module takes a configuration parameter and so it must be configured
   */
  void Configure();

  /** 
   * If event readout data comes by, I'll perform a line fit reconstruction.
   */ 
  void Physics(I3FramePtr frame);
  
  // Not interested in any other types of data (geometry,
  // calibration, etc.) so I don't declare a function for them.  
  
private:
  
  /* 
   * Default constructor, copy constructor, and assignment operator
   * declared private to prevent use
   */ 
  I3LineFit();
  I3LineFit(const I3LineFit& source);
  I3LineFit& operator=(const I3LineFit& source);
  
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
   * Parameter: either "ALL" or "FLE" --> whether to use all LeadingEdges
   * (hits) or just the first one.
   */
  std::string leadingEdge_;

  /**
   * Parameter: power of hits weights.
   * 
   * Hits are weighted with the amplitude raised to this power. Typically 0. 
   * (for all hits weight=1) or 1. (weight=amplitude).
   */
  double ampWeightPower_;

  // Named stream for my logging messages.  Allows my messages to be
  // sent to a particular log file, to be set to a special threshold, etc.
  SET_LOGGER("I3LineFit");

};  // end of class I3LineFit

#endif
