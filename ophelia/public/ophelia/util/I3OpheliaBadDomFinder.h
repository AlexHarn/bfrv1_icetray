/**
 * copyright  (C) 2004
 * the icecube collaboration
 *
 * @class I3OpheliaBadDomFinder
 * @file I3OpheliaBadDomFinder.h
 * @version $Revision$
 * @date $Date$
 * @author mase
 */

#ifndef I3OPHELIABADDOMFINDER_H_INCLUDED
#define I3OPHELIABADDOMFINDER_H_INCLUDED

#include "icetray/I3ConditionalModule.h"
#include "icetray/OMKey.h"
#include "dataclasses/I3Vector.h"
#include "dataclasses/calibration/I3Calibration.h"

/**
 * @brief Find bad DOMs and update bad DOM list in the frame
 */
class I3OpheliaBadDomFinder : public I3ConditionalModule
{
 public:

  /* allow log level configuration via log4cplus.conf */
  SET_LOGGER("I3OpheliaBadDomFinder");

  /**
   * Constructor
   */ 
  I3OpheliaBadDomFinder(const I3Context& ctx);
  
  /**
   * Destructor
   */ 
  ~I3OpheliaBadDomFinder();
  
  /**
   * This module takes a configuration parameter and so it must be configured
   */
  void Configure();

  /** 
   * Physics
   */ 
  void Physics(I3FramePtr frame);
  

private:
  
  /* 
   * Default constructor, copy constructor, and assignment operator
   * declared private to prevent use
   */ 
  I3OpheliaBadDomFinder();
  I3OpheliaBadDomFinder(const I3OpheliaBadDomFinder& source);
  I3OpheliaBadDomFinder& operator=(const I3OpheliaBadDomFinder& source);
  

  /**
   * Function to check transit time and add new bad DOM
   */
  void CheckTransitTime(const I3OMGeoMap& omgeo,
			const std::map<OMKey, I3DOMCalibration>& domCal, 
			I3VectorOMKeyPtr vect_omkey);

  /**
   * Parameter: name of the in/output bad DOM list
   */
  std::string inputBadDomListName_;
  std::string outputBadDomListName_;

  /**
   * Parameter: option to check PMT transit time in I3Calibration
   */
  bool checkTransitTime_;

};  // end of class I3OpheliaBadDomFinder

#endif // I3OPHELIABADDOMFINDER_H_INCLUDED
