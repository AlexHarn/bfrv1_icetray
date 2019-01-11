/**
 * \file I3HiveSplitter.h
 *
 * (c) 2012 the IceCube Collaboration
 *
 * $Id: HiveSplitter.h 99900 2013-02-26 10:10:43Z mzoll $
 * \version $Revision: 99900 $
 * \date $Date: 2013-02-26 11:10:43 +0100 (Tue, 26 Feb 2013) $
 * \author Marcel Zoll <marcel.zoll@fysik.su.se>
 *
 * The IceTray I3Module wrapper arround the central algorithm HiveSplitter
 */

#ifndef I3HIVESPLITTER_H
#define I3HIVESPLITTER_H

#include "icetray/I3ConditionalModule.h"
#include "phys-services/I3Splitter.h"
#include "icetray/I3Units.h"
#include "icetray/I3Int.h"
#include "dataclasses/I3Constants.h"
#include "dataclasses/geometry/I3Geometry.h"
#include "dataclasses/physics/I3RecoPulse.h"
#include "dataclasses/I3MapOMKeyMask.h"

#include <boost/make_shared.hpp>

#include "HiveSplitter/HiveSplitter.h"

///The main splitter module
class I3HiveSplitter : public I3ConditionalModule, private I3Splitter{
protected:
  //========================
  // Configurable Parameters
  //========================

  /// PARAM: Name of the pulses to split
  std::string inputName_;
  /// PARAM: Name of the pulses to put in the split frames
  std::string outputName_;
  /// PARAM: Whether to save an integer in the frame indicating the number of subevents generated
  bool saveSplitCount_;
  /// PARAM: Mode to not split but rather unite all output clusters into one single output Map
  uint noSplitOpt_;
  /// PARAM: modify the EventHeader of the new subframes with the Start and EndTime of the Subpulses
  bool modifyObjectsOpt_;
  ///Params which are delivered to HiveSplitter
  HiveSplitter_ParameterSet param_set_;
  /// flag to prohibit missuse: splitter needs to be configured before running any Q-frames
  bool splitter_configured_;
  
private:
  HiveSplitter hiveSplitter_;

public:
  //================
  // Main Interface
  //================
  /// Constructor: configure Default values, register Parameters, register Outbox
  I3HiveSplitter(const I3Context& context);

  /// Configure Methode to interact with icetray
  void Configure();

  /// DAQ call Methode to interact with icetray
  void DAQ(I3FramePtr frame);

  /// Geometry call methode to interact with Geometry frame
  void Geometry(I3FramePtr frame);
};

I3_MODULE(I3HiveSplitter);

#endif