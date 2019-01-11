/**
 * \file I3IceHive.h
 *
 * (c) 2012 the IceCube Collaboration
 *
 * $Id: I3IceHive.h 99900 2013-02-26 10:10:43Z mzoll $
 * \version $Revision: 99900 $
 * \date $Date: 2013-02-26 11:10:43 +0100 (Tue, 26 Feb 2013) $
 * \author Marcel Zoll <marcel.zoll@fysik.su.se>
 *
 * The IceTray I3Module wrapper around the central algorithm HiveSplitter and TriggerSplitter,
 * which in turn use a API trough SubEventStartStop
 */

#ifndef I3ICEHIVE_H
#define I3ICEHIVE_H

#include "icetray/I3ConditionalModule.h"
#include "phys-services/I3Splitter.h"
#include "icetray/I3Units.h"
#include "icetray/I3Int.h"
#include "dataclasses/I3Constants.h"
#include "dataclasses/geometry/I3Geometry.h"
#include "dataclasses/calibration/I3Calibration.h"
#include "dataclasses/status/I3DetectorStatus.h"
#include "dataclasses/physics/I3RecoPulse.h"
#include "dataclasses/I3MapOMKeyMask.h"

#include <boost/make_shared.hpp>

#include "IceHive/HiveSplitter.h"
#include "IceHive/TriggerSplitter.h"

#define IH_CONFIG_ID 999 //Give HiveSplitter a UNIQUE trigger configuration ID

///The main module which unites the algorithms HiveSplitter and TriggerSplitter
template <class Response>
class I3IceHive : public I3ConditionalModule,
                  private I3Splitter {
  SET_LOGGER("I3IceHive");

private: //definitions
  typedef std::vector<Response> ResponseSeries;
  typedef I3Map<OMKey,ResponseSeries> ResponseSeriesMap;
  typedef boost::shared_ptr<ResponseSeriesMap> ResponseSeriesMapPtr;
  typedef boost::shared_ptr<const ResponseSeriesMap> ResponseSeriesMapConstPtr;
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
  /// PARAM: Name of the pulses to split in the frame
  std::string inputName_;
  /// PARAM: Name of the pulses to put in the split frames
  std::string outputName_;
  /// PARAM: Name of the TriggerHierarchy in the frame
  std::string trigHierName_;
  /// PARAM: Whether to save an integer in the frame indicating the number of subevents generated
  bool saveSplitCount_;
  /// PARAM: Update the TriggerHierarchy with the cut out triggers
  bool updateTrigHierarchy_;
  ///Params which are delivered to HiveSplitter
  HiveSplitter_ParameterSet hs_param_set_;
  ///Params which are delivered to TriggerSplitter
  TriggerSplitter_ParameterSet ts_param_set_;
  /// PARAM: Use TriggerSplitter prior to HiveSplitter
  bool triggerSplitterOpt_;
  /// PARAM: Length of readout window to pad before event/trigger [ns]
  double readWindowMinus_;
  /// PARAM: Length of readout window to pad after event/trigger [ns]
  double readWindowPlus_;
  
private: //bookkeeping  
  //number of q-frames passed by
  uint64_t n_frames_;
  //number of splits produced
  uint64_t n_splits_;

private: //properties and methods related to configuration
  //hide this parameter for now
  /// pad that many ns noise in front and back of the events
  double noisePadding_;
  //facilitate the splitting
  ///most private HiveSplitter instance
  HiveSplitter* hiveSplitter_;
  ///most private TriggerSplitter instance
  TriggerSplitter* triggerSplitter_;

  //keep pointers to vital objects
  /// has any of the vital objects changed?
  bool configuration_changed_;
  /// store if we have seen all necessary frames GCD
  bool G_frame_seen_, C_frame_seen_, D_frame_seen_;

  /** @brief (Re)configure the splitters with new GCD objects
   * @param frame the frame that should have GCD objects present
   */
  void ConfigureSplitters(const I3FramePtr frame);
  
  /** @brief update the triggerHierarchy with a subtrigger for IceHive
   * @param trigHier the triggerHierarchy to augment
   * @param start_time the starttime of the trigger
   * @param end_time the starttime of the trigger
   * @param configID a Configuraturation ID for the added Trigger
   */
  I3TriggerHierarchy AugmentTriggerHierarchy (I3TriggerHierarchy trigHier,
                                              const int start_time,
                                              const int end_time,
                                              const int configID = IH_CONFIG_ID) const;

public: //methods
  //================
  // Main Interface
  //================
  /// Constructor: configure Default values, register Parameters, register Outbox
  I3IceHive(const I3Context& context);
  
  /// Destructor
  virtual ~I3IceHive();

  /// Finish routine printing a little bit of information 
  void Finish();
  
  /// Configure Method to interact with icetray
  void Configure();

  /// DAQ call Method to interact with icetray
  void DAQ(I3FramePtr frame);

  /// Geometry call method to interact with Geometry-frame
  void Geometry(I3FramePtr frame);

  /// Calibration call method to interact with Calibration-frame
  void Calibration(I3FramePtr frame);

  /// DetectorStatus call method to interact with Status-frame
  void DetectorStatus(I3FramePtr frame);
};

