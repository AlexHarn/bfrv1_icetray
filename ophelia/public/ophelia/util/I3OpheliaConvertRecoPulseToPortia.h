/**
 * copyright  (C) 2008
 * the icecube collaboration
 *
 * @class I3OpheliaConvertRecoPulseToPortia
 * @file I3OpheliaConvertRecoPulseToPortia.h
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
 * @brief a module to convert I3RecoPulseSeries to I3PortiaPulse
 */
class I3OpheliaConvertRecoPulseToPortia : public I3ConditionalModule
{
 public:

  /**
   * Constructor
   */ 
  I3OpheliaConvertRecoPulseToPortia(const I3Context& ctx);
  
  /**
   * Destructor
   */ 
  ~I3OpheliaConvertRecoPulseToPortia();
  
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
  I3OpheliaConvertRecoPulseToPortia();
  I3OpheliaConvertRecoPulseToPortia(const I3OpheliaConvertRecoPulseToPortia& source);
  I3OpheliaConvertRecoPulseToPortia& operator=(const I3OpheliaConvertRecoPulseToPortia& source);
  
  /**
   * Parameter: name of the input I3PortiaPulseMap. to be converted
   * into I3RecoPulseSeriesMap.
   */
  std::string outputPortiaPulse_;

  /**
   * Parameter: name of the output I3RecoPulseSeriesMap converted
   * from I3PortiaPulseMap.
   */
  std::string inputRecoPulse_;

  SET_LOGGER("I3OpheliaConvertRecoPulseToPortia");

};  // end of class I3OpheliaConvertRecoPulseToPortia

#endif //I3OPHELIACONVERTPORTIA_H_INCLUDED
