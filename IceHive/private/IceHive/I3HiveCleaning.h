/**
 * \file I3HiveCleaning.h
 *
 * (c) 2012 the IceCube Collaboration
 *
 * $Id: I3HiveCleaning.h 99900 2013-02-26 10:10:43Z mzoll $
 * \version $Revision: 99900 $
 * \date $Date: 2013-02-26 11:10:43 +0100 (Tue, 26 Feb 2013) $
 * \author Marcel Zoll <marcel.zoll@fysik.su.se>
 *
 * The IceTray I3Module wrapper around the central algorithm HiveCleaning
 */

#ifndef I3HIVECLEANING_H
#define I3HIVECLEANING_H

#include "icetray/I3ConditionalModule.h"
#include "icetray/I3Units.h"

#include "dataclasses/geometry/I3Geometry.h"
#include "dataclasses/physics/I3RecoPulse.h"
#include "dataclasses/I3MapOMKeyMask.h"

#include <boost/make_shared.hpp>

#include "IceHive/HiveCleaning.h"

///The main icetray module
template <class Response>
class I3HiveCleaning : public I3ConditionalModule {
  SET_LOGGER("I3HiveCleaning");
  
private: //definitions
  typedef std::vector<Response> ResponseSeries;
  typedef I3Map<OMKey,ResponseSeries> ResponseSeriesMap;
  typedef boost::shared_ptr<ResponseSeriesMap > ResponseSeriesMapPtr;
  typedef boost::shared_ptr<const ResponseSeriesMap > ResponseSeriesMapConstPtr;
//FIXME until there is no general I3MapOMKeyMask, use hardcodings
//   typedef I3MapOMKeyMask<Response> ResponseSeriesMapMask;
//   typedef boost::shared_ptr<ResponseSeriesMapMask > ResponseSeriesMapMaskPtr;
//   typedef boost::shared_ptr<const ResponseSeriesMapMask> ResponseSeriesMapMaskConstPtr;
  typedef I3RecoPulseSeriesMapMask ResponseSeriesMapMask;
  typedef I3RecoPulseSeriesMapMaskPtr ResponseSeriesMapMaskPtr;
  typedef I3RecoPulseSeriesMapMaskConstPtr ResponseSeriesMapMaskConstPtr;
  
protected://parameters
  //========================
  // Configurable Parameters
  //========================
  /// PARAM: Name of the pulses to clean in the frame
  std::string inputName_;
  /// PARAM: Name of the cleaned pulses to put into the frame
  std::string outputName_;
  /// PARAMs which are delivered to HiveCleaning
  HiveCleaning_ParameterSet hc_param_set_;
  /// PARAM: which stream to execute on
  I3Frame::Stream stream_;

private://properties
  ///most private HiveCleaning instance
  HiveCleaning* hiveCleaning_;
  
private: //bookkeeping  
  //number of p-frames passed by
  uint64_t n_frames_;
  //number of empty pulse-series produced
  uint64_t n_empty_;
  
private://methods
  /** Reconfigure the subordinated modules
   * @param frame a frame containing the GCD objects, as forward propagated hand-downs
   */
  void Reconfigure(I3FramePtr frame);

public://methods
  //================
  // Main Interface
  //================
  /// Constructor: configure Default values, register Parameters, register Outbox
  I3HiveCleaning(const I3Context& context);
  
  ///Destructor
  ~I3HiveCleaning();

  /// Configure method to interact with icetray
  void Configure();

  /// General call method to interact with icetray : Needs to be hooked up to an execution stream
  void PerformCleaning(I3FramePtr frame);

  /// Geometry call method to interact with Geometry-frame
  void Geometry(I3FramePtr frame);
  
  ///report some numbers of interest
  void Finish();
};

I3_MODULE(I3HiveCleaning<I3RecoPulse>);


//==================== IMPLEMENTATIONS =============================
#include "icetray/I3Units.h"
#include "icetray/I3Int.h"
#include "dataclasses/I3MapOMKeyMask.h"
#include "dataclasses/physics/I3EventHeader.h"