/// Instantiate IceHive for the use with I3RecoPulses, aka working on I3RecoPulseSeriesMap(Masks)
I3_MODULE(I3IceHive<I3RecoPulse>);


//============================= IMPLEMENTATIONS ==================
#include "icetray/I3Units.h"
#include "icetray/I3Int.h"
#include "dataclasses/I3MapOMKeyMask.h"
#include "dataclasses/physics/I3EventHeader.h"
#include "IceHive/IceHiveHelpers.h"

//===============class I3IceHive=================================
template <class Response>
I3IceHive<Response>::I3IceHive(const I3Context& context):
  I3ConditionalModule(context),
  I3Splitter(configuration_),
  inputName_(),
  outputName_(),
  trigHierName_("I3TriggerHierarchy"),
  saveSplitCount_(false),
  updateTrigHierarchy_(false),
  //parameter sets for subordinated modules
  hs_param_set_(),
  ts_param_set_(),
  //derived parameters
  triggerSplitterOpt_(false),
  readWindowMinus_(ts_param_set_.tWindowMinus),
  readWindowPlus_(ts_param_set_.tWindowPlus),
  //bookkeeping
  n_frames_(0),
  n_splits_(0),
  //initialize the splitter algorithms
  hiveSplitter_(NULL),
  triggerSplitter_(NULL),
  //do we need to reconfigure?
  configuration_changed_(true),
  G_frame_seen_(false),
  C_frame_seen_(false),
  D_frame_seen_(false)
{
  log_debug("Creating HiveSplitter instance");
  AddParameter("InputName", "Name of the input pulses in the Q-frame", inputName_);
  AddParameter("OutputName", "OutputName of the processed pulses", outputName_);
  AddParameter("SaveSplitCount", "Whether to save an integer in the frame indicating the number of "
                "subevents generated", saveSplitCount_);
  AddParameter("UpdateTriggerHierarchy", "Should the TriggerHIerarchy by updated", updateTrigHierarchy_);
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
  AddParameter("SingleDenseRingLimits", "Limits for on-ring distances in pairs Minus/Plus in [m] for propagation terms, if considering strings in the regular IceCube hexagonal structure", hs_param_set_.SingleDenseRingLimitPairs);
  AddParameter("DoubleDenseRingLimits", "Limits for on-ring distances in pairs Minus/Plus in [m] for propagation terms, if the hexagonal structure is regularly filled by one more string", hs_param_set_.DoubleDenseRingLimitPairs);
  AddParameter("TripleDenseRingLimits", "Limits for on-ring distances in pairs Minus/Plus in [m] for propagation terms, if the hexagonal structure is regularly filled with six more strings", hs_param_set_.TripleDenseRingLimitPairs);
  AddParameter("SingleDenseRingVicinity", "Limits for on-ring distances in pairs Minus/Plus in [m] for static terms, if considering strings in the regular IceCube hexagonal structure", hs_param_set_.SingleDenseRingVicinityPairs);
  AddParameter("DoubleDenseRingVicinity", "Limits for on-ring distances in pairs Minus/Plus in [m] for static terms, if the hexagonal structure is regularly filled by one more string", hs_param_set_.DoubleDenseRingVicinityPairs);
  AddParameter("TripleDenseRingVicinity", "Limits for on-ring distances in pairs Minus/Plus in [m] for static terms, if the , hexagonal structure is regularly filled with six more strings", hs_param_set_.TripleDenseRingVicinityPairs);
  AddParameter("UseTriggerSplitter", "Use Splitting with TriggerSplitter previous to application of HiveSplitter", triggerSplitterOpt_);
  AddParameter("TrigHierName", "Name of the input TriggerHierarchy", trigHierName_);
  AddParameter("TriggerConfigIDs", "Vector of Trigger-IDs for the relevant triggers. If vector remains empty the complete time-window is considered; Default [1006(SMT8), 1007(String), 1011(SMT3), 21001(Volume)]", ts_param_set_.configIDs);
  AddParameter("NoSplitDt", "Don't split for trigger time difference lower than this value in [ns] (end of previous trigger to the start of next)", ts_param_set_.noSplitDt);
  AddParameter("ReadoutWindowMinus", "Length of readout window to pad before event/trigger in [ns]. This time is used in the TriggerSplitter pulse extraction and is added to the '_Noised' pulse output for every subevent.", readWindowMinus_);
  AddParameter("ReadoutWindowPlus", "Length of readout window to pad after end of event/trigger in [ns]. This time is used in the TriggerSplitter pulse extraction and is added to the '_Noised' pulse output for every subevent.", readWindowPlus_);
  AddParameter("SubEventStreamName", "The name of the SubEvent stream.", configuration_.InstanceName()); 
  AddOutBox("OutBox");

  log_info("This is I3IceHive!");
  log_debug("Leaving Init()");
}

