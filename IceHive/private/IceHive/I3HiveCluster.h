/**
 * \file I3HiveCluster.h
 *
 * (c) 2012 the IceCube Collaboration
 *
 * $Id: I3HiveCluster.h 99900 2013-02-26 10:10:43Z mzoll $
 * \version $Revision: 99900 $
 * \date $Date: 2013-02-26 11:10:43 +0100 (Tue, 26 Feb 2013) $
 * \author Marcel Zoll <marcel.zoll@fysik.su.se>
 *
 * The IceTray I3Module wrapper around the HiveSplitter and use of it as a cleaning module
 */

#ifndef I3HIVECLUSTER_H
#define I3HIVECLUSTER_H

#include "icetray/I3ConditionalModule.h"
#include "icetray/I3Units.h"

#include "dataclasses/geometry/I3Geometry.h"
#include "dataclasses/physics/I3RecoPulse.h"
#include "dataclasses/I3MapOMKeyMask.h"

#include <boost/make_shared.hpp>

#include "IceHive/HiveSplitter.h"

///The main icetray module
template <class Response>
class I3HiveCluster : public I3ConditionalModule {
  SET_LOGGER("I3HiveCluster");
  
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
  HiveSplitter_ParameterSet hs_param_set_;
  /// PARAM: which stream to execute on
  I3Frame::Stream stream_;

private://properties
  ///most private HiveCleaning instance
  HiveSplitter* hiveSplitter_;
  
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
  I3HiveCluster(const I3Context& context);
  
  ///Destructor
  ~I3HiveCluster();

  /// Configure method to interact with icetray
  void Configure();

  /// General call method to interact with icetray : Needs to be hooked up to an execution stream
  void PerformCleaning(I3FramePtr frame);

  /// Geometry call method to interact with Geometry-frame
  void Geometry(I3FramePtr frame);
};

I3_MODULE(I3HiveCluster<I3RecoPulse>);


//==================== IMPLEMENTATIONS =============================
#include "icetray/I3Units.h"
#include "icetray/I3Int.h"
#include "dataclasses/I3MapOMKeyMask.h"
#include "dataclasses/physics/I3EventHeader.h"

//===============class I3HiveCluster=================================
template <class Response>
I3HiveCluster<Response>::I3HiveCluster(const I3Context& context):
  I3ConditionalModule(context),
  inputName_(""),
  outputName_(""),
  hs_param_set_(),
  stream_(I3Frame::Physics),
  //initialize the splitter algorithms
  hiveSplitter_(NULL)
{
  log_debug("Creating I3HiveCluster instance");
  AddParameter("InputName", "Name of the input pulses in the Q-frame", inputName_);
  AddParameter("OutputName", "Name of the processed pulses will be written to", outputName_);
  AddParameter("Multiplicity", "Required multiplicity in causal connected hits to form a subevent", hs_param_set_.multiplicity);
  AddParameter("TimeWindow", "Time span within which the multiplicity requirement must be met in [ns]", hs_param_set_.timeWindow);
  AddParameter("TimeStatic", "Maximum time which will "
                "allow a close-by pair of hits to be considered connected in [ns]", hs_param_set_.timeStatic);
  AddParameter("TimeCVMinus", "Maximum negative deviation from speed of light (in vacuum) travel time which will "
                "allow a pair of hits to be considered connected in [ns]", hs_param_set_.timeCVMinus);
  AddParameter("TimeCVPlus", "Maximum positive deviation from speed of light (in vacuum) travel time which will "
                "allow a pair of hits to be considered connected in [ns]", hs_param_set_.timeCVPlus);
  AddParameter("TimeCNMinus", "Maximum positive deviation from speed of light (in ice/water) travel time which will "
                "allow a pair of hits to be considered connected in [ns]", hs_param_set_.timeCNMinus);
  AddParameter("TimeCNPlus", "Maximum positive deviation from speed of light (in ice/water) travel time which will "
                "allow a pair of hits to be considered connected in [ns]", hs_param_set_.timeCNPlus);
  AddParameter("SingleDenseRingLimits",
               "Limits for on-ring distances in pairs Minus/Plus in [m] for propagation terms, if considering strings in the regular IceCube hexagonal structure",
               hs_param_set_.SingleDenseRingLimitPairs);
  AddParameter("DoubleDenseRingLimits",
               "Limits for on-ring distances in pairs Minus/Plus in [m] for propagation terms, if the hexagonal structure is regularly filled by one more string",
               hs_param_set_.DoubleDenseRingLimitPairs);
  AddParameter("TripleDenseRingLimits",
               "Limits for on-ring distances in pairs Minus/Plus in [m] for propagation terms, if the hexagonal structure is regularly filled with six more strings",
               hs_param_set_.TripleDenseRingLimitPairs);
  AddParameter("SingleDenseRingVicinity",
               "Limits for on-ring distances in pairs Minus/Plus in [m] for static terms, if considering strings in the regular IceCube hexagonal structure",
               hs_param_set_.SingleDenseRingVicinityPairs);
  AddParameter("DoubleDenseRingVicinity",
               "Limits for on-ring distances in pairs Minus/Plus in [m] for static terms, if the hexagonal structure is regularly filled by one more string",
               hs_param_set_.DoubleDenseRingVicinityPairs);
  AddParameter("TripleDenseRingVicinity",
               "Limits for on-ring distances in pairs Minus/Plus in [m] for static terms, if the , hexagonal structure is regularly filled with six more strings",
               hs_param_set_.TripleDenseRingVicinityPairs);
  AddParameter("Stream", "The Stream to execute on", stream_);
  AddOutBox("OutBox");

  log_info("This is I3HiveCluster!");
  log_debug("Leaving Init()");
}

