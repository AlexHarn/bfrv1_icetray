/**
 *  
 * copyright (c) 2011
 * the IceCube Collaboration
 * $Id$
 *
 * @date $Date: 2011-10-22$
 * @author naoko
 *
 */

#include <icetray/I3Module.h>
#include <icetray/I3Context.h>
#include <icetray/I3Frame.h>
#include <icetray/I3Logging.h>
#include <boost/foreach.hpp>
#include <boost/make_shared.hpp>
#include <boost/assign.hpp>
#include <icetray/I3Units.h>
#include <dataclasses/I3Double.h>
#include <dataclasses/I3Map.h>
#include <dataclasses/I3TimeWindow.h>
#include <dataclasses/I3MapOMKeyMask.h>
#include <dataclasses/TriggerKey.h>
#include <dataclasses/physics/I3EventHeader.h>
#include <dataclasses/physics/I3Trigger.h>
#include <dataclasses/physics/I3TriggerHierarchy.h>
#include <dataclasses/physics/I3DOMLaunch.h>
#include <dataclasses/physics/I3RecoPulse.h>
#include <dataclasses/calibration/I3Calibration.h>
#include <dataclasses/status/I3DetectorStatus.h>
#include <phys-services/I3Splitter.h>
#include <algorithm>
#include <vector>
#include <string>
#include <cfloat>

class I3TriggerSplitter : public I3Module, private I3Splitter {
public:
  I3TriggerSplitter(const I3Context& context);
  void Configure();
  void DAQ(I3FramePtr frame);
  void Calibration(I3FramePtr frame);
  void DetectorStatus(I3FramePtr frame);
  
private:
  std::vector<std::string> inputResponses_;
  std::vector<std::string> outputResponses_;
  std::string trigHierName_;
  std::vector<int> configIDs_;
  double noSplitDt_;
  double tWindowMinus_;
  double tWindowPlus_;
  bool writeFrameTimes_;
  bool writeTimeWindow_;
  std::string updateTrigHierarchy_;
  double updateTrigHierarchyMode_;
  I3CalibrationConstPtr calibration_;
  I3DetectorStatusConstPtr status_;
  bool useCDinfoForDt_;
  void CalcLaunchDTs();
  double GetLaunchDT(const I3DOMCalibration &calib, const I3DOMStatus &status);
  
  typedef std::map<OMKey, double> offset_map_t;
  offset_map_t offsets_;
  double min_offset_, max_offset_;
  
  struct Trigs {
    double time;
    double length;
    
    bool operator<(const Trigs &other) const
    {
      return time < other.time;
    }
  };
  
  // A reduced readout window, with functions to trim 
  class SubTrigger {
  private:
    double start_time_;
    double end_time_;
    
    offset_map_t *offsets_;
    double min_offset_, max_offset_;
    OMKey current_key_;
    double current_offset_;
    
  public:	
    SubTrigger(double start, double end, offset_map_t *offsets, double min_offset,
               double max_offset) :
      start_time_(start), end_time_(end), offsets_(offsets),
      min_offset_(min_offset), max_offset_(max_offset),
      current_key_(offsets_->begin()->first),
      current_offset_(offsets_->begin()->second)
    {}
		
    // Select pulses that fall within this subtrigger
    bool operator()(const OMKey &key, size_t idx, const I3RecoPulse &pulse);
    // Create and store a mask of pulses that fall within this subtrigger
    void MaskPulses(I3FramePtr frame, const std::string &inputName,
		    const std::string &outputName);
    // Replace the I3EventHeader in the frame with one that contains
    // the start and end time of this subtrigger
    void UpdateHeader(I3FramePtr frame);
    // Rewrite the trigger hierarchy to only include triggers that
    // participate in this subtrigger
    void UpdateTriggers(I3FramePtr frame, I3TriggerHierarchyConstPtr trigHier,
                        const std::vector<int> &configIDs, std::pair<double, double> twindow);
    void WriteTimeWindow(I3FramePtr frame);
  };
	
};




