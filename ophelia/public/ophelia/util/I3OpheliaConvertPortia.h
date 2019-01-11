/**
 * copyright  (C) 2008
 * the icecube collaboration
 *
 * @class I3OpheliaConvertPortia
 * @file I3OpheliaConvertPortia.h
 * @version $Revision$
 * @date $Date$
 * @author mio
 *
 * This is an I3Module to convert dataclasses derived from portia project
 * to general dataclass.
 *    a) I3PortiaEvent to I3MapStringDouble
 *    b) I3PortiaPulseMap to I3RecoPulseSeriesMap
 */

#ifndef I3OPHELIACONVERTPORTIA_H_INCLUDED
#define I3OPHELIACONVERTPORTIA_H_INCLUDED

#include <string>
#include "icetray/I3ConditionalModule.h"

/**
 * @brief a module to convert I3OpheliaFirstGuessTrack to I3Particle
 */
class I3OpheliaConvertPortia : public I3ConditionalModule
{
 public:

  /**
   * Constructor
   */ 
  I3OpheliaConvertPortia(const I3Context& ctx);
  
  /**
   * Destructor
   */ 
  ~I3OpheliaConvertPortia();
  
  /**
   * This module takes a configuration parameter and so it must be configured
   */
  void Configure();

  /** 
   * Convert I3PortiaPulseMap into I3RecoPulseSeriesMap
   */ 
  //void DAQ(I3FramePtr frame);
  void Physics(I3FramePtr frame);
  

private:
  
  /* 
   * Default constructor, copy constructor, and assignment operator
   * declared private to prevent use
   */ 
  I3OpheliaConvertPortia();
  I3OpheliaConvertPortia(const I3OpheliaConvertPortia& source);
  I3OpheliaConvertPortia& operator=(const I3OpheliaConvertPortia& source);
  
  /**
   * Parameter: name of the input I3PortiaPulseMap. to be converted
   * into I3MapStringDouble.
   */
  std::string inputPortiaEvent_;

  /**
   * Parameter: name of the output I3MapStringDouble converted
   * from I3PortiaEvent.
   */
  std::string outputPortiaEventMap_;

  /**
   * Parameter: name of the input I3PortiaPulseMap. to be converted
   * into I3RecoPulseSeriesMap.
   */
  std::string inputPortiaPulse_;

  /**
   * Parameter: name of the output I3RecoPulseSeriesMap converted
   * from I3PortiaPulseMap.
   */
  std::string outputRecoPulse_;

  SET_LOGGER("I3OpheliaConvertPortia");

};  // end of class I3OpheliaConvertPortia

#endif //I3OPHELIACONVERTPORTIA_H_INCLUDED