template <class Response>
I3IceHive<Response>::~I3IceHive() {
  delete hiveSplitter_;
  delete triggerSplitter_;
}

template <class Response>
void I3IceHive<Response>::Configure() {
  log_debug("Entering Configure()");
  GetParameter("InputName", inputName_);
  GetParameter("OutputName", outputName_);
  GetParameter("SaveSplitCount", saveSplitCount_);
  GetParameter("UpdateTriggerHierarchy", updateTrigHierarchy_);
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
  GetParameter("UseTriggerSplitter", triggerSplitterOpt_);
  GetParameter("TrigHierName", trigHierName_);
  GetParameter("TriggerConfigIDs", ts_param_set_.configIDs);
  GetParameter("NoSplitDt", ts_param_set_.noSplitDt);
  GetParameter("ReadoutWindowMinus", readWindowMinus_);
  GetParameter("ReadoutWindowPlus", readWindowPlus_);
  GetParameter("SubEventStreamName", sub_event_stream_name_);
  
  if (inputName_.empty())
    log_fatal("Configure the Name of the Input");
  if (outputName_.empty())
    log_fatal("Configure the Name of the Output");
  if (trigHierName_.empty())
    log_fatal("Configure the Name of the TriggerHierarchy");

  if (triggerSplitterOpt_ && trigHierName_.empty())
    log_fatal("Choosing the option 'UseTriggerSplitter' you have to configure 'TrigHierName'");

  //gathered enough information to instantiate and configure HiveSplitter and if requested TriggerSplitter
  if (triggerSplitterOpt_) {
    log_warn("You chose to apply TriggerSplitter prior to HiveSplitter");
    log_warn("It is strongly advised not to do so;"
    "please consider removing this option and use suitable modules independent of the TriggerHierarchy");
    ts_param_set_.tWindowMinus = readWindowMinus_;
    ts_param_set_.tWindowPlus = readWindowPlus_;
    hiveSplitter_ = new HiveSplitter(hs_param_set_);
    triggerSplitter_ = new TriggerSplitter(ts_param_set_);
  }else{
    log_info("You chose to apply HiveSplitter only");
    log_warn("You may get NoiseClusters in the Output from long events, you need to account and remove them with appropriate measures");
    if (!updateTrigHierarchy_){
      log_warn("The TriggerHierarchy will not be updated either!");
    }
    hiveSplitter_ = new HiveSplitter(hs_param_set_);
  }
  log_debug("Leaving Configure()");
}


template<class Response>
void I3IceHive<Response>::Finish(){
  log_debug("Entering Finish()");
  log_notice_stream(GetName()<<": Processed "<<n_frames_<<" frames producing "<<n_splits_<<" splits");
  I3Module::Finish();
};

template <class Response>
void I3IceHive<Response>::Geometry(I3FramePtr frame){
  log_debug("Entering Geometry()");
  G_frame_seen_ = true;
  configuration_changed_ = true;
  PushFrame(frame);
}


template <class Response>
void I3IceHive<Response>::Calibration(I3FramePtr frame){
  log_debug("Entering Calibration()");
  C_frame_seen_ = true;
  configuration_changed_ = true;
  PushFrame(frame);
}


template <class Response>
void I3IceHive<Response>::DetectorStatus(I3FramePtr frame){
  log_debug("Entering DetectorStatus()");
  D_frame_seen_ = true;
  configuration_changed_ = true;
  if (G_frame_seen_ && C_frame_seen_ && D_frame_seen_) {//reconfigure the module when a complete GCD has passed
    ConfigureSplitters(frame);
    configuration_changed_= false;
  }
  PushFrame(frame);
}