I3TriggerSplitter::I3TriggerSplitter(const I3Context& context) : I3Module(context), I3Splitter(configuration_) {
  trigHierName_   = "I3TriggerHierarchy";
#if __cplusplus < 201103L
  configIDs_      = boost::assign::list_of(1006)(1007)(1011)(21001);
#else
  configIDs_      = std::vector<int>{1006,1007,1011,21001};
#endif
  noSplitDt_      = 10000./I3Units::ns;
  tWindowMinus_   =  4000./I3Units::ns;
  tWindowPlus_	=  6000./I3Units::ns;
  writeFrameTimes_= true;
  writeTimeWindow_= false;
  updateTrigHierarchy_ = "UPDATE";
  updateTrigHierarchyMode_ = 1;
  useCDinfoForDt_ = true;
  
  AddOutBox("OutBox");
  AddParameter("InputResponses",
               "List of input Recopulse series names",
               inputResponses_);
  AddParameter("OutputResponses",
               "List of output Recopulse series mask names", 
               outputResponses_);
  AddParameter("TrigHierName",
               "Name of the input TriggerHierarchy "
               "default I3TriggerHierarchy", 
               trigHierName_);
  AddParameter("TriggerConfigIDs",
               "Vector of config IDs for the relevant triggers "
               "default [1006 (SMT8), 1007 (string), 1011 (SMT3), 21001 (volume)",
               configIDs_);
  AddParameter("NoSplitDt",
               "don't split for Dt <= to this (end of previous trigger to the start of next) "
               "default 10000 ns",
               noSplitDt_);
  AddParameter("ReadoutWindowMinus",
               "Length of readout window to pad before trigger [ns] "
               "default 4000 ns",
               tWindowMinus_);
  AddParameter("ReadoutWindowPlus",
               "Length of readout window to pad after end of trigger [ns] "
               "default 6000 ns",
               tWindowPlus_);
  AddParameter("WriteoutFrameTimes",
               "Write the start and end of the readout times in P-frame's I3EventHeader"
               "default true",
               writeFrameTimes_);
  AddParameter("WriteTimeWindow",
               "write into p-frames the time window of trigger times"
               "default false",
               writeTimeWindow_);
  AddParameter("UpdateTriggerHierarchy",
               "'NONE' = don't do anything, 'UPDATE'=Write a new triggerhierachy in the P-frame, 'REPLACE'=do UPDATE and rename the q-frame trigger hierarchy"
               "default UPDATE",
               updateTrigHierarchy_);
  AddParameter("SubEventStreamName",
	       "The name of the SubEvent stream.",
	       configuration_.InstanceName());
}

void I3TriggerSplitter::Configure(){
  GetParameter("InputResponses", inputResponses_);
  GetParameter("OutputResponses", outputResponses_);
  GetParameter("TrigHierName", trigHierName_);
  GetParameter("TriggerConfigIDs", configIDs_);
  GetParameter("NoSplitDt", noSplitDt_);
  GetParameter("ReadoutWindowMinus", tWindowMinus_);
  GetParameter("ReadoutWindowPlus", tWindowPlus_);
  GetParameter("WriteoutFrameTimes", writeFrameTimes_);
  GetParameter("WriteTimeWindow", writeTimeWindow_);
  GetParameter("UpdateTriggerHierarchy", updateTrigHierarchy_);
  GetParameter("SubEventStreamName", sub_event_stream_name_);

  if (noSplitDt_ < 0.) {
    log_fatal("DANGEROUS!!!! NoSplitDT is set to %lf, a negative value. There will be DUPLICATE pulses in different P-frames!!!", noSplitDt_);
  }
  
  if (inputResponses_.size() != outputResponses_.size()) {
    log_fatal("Number of InputResponses and OutputResponses disagree");
  }
  
  for (size_t i = 0; i < inputResponses_.size(); ++i) {
    for (size_t j = 0; j < outputResponses_.size(); ++j) {
      if (inputResponses_[i] == outputResponses_[j]) {
        log_fatal("Cannot give same name to input and output pulse series... one is a mask of the other");
      }
    }
  }
  
  if (updateTrigHierarchy_ == "NONE")    updateTrigHierarchyMode_ = 0;
  else if (updateTrigHierarchy_ == "UPDATE")  updateTrigHierarchyMode_ = 1;
  else if (updateTrigHierarchy_ == "REPLACE") updateTrigHierarchyMode_ = 2;
  else log_fatal("No setting %s exists", updateTrigHierarchy_.c_str());
}

