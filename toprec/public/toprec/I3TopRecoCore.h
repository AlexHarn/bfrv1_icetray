/**
 * Copyright (C) 2004
 * The IceCube collaboration
 * ID: $Id$
 *
 * @file I3TopRecoCore.h
 * @version $Rev: 27576 $
 * @date $Date$
 * @author $Author: klepser $
 */

#ifndef __I3TOPRECOCORE_H_
#define __I3TOPRECOCORE_H_

#include "icetray/I3ConditionalModule.h"


/**
   @class I3TopRecoCore 
   @brief Takes station responses and calculates the core by making a
   COG calculation on the positions
 
*/
class I3TopRecoCore : public I3ConditionalModule 
{
 private:
  
  bool fVerbose; // verbosity
  
  std::string fShowerCoreName;
  std::string fDataReadoutLabel;
  
  double fPower; 
  short int fNtanks;

  // some default values for the above
  static const bool DEFAULT_VERBOSE;
  static const std::string DEFAULT_DATA_READOUT_LABEL;
  static const std::string DEFAULT_SHOWERCORENAME;
  static const bool DEFAULT_USE_HITS;
  static const double DEFAULT_WEIGHTING_POWER;
  static const short int DEFAULT_NTANKS; 

  static const std::string VERBOSE_TAG;
  static const std::string DATA_READOUT_TAG;
  static const std::string SHOWERCORENAME_TAG;
  static const std::string USE_HITS_TAG;
  static const std::string WEIGHTING_POWER_TAG;
  static const std::string NTANKS_TAG;

  static const std::string VERBOSE_DESCRIPTION;
  static const std::string DATA_READOUT_DESCRIPTION;
  static const std::string SHOWERCORENAME_DESCRIPTION;
  static const std::string USE_HITS_DESCRIPTION;
  static const std::string WEIGHTING_POWER_DESCRIPTION;
  static const std::string NTANKS_DESCRIPTION;
    
  // some define like statements which allow the compiler to check if
  // we made a mistake inside the I3Stream calls
  static const std::string PHYSICS_STREAM_NAME;
  static const std::string GEOMETRY_STREAM_NAME;
  static const std::string PHYSICS_STREAM_TITLE;
  static const std::string GEOMETRY_STREAM_TITLE;
  static const std::string OUTBOX_NAME;
  static const std::string INBOX_NAME;

  
 public:

  /**
   * The constructor getting a context
   */
  I3TopRecoCore (const I3Context& ctx);

  /**
   * The destructor
   */
  ~I3TopRecoCore ();

  /**
   * Configure the tracer
   */
  void Configure();

  /**
   * Get a physics event
   */
  void Physics (I3FramePtr frame);

 private:

  SET_LOGGER("I3TopRecoCore");
};

#endif

