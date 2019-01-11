/**
 * copyright  (C) 2004
 * the icecube collaboration
 *
 * @class I3OpheliaConvertJulietParticle
 * @file I3OpheliaConvertJulietParticle.h
 * @version $Revision$
 * @date $Date$
 * @author mase
 */

#ifndef I3OPHELIACONVERTJULIETPARTICLE_H_INCLUDED
#define I3OPHELIACONVERTJULIETPARTICLE_H_INCLUDED

#include "icetray/I3ConditionalModule.h"

/**
 * @brief a module to convert I3JulietParticle to I3Particle
 */
class I3OpheliaConvertJulietParticle : public I3ConditionalModule
{
 public:

  /**
   * Constructor
   */ 
  I3OpheliaConvertJulietParticle(const I3Context& ctx);
  
  /**
   * Destructor
   */ 
  ~I3OpheliaConvertJulietParticle();
  
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
  I3OpheliaConvertJulietParticle();
  I3OpheliaConvertJulietParticle(const I3OpheliaConvertJulietParticle& source);
  I3OpheliaConvertJulietParticle& operator=(const I3OpheliaConvertJulietParticle& source);
  

  /**
   * Parameter: name of the input I3OpheliaJulietParticle to be converted
   * into I3Particle.
   */
  std::string inputMCTree_;

  /**
   * Parameter: name of the output I3Particle converted from 
   * I3OpheliaJulietParticle.
   */
  std::string outputParticle_;


  SET_LOGGER("I3OpheliaConvertJulietParticle");

};  // end of class I3OpheliaConvertJulietParticle

#endif