void I3TriggerSplitter::Calibration(I3FramePtr frame){
  calibration_ = frame->Get<I3CalibrationConstPtr>();
  if (!calibration_) {
    useCDinfoForDt_ = false;
    log_warn("DANGEROUS: no C-frame detected. 200ns is used to GUESS the launch time pulse time offset. MAKE SURE THIS IS WHAT YOU WANT!!");
  } else if (calibration_ && status_)
    CalcLaunchDTs();
  PushFrame(frame);
}

void I3TriggerSplitter::DetectorStatus(I3FramePtr frame){
  status_ = frame->Get<I3DetectorStatusConstPtr>();
  if (!status_) {
    useCDinfoForDt_ = false;
    log_warn("DANGEROUS: no D-frame detected. 200ns is used to GUESS the launch time pulse time offset. MAKE SURE THIS IS WHAT YOU WANT!!");
  } else if (calibration_ && status_)
    CalcLaunchDTs();
  PushFrame(frame);
}

void I3TriggerSplitter::CalcLaunchDTs()
{
  offsets_.clear();
  min_offset_ = DBL_MAX;
  max_offset_ = -DBL_MAX;
  BOOST_FOREACH(const I3DOMStatusMap::value_type &dpair, status_->domStatus) {
    I3DOMCalibrationMap::const_iterator c_it =
      calibration_->domCal.find(dpair.first);
    if (c_it != calibration_->domCal.end()) {
      double offset = GetLaunchDT(c_it->second, dpair.second);
      offsets_[dpair.first] = offset;
      if (std::isfinite(offset) && offset < min_offset_)
        min_offset_ = offset;
      if (std::isfinite(offset) && offset > max_offset_)
        max_offset_ = offset;
    }
  }
}