template <class Response>
void I3IceHive<Response>::ConfigureSplitters(const I3FramePtr frame) {
  log_debug("Entering ConfigureSplitters()");

  I3GeometryConstPtr geometry;
  I3CalibrationConstPtr calibration;
  I3DetectorStatusConstPtr status;
  
  geometry= frame->Get<I3GeometryConstPtr>();
  if (!geometry)
    log_fatal("Unable to find Geometry data!");

  if (C_frame_seen_ && triggerSplitterOpt_) {
    calibration = frame->Get<I3CalibrationConstPtr>();
    if (!calibration)
      log_fatal("Unable to find Calibration data!");
  }
  if (D_frame_seen_ && triggerSplitterOpt_) {
    status = frame->Get<I3DetectorStatusConstPtr>();
    if (!status)
      log_fatal("Unable to find DetectorStatus data!");
  }
  
  //enough information to build the look-up tables
  hiveSplitter_->BuildLookUpTables(*geometry); //an always
  if (triggerSplitterOpt_) {
    if (!calibration || !status) {
      log_warn("No Calibration or DetectorStatus present for the use with TriggerSplitter");
      triggerSplitter_->DefaultOffsetMap();
    }
    else
      triggerSplitter_->BuildOffsetMap(*calibration, *status);
  }
}


template <class Response>
I3TriggerHierarchy
I3IceHive<Response>::AugmentTriggerHierarchy (I3TriggerHierarchy trigHier,
                                              const int start_time,
                                              const int end_time,
                                              const int configID) const {
  
  I3Trigger hsTrigger; //create the HiveSplitter-trigger
  hsTrigger.GetTriggerKey() = TriggerKey(TriggerKey::IN_ICE, TriggerKey::FRAGMENT_MULTIPLICITY, configID);
  hsTrigger.SetTriggerFired(true);
  hsTrigger.SetTriggerTime(start_time);
  hsTrigger.SetTriggerLength(end_time - start_time);

  I3Trigger tpTrigger; //create the individual throughput trigger associated to the HiveSplitter-trigger
  tpTrigger.GetTriggerKey() = TriggerKey(TriggerKey::GLOBAL, TriggerKey::THROUGHPUT);
  tpTrigger.SetTriggerFired(true);
  tpTrigger.SetTriggerTime(start_time-readWindowMinus_);
  tpTrigger.SetTriggerLength((end_time+readWindowPlus_) - (start_time-readWindowMinus_));
  //append the throughput trigger to the global trigger
  I3TriggerHierarchy::iterator eachTrig = trigHier.append_child(trigHier.end(), tpTrigger);
  //insert actual trigger as child of the individual throughput
  trigHier.append_child(eachTrig, hsTrigger);

  return trigHier;
};


