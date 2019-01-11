/**
 * copyright  (C) 2004
 * the icecube collaboration
 *
 * @class I3PortiaEventOMKeyConverter
 * @file I3PortiaEventOMKeyConverter.h
 * @version $Revision$
 * @date $Date$
 * @author mase
 */

#ifndef I3PORTIAEVENTOMKEYCONVERTER_H_INCLUDED
#define I3PORTIAEVENTOMKEYCONVERTER_H_INCLUDED

#include "icetray/I3ConditionalModule.h"

/**
 * @brief a module to convert I3PortiaEvent to OMKey
 */
class I3PortiaEventOMKeyConverter : public I3ConditionalModule
{
 public:

  /**
   * Constructor
   */ 
  I3PortiaEventOMKeyConverter(const I3Context& ctx);
  
  /**
   * Destructor
   */ 
  ~I3PortiaEventOMKeyConverter();
  
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
  I3PortiaEventOMKeyConverter();
  I3PortiaEventOMKeyConverter(const I3PortiaEventOMKeyConverter& source);
  I3PortiaEventOMKeyConverter& operator=(const I3PortiaEventOMKeyConverter& source);
  

  /**
   * Parameter: name of the input I3PortiaEvent to be converted
   * into OMKey.
   */
  std::string inputPortiaEvent_;

  /**
   * Parameter: name of the output OMKey converted from 
   * I3PortiaEvent
   */
  std::string outputOMKeyListName_;

  SET_LOGGER("I3PortiaEventOMKeyConverter");

};  // end of class I3PortiaEventOMKeyConverter

#endif