template <class Response>
I3HiveCluster<Response>::~I3HiveCluster() {
  delete hiveSplitter_;
}

template <class Response>
void I3HiveCluster<Response>::Configure() {
  log_debug("Entering Configure()");
  GetParameter("InputName", inputName_);
  GetParameter("OutputName", outputName_);
  GetParameter("Multiplicity", hs_param_set_.multiplicity);
  GetParameter("TimeWindow", hs_param_set_.timeWindow);
  GetParameter("TimeStatic", hs_param_set_.timeStatic);
  GetParameter("TimeCVMinus", hs_param_set_.timeCVMinus);
  GetParameter("TimeCVPlus", hs_param_set_.timeCVPlus);
  GetParameter("TimeCNMinus", hs_param_set_.timeCNMinus);
  GetParameter("TimeCNPlus", hs_param_set_.timeCNPlus);
  GetParameter("SingleDenseRingLimits", hs_param_set_.SingleDenseRingLimitPairs);
  GetParameter("DoubleDenseRingLimits", hs_param_set_.DoubleDenseRingLimitPairs);
  GetParameter("TripleDenseRingLimits", hs_param_set_.TripleDenseRingLimitPairs);
  GetParameter("SingleDenseRingVicinity", hs_param_set_.SingleDenseRingVicinityPairs);
  GetParameter("DoubleDenseRingVicinity", hs_param_set_.DoubleDenseRingVicinityPairs);
  GetParameter("TripleDenseRingVicinity", hs_param_set_.TripleDenseRingVicinityPairs);
  GetParameter("Stream", stream_);
  
  if (inputName_=="")
    log_fatal("Configure the Name of the Input!");
  if (outputName_=="")
    log_fatal("Configure the Name of the Output!");

  Register(stream_, &I3HiveCluster::PerformCleaning);

  //gathered enough information to instantiate and configure HiveSplitter
  hiveSplitter_ = new HiveSplitter(hs_param_set_);

  log_debug("Leaving Configure()");
}

template <class Response>
void I3HiveCluster<Response>::Geometry(I3FramePtr frame){
  Reconfigure(frame);
  PushFrame(frame);
}


template <class Response>
void I3HiveCluster<Response>::Reconfigure(I3FramePtr frame) {
  I3GeometryConstPtr geometry = frame->Get<I3GeometryConstPtr>();
  if (!geometry)
    log_fatal("Unable to find <I3Geometry>('I3Geometry') in frame!");

  hiveSplitter_->BuildLookUpTables(*geometry);
}


template <class Response>
void I3HiveCluster<Response>::PerformCleaning (I3FramePtr frame) {
  log_debug("Entering PerformCleaning()");

  using namespace HitSorting;
  
  //fetch inputs
  ResponseSeriesMapConstPtr pulses = frame->Get<ResponseSeriesMapConstPtr>(inputName_);
  if (!pulses) {
    log_error_stream("Could not locate the <RecoPulsesSeriesMap>("<<inputName_ <<") in the frame; will do nothing");
    PushFrame(frame); //nothing to do here
    return;
  }

  //turn the crank
  log_debug("Performing Cleaning");
  
  //fetch inputs
  if (!frame->Has(inputName_)) {
    log_error_stream("Could not locate the key '"<<inputName_ <<"' in the frame; nothing to be done");
    PushFrame(frame);
    return;
  }
  
  HitSorting::I3RecoPulseSeriesMap_HitFacility hitFacility(frame, inputName_);
  
  const HitSorting::HitSeries hits = hitFacility.GetHits<HitSorting::HitSeries>();
  
  const HitSorting::HitSeriesSeries subEvents = hiveSplitter_->Split(hits);
  
  HitSorting::HitSeries unitedHits;
  BOOST_FOREACH(const HitSorting::HitSeries &subEvent, subEvents)
    unitedHits.insert(unitedHits.end(), subEvent.begin(), subEvent.end());
    
  const ResponseSeriesMapMask mask = hitFacility.MaskFromHits(unitedHits);  
  frame->Put(outputName_, boost::make_shared<ResponseSeriesMapMask>(mask));
  
  PushFrame(frame);

  log_debug("Leaving PerformCleaning()");
};

#endif
