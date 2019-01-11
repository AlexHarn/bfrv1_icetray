/**
 * copyright  (C) 2004
 * the icecube collaboration
 *
 * @class I3OpheliaConvertFirstGuessTrack
 * @file I3OpheliaConvertFirstGuessTrack.h
 * @version $Revision$
 * @date $Date$
 * @author mase
 */

#ifndef I3OPHELIACONVERTFIRSTGUESS_H_INCLUDED
#define I3OPHELIACONVERTFIRSTGUESS_H_INCLUDED

#include "icetray/I3ConditionalModule.h"

/**
 * @brief a module to convert I3OpheliaFirstGuessTrack to I3Particle
 */
class I3OpheliaConvertFirstGuessTrack : public I3ConditionalModule
{
 public:

  /**
   * Constructor
   */ 
  I3OpheliaConvertFirstGuessTrack(const I3Context& ctx);
  
  /**
   * Destructor
   */ 
  ~I3OpheliaConvertFirstGuessTrack();
  
  /**
   * This module takes a configuration parameter and so it must be configured
   */
  void Configure();

  /** 
   * 
   */ 
  void Physics(I3FramePtr frame);
  

private:
  
  /* 
   * Default constructor, copy constructor, and assignment operator
   * declared private to prevent use
   */ 
  I3OpheliaConvertFirstGuessTrack();
  I3OpheliaConvertFirstGuessTrack(const I3OpheliaConvertFirstGuessTrack& source);
  I3OpheliaConvertFirstGuessTrack& operator=(const I3OpheliaConvertFirstGuessTrack& source);
  

  /**
   * Parameter: name of the input I3OpheliaFirstGuessTrack to be converted
   * into I3Particle.
   */
  std::string inputOpheliaFGTrack_;

  /**
   * Parameter: name of the output I3Particle converted from 
   * I3OpheliaFirstGuessTrack.
   */
  std::string outputParticle_;


  SET_LOGGER("I3OpheliaConvertFirstGuessTrack");

};  // end of class I3OpheliaConvertFirstGuessTrack

#endif