void I3TriggerSplitter::DAQ(I3FramePtr frame){
  log_trace("Entering I3TriggerSplitter::DAQ()");
  
  // Get the trigger times and lengths
  I3TriggerHierarchyConstPtr trigHier = frame->Get<I3TriggerHierarchyConstPtr>(trigHierName_);
  if(!trigHier) log_fatal("No I3TriggerHierarchy found!");
  
  std::vector<Trigs> SplitTrigs;   
  Trigs T;
  I3TriggerHierarchy::iterator trigIter;
  for(trigIter = trigHier->begin() ; trigIter != trigHier->end(); ++trigIter) {
    if(trigIter->GetTriggerKey().CheckConfigID()) {
      log_debug("Got trigger %i", trigIter->GetTriggerKey().GetConfigID());
      
      if( find(configIDs_.begin(), configIDs_.end(), trigIter->GetTriggerKey().GetConfigID()) != configIDs_.end() ) {
        T.time = trigIter->GetTriggerTime();
        T.length = trigIter->GetTriggerLength();
        SplitTrigs.push_back(T);
      }
    } else {
      log_debug("Got trigger %s NA", TriggerKey::GetTypeString(trigIter->GetTriggerKey().GetType()));
    }
  }
  
  //change trigger hierarchy name in Q-frame (so p-frames can see both parent I3TH and it's own)
  if (updateTrigHierarchyMode_==2){
    frame->Delete("I3TriggerHierarchy");
    frame->Put("I3QTriggerHierarchy", trigHier);
  }
  
  // figure out chopping blocks
  log_debug("Got %i triggers designated as splitters", (int)SplitTrigs.size());
  if (SplitTrigs.size() == 0){
    PushFrame(frame);
    return;
  }
  
  std::sort(SplitTrigs.begin(), SplitTrigs.end());
  
  std::vector<SubTrigger> subtriggers;
  double start = SplitTrigs[0].time-tWindowMinus_;
  double end = 0.0;
  for (unsigned int i=0; i<SplitTrigs.size(); i++){
    double endTimeNow = SplitTrigs[i].time + SplitTrigs[i].length;
    if (endTimeNow > end) end = endTimeNow; //current chopping block's trigger end
    if (i == SplitTrigs.size()-1){
      subtriggers.push_back(SubTrigger(start, end+tWindowPlus_, &offsets_,
                                       min_offset_, max_offset_));
    } else {
      double startTimeNext = SplitTrigs[i+1].time;
      if (startTimeNext-end > noSplitDt_) {
        subtriggers.push_back(SubTrigger(start, end+tWindowPlus_, &offsets_,
                                         min_offset_, max_offset_));
        start = startTimeNext-tWindowMinus_;
      }
    }
  }

  // ensured by log_fatal in Configure():
  assert(inputResponses_.size() == outputResponses_.size());
  
  PushFrame(frame); /* Send the DAQ frame downstream */
  
  BOOST_FOREACH(SubTrigger &subtrigger, subtriggers) {
    
    I3FramePtr subframe = GetNextSubEvent(frame);
    
    std::vector<std::string>::const_iterator inputName  = inputResponses_.begin();
    std::vector<std::string>::const_iterator outputName = outputResponses_.begin();
    for ( ; inputName != inputResponses_.end(); inputName++, outputName++)
      subtrigger.MaskPulses(subframe, *inputName, *outputName);
    
    if (writeFrameTimes_)
      subtrigger.UpdateHeader(subframe);
    if (updateTrigHierarchyMode_ > 0)
      subtrigger.UpdateTriggers(subframe, trigHier, configIDs_,
                                std::make_pair(tWindowMinus_, tWindowPlus_));
    if (writeTimeWindow_)	subtrigger.WriteTimeWindow(subframe);
    
    PushFrame(subframe); /* Send the Physics frame downstream */
  }
}

/* *
 * estimate the delay between launches (trigger) times and pulse times 
 * This is an un-doing of what is done in I3WaveCalibrator::GetStartTime() to shift the times
 * In no way is this a general physical estimate of the various delays, and therefore,
 * this calculates a wavecalibrator setting dependent parameter.
 * fadc delay is included to account for the longest DT situation of having a pulse 
 * start in the first FADC bin
 * */
double I3TriggerSplitter::GetLaunchDT(const I3DOMCalibration &calib, const I3DOMStatus &status) {
  const LinearFit &fit = calib.GetTransitTime();
  const double pmtHV = status.pmtHV/I3Units::V;
  const double transit_time = fit.slope/sqrt(pmtHV) + fit.intercept;
  return transit_time - calib.GetFADCDeltaT(); //fadc dt larger than atwd
}

bool
I3TriggerSplitter::SubTrigger::operator()(const OMKey &key, 
                                          size_t idx,
                                          const I3RecoPulse &pulse)
{
  if (key != current_key_) {
    offset_map_t::const_iterator it = offsets_->find(key);
    if (it == offsets_->end())
      current_offset_ = 200.;
    else
      current_offset_ = it->second;
    current_key_ = key;
  }
  return (pulse.GetTime() + current_offset_ + 25.0 >= start_time_)
    && (pulse.GetTime() + current_offset_ < end_time_);
}