//===============class I3HiveCleaning=================================
template <class Response>
I3HiveCleaning<Response>::I3HiveCleaning(const I3Context& context):
  I3ConditionalModule(context),
  inputName_(""),
  outputName_(""),
  hc_param_set_(),
  stream_(I3Frame::Physics),
  //initialize the splitter algorithms
  hiveCleaning_(NULL),
  //bookkeeping
  n_frames_(0),
  n_empty_(0)
{
  log_debug("Creating I3HiveCleaning instance");
  AddParameter("InputName", "Name of the input pulses in the Q-frame", inputName_);
  AddParameter("OutputName", "Name of the processed pulses will be written to", outputName_);
  AddParameter("Multiplicity", "Required multiplicity of connected neighbours", hc_param_set_.multiplicity);
  AddParameter("TimeStaticMinus",
               "Maximum positive time which will allow a close-by pair of hits to be considered connected in [ns]",
               hc_param_set_.timeStaticMinus);
  AddParameter("TimeStaticPlus",
               "Maximum positive time which will allow a close-by pair of hits to be considered connected in [ns]",
               hc_param_set_.timeStaticPlus);
  AddParameter("SingleDenseRingVicinity",
               "Limits for on-ring distances in pairs Minus/Plus in [m] for static terms, if considering strings in the regular IceCube hexagonal structure",
               hc_param_set_.SingleDenseRingVicinityPairs);
  AddParameter("DoubleDenseRingVicinity",
               "Limits for on-ring distances in pairs Minus/Plus in [m] for static terms, if the hexagonal structure is regularly filled by one more string",
               hc_param_set_.DoubleDenseRingVicinityPairs);
  AddParameter("TripleDenseRingVicinity",
               "Limits for on-ring distances in pairs Minus/Plus in [m] for static terms, if the hexagonal structure is regularly filled by one more string",
               hc_param_set_.TripleDenseRingVicinityPairs);
  AddParameter("Stream", "The Stream to execute on", stream_);
  AddOutBox("OutBox");

  log_info("This is I3HiveCleaning!");
  log_debug("Leaving Init()");
}

template <class Response>
I3HiveCleaning<Response>::~I3HiveCleaning() {
  delete hiveCleaning_;
}

template <class Response>
void I3HiveCleaning<Response>::Configure() {
  log_debug("Entering Configure()");
  GetParameter("InputName", inputName_);
  GetParameter("OutputName", outputName_);
  GetParameter("Multiplicity", hc_param_set_.multiplicity);
  GetParameter("TimeStaticMinus", hc_param_set_.timeStaticMinus);
  GetParameter("TimeStaticPlus", hc_param_set_.timeStaticPlus);
  GetParameter("SingleDenseRingVicinity", hc_param_set_.SingleDenseRingVicinityPairs);
  GetParameter("DoubleDenseRingVicinity", hc_param_set_.DoubleDenseRingVicinityPairs);
  GetParameter("TripleDenseRingVicinity", hc_param_set_.TripleDenseRingVicinityPairs);
  GetParameter("Stream", stream_);
  
  if (inputName_=="")
    log_fatal("Configure the Name of the Input!");
  if (outputName_=="")
    log_fatal("Configure the Name of the Output!");

  Register(stream_, &I3HiveCleaning::PerformCleaning);

  //gathered enough information to instantiate and configure HiveCleaning
  hiveCleaning_ = new HiveCleaning(hc_param_set_);

  log_debug("Leaving Configure()");
}

template <class Response>
void I3HiveCleaning<Response>::Geometry(I3FramePtr frame){
  Reconfigure(frame);
  PushFrame(frame);
}


template <class Response>
void I3HiveCleaning<Response>::Finish() {
  log_notice_stream("In "<<n_empty_ <<" of "<<n_frames_ <<" cases everything has been cleaned away.");
};


template <class Response>
void I3HiveCleaning<Response>::Reconfigure(I3FramePtr frame) {
  I3GeometryConstPtr geometry = frame->Get<I3GeometryConstPtr>();
  if (!geometry)
    log_fatal("Unable to find <I3Geometry>('I3Geometry') in frame!");

  hiveCleaning_->BuildLookUpTables(*geometry);
}


template <class Response>
void I3HiveCleaning<Response>::PerformCleaning (I3FramePtr frame) {
  log_debug("Entering PerformCleaning()");

  using namespace HitSorting;
  
  //fetch inputs
  if (!frame->Has(inputName_)) {
    log_error_stream("Could not locate the <RecoPulsesSeriesMap>("<<inputName_ <<") in the frame; nothing to be done");
    PushFrame(frame);
    return;
  }
  
  n_frames_++; //bookkeeping
  
  HitSorting::I3RecoPulseSeriesMap_HitFacility hitFacility(frame, inputName_);
  
  const HitSorting::HitSeries hits = hitFacility.GetHits<HitSorting::HitSeries>();

  //turn the crank
  log_debug("Performing Cleaning");  
  const HitSorting::HitSeries subEvent = hiveCleaning_->Clean(hits);
  
  if (! subEvent.size()){
      log_debug("everything was cleaned away");
      n_empty_++;
  }
  
  const ResponseSeriesMapMask mask = hitFacility.MaskFromHits(subEvent);  
  frame->Put(outputName_, boost::make_shared<ResponseSeriesMapMask>(mask));
  
  PushFrame(frame);

  log_debug("Leaving PerformCleaning()");
};

#endif
