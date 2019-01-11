/**
 * Copyright (C) 2008
 * The IceCube collaboration
 * ID: $Id: $
 *
 * @file I3VEMCalExtractor.h
 * @version $Rev: $
 * @date $Date: $
 * @author tilo
 */


#ifndef _VEMCAL_I3VEMCALEXTRACTOR_H_
#define _VEMCAL_I3VEMCALEXTRACTOR_H_


#include <icetray/I3ConditionalModule.h>
#include <vemcal/I3VEMCalData.h>
#include <string>

/**
 * @brief This module extracts all the information which is needed to perform
 * the muon calibration and writes the data in a condensed format to the frame.
 */

class I3VEMCalExtractor : public I3ConditionalModule 
{
public:
  I3VEMCalExtractor(const I3Context& context);

  virtual ~I3VEMCalExtractor();
  
  /// Re-implementation of the Configure method of the I3Module
  void Configure();
  
  /// Re-implementation of the Physics method of the I3Module
  void DAQ(I3FramePtr frame);
  
private:
  
  /**
   * Extracts the minimum bias his from the frame and stores the
   * hit information in the I3VEMCalData container.
   */
  void FillVEMData(const I3Frame& frame, I3VEMCalData& vemCalData);
  
  /**
   * Extracts the HG-LG correlation hits from the normal physics
   * pulses and stores the information in the I3VEMCalData container.
   */
  void FillHGLGData(const I3Frame& frame, I3VEMCalData& vemCalData);
  
  /**
   * Check if minbias hit is coincident with air shower hits
   */
  bool HasLCHits(const I3Frame& frame);
  
  
  /// Name of the IceTop minimum bias pulses in the frame
  std::string minbiasPulsesName_;

  /// Name of the IceTop minimum bias pulse info in the frame
  std::string minbiasPulseInfoName_;
  
  /// Name of the IceTop physics pulses in the frame
  std::string icetopPulsesName_;

  /// Name of the IceTop physics pulse info in the frame
  std::string icetopPulseInfoName_;
  
  /// Name of the IceTop veto source in the frame
  std::string showerVetoName_;
  
  /// Output name of the I3VEMCalData container in the frame
  std::string vemCalDataName_;
  
  SET_LOGGER("I3VEMCalExtractor");
};


#endif