void
I3TriggerSplitter::SubTrigger::MaskPulses(I3FramePtr frame,
                                          const std::string &inputName, 
                                          const std::string &outputName)
{
  I3RecoPulseSeriesMapMaskPtr mask
    (new I3RecoPulseSeriesMapMask(*frame, inputName, *this));
  
  frame->Put(outputName, mask);
  frame->Put(outputName + "TimeRange",
             I3TimeWindowPtr(new I3TimeWindow(start_time_ - max_offset_ - 25.0,
                                              end_time_ - min_offset_)));
}

void I3TriggerSplitter::SubTrigger::WriteTimeWindow(I3FramePtr frame){
  I3TimeWindowPtr tw(new I3TimeWindow(start_time_, end_time_));
  frame->Put("TriggerSplitterLaunchWindow", tw);
}

void
I3TriggerSplitter::SubTrigger::UpdateHeader(I3FramePtr frame)
{
  I3EventHeaderConstPtr headerPtr =
    frame->Get<I3EventHeaderConstPtr>("I3EventHeader");
  if (!headerPtr)
    log_fatal("no event header found");
  I3EventHeaderPtr header(new I3EventHeader(*headerPtr));
  
  I3Time startTime = header->GetStartTime();
  I3Time pStart(startTime + start_time_);
  I3Time pEnd(startTime + end_time_);
  
  header->SetStartTime(pStart);
  header->SetEndTime(pEnd);
  
  frame->Delete("I3EventHeader");
  frame->Put("I3EventHeader", header);
}

void
I3TriggerSplitter::SubTrigger::UpdateTriggers(I3FramePtr frame,
                                              I3TriggerHierarchyConstPtr trigHier, 
                                              const std::vector<int> &configIDs,
                                              std::pair<double, double> twindow) {
  I3TriggerHierarchyPtr subtrigs(new I3TriggerHierarchy()); // new trig hierarchy
  
  // first make and insert global merged trigger, it'll
  // ALWAYS MAKE THIS EVEN IF THERE AREN'T TRIGS TO MERGE
  TriggerKey globKey(TriggerKey::GLOBAL, TriggerKey::MERGED);
  I3Trigger globTrigger;
  globTrigger.GetTriggerKey() = globKey;
  globTrigger.SetTriggerFired(true);
  globTrigger.SetTriggerTime(start_time_);
  globTrigger.SetTriggerLength(end_time_-start_time_);
  
  I3TriggerHierarchy::iterator subIter = subtrigs->end();
  subIter = subtrigs->insert(subIter,globTrigger);
  
  // now iterate through q-frame trigger and pick-up/insert 
  // relevant trigs and create throughput
  for(I3TriggerHierarchy::iterator trigIter = trigHier->begin() ; trigIter != trigHier->end(); ++trigIter) {
    if( find(configIDs.begin(), configIDs.end(),trigIter->GetTriggerKey().GetConfigIDOptional()) != configIDs.end() ) {
      if ((trigIter->GetTriggerTime() >= start_time_) && (trigIter->GetTriggerTime() < end_time_)){
        //make and insert throughput
        TriggerKey tpKey(TriggerKey::GLOBAL, TriggerKey::THROUGHPUT);
        I3Trigger tpTrigger;
        tpTrigger.GetTriggerKey() = tpKey;
        tpTrigger.SetTriggerFired(true);
        tpTrigger.SetTriggerTime(trigIter->GetTriggerTime()-twindow.first);
        tpTrigger.SetTriggerLength(trigIter->GetTriggerLength()+twindow.first+twindow.second);
        I3TriggerHierarchy::iterator eachTrig;
        eachTrig = subtrigs->append_child(subIter, tpTrigger);
        //insert actual trigger
        subtrigs->append_child(eachTrig, trigIter);
      }
    }
  }
  I3TriggerHierarchyConstPtr subTrigHier = frame->Get<I3TriggerHierarchyConstPtr>("I3TriggerHierarchy");
  if(subTrigHier) frame->Delete("I3TriggerHierarchy");
  frame->Put("I3TriggerHierarchy", subtrigs);
}

I3_MODULE(I3TriggerSplitter);