template <class Response>
void I3IceHive<Response>::DAQ (I3FramePtr frame) {
  log_debug("Entering DAQ()");
  using namespace HitSorting;
  
  if (configuration_changed_) { //this is a precaution if a late reconfiguring is needed, when only single G,C,D frame was passed
    ConfigureSplitters(frame);
    configuration_changed_= false;
  }

  // Get the trigger times and lengths
  I3TriggerHierarchyConstPtr trigHier = frame->Get<I3TriggerHierarchyConstPtr>(trigHierName_);
  //NOTE need to init and keep this pointer already here, even if it should be void, for optional trigger-splitting and updating the triggerHierarchy
  if(!trigHier) {
    if (triggerSplitterOpt_ || updateTrigHierarchy_) {
      log_fatal_stream("Could not locate the <I3TriggerHierarchy>'"<<trigHierName_ <<"' in the frame");
    }
  }
  
  //fetch inputs
  if (!frame->Get<I3RecoPulseSeriesMapConstPtr>(inputName_)) { //NOTE probe for the existence of this map
    log_fatal_stream("Could not locate the <I3RecoPulseSeriesMap/Mask> '"<<inputName_ <<"' in the frame; nothing to be done");
  }

  //turn the crank:
  I3RecoPulseSeriesMap_HitFacility hitFacility(frame, inputName_);

  const HitSorting::HitSeries hits = hitFacility.GetHits<HitSorting::HitSeries>();

  //perform the splitting
  HitSeriesSeries subEvents;
  if (triggerSplitterOpt_) {
    const HitSeriesSeries trigger_subEvents = triggerSplitter_->Split(*trigHier, hits);
    BOOST_FOREACH(const HitSeries &trigger_subEvent, trigger_subEvents) {
      subEvents = hiveSplitter_->Split(trigger_subEvent);
    }
  }
  else {
    subEvents = hiveSplitter_->Split(hits);
  }

  
  //Save the SplitCount if so requested
  if (saveSplitCount_) {
    //Some users are concerned with how many subevents we created, so let's save them some trouble
    const unsigned int nSubEvents=subEvents.size();
    frame->Put(GetName()+"SplitCount", boost::make_shared<I3Int>(nSubEvents));
  }

  // create a mask holding all hits from all the subevents at once, and the inversion of it
  HitSeries unitedHits;
  BOOST_FOREACH(const HitSeries &hits, subEvents) {
    unitedHits.insert(unitedHits.end(), hits.begin(), hits.end());
  }
  sort(unitedHits.begin(), unitedHits.end());

  const ResponseSeriesMapMask unitedMask = hitFacility.MaskFromHits(unitedHits);

  ResponseSeriesMapMask noiseMask(*frame, inputName_, *(frame->Get<ResponseSeriesMapConstPtr>(inputName_)));
  noiseMask = noiseMask ^ unitedMask; //invert the mask

  frame->Put(outputName_+"_Physics", boost::make_shared<ResponseSeriesMapMask>(unitedMask));
  frame->Put(outputName_+"_Noise", boost::make_shared<ResponseSeriesMapMask>(noiseMask));

  //need to delay until here
  PushFrame(frame); // push the Q-frame

  //process the subevents, put them each into a new subFrame and augment with further objects
  BOOST_FOREACH(const HitSeries &subEvent, subEvents) {
    I3FramePtr subframe = GetNextSubEvent(frame);
    //modify the I3EventHeader of the newly created subframe
    I3EventHeader mod_eh = *subframe->Get<I3EventHeaderConstPtr>("I3EventHeader");
    const double &rel_start_time = subEvent.front().GetTime();
    const double &rel_end_time = subEvent.back().GetTime();
    const I3Time abs_start_time = mod_eh.GetStartTime();
    mod_eh.SetStartTime(abs_start_time+rel_start_time);
    mod_eh.SetEndTime(abs_start_time+rel_end_time);
    subframe->Delete("I3EventHeader");
    subframe->Put("I3EventHeader", boost::make_shared<I3EventHeader>(mod_eh));
    //convert the subevent Hit set into a proper mask on the pulses series
    const ResponseSeriesMapMask subevent_mask = hitFacility.MaskFromHits(subEvent);
    subframe->Put(outputName_, boost::make_shared<ResponseSeriesMapMask>(subevent_mask));
    //also create a separate object holding start and end times of the event
    const I3TimeWindow subevent_time_window(subEvent.front().GetTime(), subEvent.back().GetTime());
    subframe->Put(outputName_ + "_TimeRange", boost::make_shared<I3TimeWindow>(subevent_time_window));

    //Get me the noise within the time window of that subevent; this is an overlay
    ResponseSeriesMapConstPtr noisePulses = noiseMask.Apply(*frame);
    
    const I3TimeWindow noised_time_window(subEvent.front().GetTime()-readWindowMinus_,
                                          subEvent.back().GetTime()+readWindowPlus_);
    
    const ResponseSeriesMap noiseInRange = IceHiveHelpers::GetPulsesInTimeRange (*noisePulses, noised_time_window);
    const ResponseSeriesMapMask mask_noiseInRange(*frame, noiseMask.GetSource(), noiseInRange);

    const ResponseSeriesMapMask noised_mask = subevent_mask | mask_noiseInRange;
    subframe->Put(outputName_+"_Noised", boost::make_shared<ResponseSeriesMapMask>(noised_mask));

    if (updateTrigHierarchy_) {
      I3TriggerHierarchy trigHier_clipped = IceHiveHelpers::ClipTriggerHierarchy(*trigHier,
                                                                               noised_time_window,
                                                                               ts_param_set_.configIDs);
      //when this option is selected clip the Trigger Hierarchy to the according time-range and insert a Trigger for HiveSplitter
      trigHier_clipped = AugmentTriggerHierarchy(trigHier_clipped,
                                                 subEvent.front().GetTime(),
                                                 subEvent.back().GetTime());
      
      subframe->Put(trigHierName_+"_clipped", boost::make_shared<I3TriggerHierarchy>(trigHier_clipped));
    }

    n_splits_++;
    
    PushFrame(subframe); //push the subevent P-frame 
  }
  n_frames_++;
  
  log_debug("Leaving DAQ()");
};

#endif
