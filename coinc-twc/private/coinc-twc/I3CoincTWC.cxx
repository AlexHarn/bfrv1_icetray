/**
    copyright  (C) 2010
    the IceCube Collaboration
    $Id: I3CoincTWC  $

    @version $Revision:  2.0 $
    @author Tom Feusels (tom.feusels@ugent.be)
    based on StaticTWC
*/

#include <icetray/I3ConditionalModule.h>
#include <icetray/I3Context.h>
#include <icetray/I3Frame.h>
#include <icetray/I3Logging.h>

#include <dataclasses/I3Map.h>
#include <dataclasses/I3MapOMKeyMask.h>
#include <dataclasses/TriggerKey.h>
#include <dataclasses/physics/I3Trigger.h>
#include <dataclasses/physics/I3TriggerHierarchy.h>
#include <dataclasses/physics/I3DOMLaunch.h>
#include <dataclasses/physics/I3RecoPulse.h>
#include <dataclasses/status/I3DetectorStatus.h>
#include <trigger-sim/utilities/DetectorStatusUtils.h>

#include <algorithm>
#include <list>
#include <string>
#include <cfloat>
#include <sstream>

using DetectorStatusUtils::_sourceID;
using DetectorStatusUtils::_typeID;
using DetectorStatusUtils::GetBestMatches;

bool TimeOrdered(const I3Trigger& lhs, const I3Trigger& rhs){
  return lhs.GetTriggerTime() < rhs.GetTriggerTime();
}


template <class Response>
class I3CoincTWC : public I3ConditionalModule
{
  public:
  
  I3CoincTWC(const I3Context& context) : I3ConditionalModule(context),
					  inputResponse_(""),
					  outputResponse_(""),
					  triggerName_("I3TriggerHierarchy"),
					  tWindowMin_(3500.),
					  tWindowMax_(9000.),
					  epsilon_min_(300.),
					  epsilon_max_(400.),
					 clean_max_(9000.),
					  keepMultipleCausalTriggers_(true),
					 keepLooking_(false),
					 itPulsesName_(""),
					 singleSMT_(true),
					 useSMT3_(false),
					 useSMT8_(true),
					 numberOfWarnings_(0),
					 numberOfITs_(0),
					 numberOfIIs_(0),
					 maxTimeDiff_(200),
					 method_("method1")
  {
    AddOutBox("OutBox");
    AddParameter("InputResponse","Name of the input response map", inputResponse_);
    AddParameter("OutputResponse","Name for the output response map", outputResponse_);
    AddParameter("TriggerHierarchyName","Name of the input TriggerHierarchy", triggerName_);
    AddParameter("WindowMin","Time difference between IceTop and InIce SMT must be larger than WindowMin", tWindowMin_);
    AddParameter("WindowMax","Time difference between IceTop and InIce SMT must be smaller than WindowMax", tWindowMax_);
    AddParameter("cleanWindowMinus","Clean all hits before first causal InIce SMT trigger Time minus cleanWindowMinus, For DOMLaunches : very small or zero, for RecoPulses : ~150", epsilon_min_);
    AddParameter("cleanWindowPlus","Clean all hits after last causal InIce SMT trigger Time plus cleanWindowPlus, For DOMLaunches : very small or zero, for RecoPulses : ~270 (-150 offset + 420 ATWD)", epsilon_max_);
    AddParameter("cleanWindowMaxLength","Restrict the cleaning window to this length, if 0 WindowLength only restricted by cleanWindowPlus", clean_max_);
    AddParameter("KeepMultipleCausalTriggers","Keep only the first causal InIce SMT triggers or keep all that fit in the window", keepMultipleCausalTriggers_);
    AddParameter("KeepLookingForCausalTrigger","Keep looking for at least one best causal trigger by extending the search window",keepLooking_);
    AddParameter("IceTopVEMPulsesName","Name of IceTopVEMPulses in the frame if there is one (empty string if none). Will be used for making a choice when multiple IT SMTs. NEW : And matching P frames with this Mask and put the new cleaned II pulses Mask in the SAME frame!",itPulsesName_);
    AddParameter("CheckSingleSMTs","Instead of only looking at multiple InIceSMT also check whether the one InIceSMT is really causally connected",singleSMT_);
    AddParameter("UseSMT8","Look for causal IceTopSMT-InIceSMT8 (global InIce SMT) triggers",useSMT8_);
    AddParameter("UseSMT3","Look for causal IceTopSMT-InIceSMT3 (DeepCore SMT) triggers",useSMT3_);
    AddParameter("MaxTimeDiff","Maximum time difference between IceTop trigger time and time of first HLC pulse to be related",maxTimeDiff_);
    AddParameter("Strategy","Search strategy for causal II triggers. \"method1\" : look in static window. \"method2\" : add a region with timediff dependent triggerlength before windowMin (takes into account that an earlier unrelated event in IceCube might trigger the same InIceSMT EARLIER than expected",method_);
    AddParameter("Stream","Frame type on which to run",I3Frame::Physics);
  }
  
  void Configure()
  {
    GetParameter("InputResponse", inputResponse_);
    GetParameter("OutputResponse", outputResponse_);
    GetParameter("TriggerHierarchyName", triggerName_);
    GetParameter("WindowMin", tWindowMin_);
    GetParameter("WindowMax", tWindowMax_);
    GetParameter("cleanWindowMinus", epsilon_min_);
    GetParameter("cleanWindowPlus", epsilon_max_);
    GetParameter("cleanWindowMaxLength",clean_max_);
    GetParameter("KeepMultipleCausalTriggers", keepMultipleCausalTriggers_);
    GetParameter("KeepLookingForCausalTrigger",keepLooking_);
    GetParameter("IceTopVEMPulsesName",itPulsesName_);
    GetParameter("CheckSingleSMTs",singleSMT_);
    GetParameter("UseSMT8",useSMT8_);
    GetParameter("UseSMT3",useSMT3_);
    GetParameter("MaxTimeDiff",maxTimeDiff_);
    GetParameter("Strategy",method_);
    I3Frame::Stream stream;
    GetParameter("Stream",stream);
    Register(stream, &I3CoincTWC::HitCleaning);

    if( !(useSMT3_ || useSMT8_)){
      log_fatal("Choose at least one of the available SMTs, both are false now");
    }
    
    if( (method_ != "method1") && (method_ != "method2") ){
      log_warn("Unknown method %s! Will switch to default method1",method_.c_str());
      method_ = "method1";
    }
  }
  
  //Get the ConfigIDs for SMT3 and SMT8 from the GCD and do it only once
  void DetectorStatus(I3FramePtr frame)
  {
    I3DetectorStatusConstPtr status = frame->Get<I3DetectorStatusConstPtr>(); 
    if(!status) {
      log_fatal("No I3DetectorStatus found!");
    }
    std::vector<TriggerKey> matches = 
      GetBestMatches(status,
		     _sourceID = TriggerKey::IN_ICE,
		     _typeID = TriggerKey::SIMPLE_MULTIPLICITY);

    BOOST_FOREACH(const TriggerKey& tk, matches){
      // this should never happen, but is just a safeguard for the
      // call that follows it.
      if(status->triggerStatus.find(tk) == status->triggerStatus.end())
	log_fatal("GetBestMatches returned a TriggerKey that was not "
		  "found in the I3TriggerStatusMap.  \nThis should never happen.");
      I3TriggerStatus trigStatus(status->triggerStatus.find(tk)->second);
      boost::optional<int> threshold;
      trigStatus.GetTriggerConfigValue("threshold",threshold);

      // dump the contents in case of an error
      // a little wasteful if there's no error, but very cheap.
      std::stringstream sstr;
      sstr<<"TriggerKey = "<<tk<<std::endl;
      typedef std::map<std::string,std::string> ts_map_t;
      BOOST_FOREACH(const ts_map_t::value_type& value, trigStatus.GetTriggerSettings()){
	sstr<<"   "<<value.first<<" : "<<value.second<<std::endl;
      }

      if( threshold ) {
	if(threshold.get() == 8) SMT8_ = tk.GetConfigID();	  
	if(threshold.get() == 3) SMT3_ = tk.GetConfigID();
	if(threshold.get() != 8 && threshold.get() != 3){	
	  log_error("Can't determine whether this is SMT8 or SMT3.\n"		    
		    "Dumping the contents of the I3TriggerStatus:\n");
	  log_fatal("%s",sstr.str().c_str());
	}
      }else{
	log_error("No Setting called \"threshold\" found in the TriggerStatus.\n"
		  "Dumping the contents of the I3TriggerStatus:\n");
	log_fatal("%s",sstr.str().c_str());
      }
    }
    log_info("The configID for SMT8 for this GCD file is %i and for SMT3 it's %i",
	     SMT8_,SMT3_);

    PushFrame(frame);
  }

  void HitCleaning(I3FramePtr frame)
  {
    
    if(!itPulsesName_.empty()){
      // Then put the mask of the cleaned pulses made here in the corresponding frame (so they're together)
      if(!frame->Has(itPulsesName_) ){
	PushFrame(frame);
	return;
      }
    } else  {
      log_warn("No IceTopPulseName given : output will be in same frame as input, and triggers purely based on TriggerHierarchy !");
      ++numberOfWarnings_;
      if (!frame->Has(inputResponse_)) {
	PushFrame(frame); 
	return;                          
      }    
    }

    // Get the trigger hierarchy
    I3TriggerHierarchyConstPtr triggers = frame->Get<I3TriggerHierarchyConstPtr>(triggerName_);
    if(!triggers) {
      log_warn("No I3TriggerHierarchy found!");
      ++numberOfWarnings_;
      PushFrame(frame,"OutBox");
      return;
    }
    
    std::list<I3Trigger> ii_SMT;  //can be either SMT8 or SMT3, or only SMT8 or only SMT3, user decides!
    std::list<I3Trigger> it_SMT;

    // Put SMT triggers in a list
    // TODO : For lowest energy coincident, maybe String Trigger InIce too? 
    I3TriggerHierarchy::iterator triggerIter;
    for(triggerIter = triggers->begin() ; triggerIter != triggers->end(); ++triggerIter) {
      if( (triggerIter->GetTriggerKey()).GetType() == TriggerKey::SIMPLE_MULTIPLICITY) { 
	if( (triggerIter->GetTriggerKey()).GetSource() == TriggerKey::IN_ICE){
	  if( ( (triggerIter->GetTriggerKey()).GetConfigID() == SMT8_ ) && (useSMT8_) ){
	    log_trace("InIce SMT8 found");
	    ii_SMT.push_back(*triggerIter);
	  } 
	  // And/or optionally look at the DeepCore SMT3 too
	  if( ( (triggerIter->GetTriggerKey()).GetConfigID() == SMT3_ ) && (useSMT3_) ){
	    log_trace("InIce SMT3 found");
	    ii_SMT.push_back(*triggerIter);
	  }
	} else if( (triggerIter->GetTriggerKey()).GetSource() == TriggerKey::ICE_TOP){
	  log_trace("IceTop SMT found");
	  it_SMT.push_back(*triggerIter);
	}
      } 
    }
    // Sort InIce SMT && IT SMT by TriggerTime 
    ii_SMT.sort(TimeOrdered);
    it_SMT.sort(TimeOrdered);

    //Let's deal with multiple IT SMTs, topeventbuilder already made the choice, let's take the trigger that belongs to that choice
    if(it_SMT.size() > 1){ // Which is rare but happens
      log_debug("Multiple (%i) IT SMTs", (int)it_SMT.size());
      if(ii_SMT.size() > 1)
	++numberOfITs_;
      if(frame->Has(itPulsesName_)){
	// Deals both with Mask and real Maps ! (The frame->Get does the job internally in Utility.h)
	I3RecoPulseSeriesMap vemPulses = frame->Get<I3RecoPulseSeriesMap>(itPulsesName_);
	
	// If no IceTop SMT found within some max time difference from the first IT pulse, just push frame
	if( ! SelectIceTopTrigger(it_SMT,vemPulses) ){
	  PushFrame(frame,"OutBox");
	  return;
	}
      }
      else 
	log_warn("We are dealing with multiple IceTop SMT Triggers and no IceTopVEMPulses to decide! If you wanted to keep Multiple Triggers, then the timeWindow for cleaning will be extended !!");
    }

    // If IT SMT found a match, not if IT SMT found two matches !
    unsigned int nrCausalTriggers = 0; 
    
    double t_min = 0.;
    double t_max = 0.;
    double causalWindow_t_min = tWindowMin_;
    double causalWindow_t_max = tWindowMax_;
    short unsigned int condition = 1;
    if(singleSMT_)
      condition = 0;
    if(it_SMT.size() > 0 && ii_SMT.size() > condition){
      log_debug("%i InIce Simple Multiplicity Triggers found",(int)ii_SMT.size());
      ++numberOfIIs_;
          
      //Clean Pulses
      for(std::list<I3Trigger>::const_iterator it_trig_iter= it_SMT.begin(); it_trig_iter != it_SMT.end(); ++it_trig_iter){
	log_trace("IT : Trigger Time : %lf, TriggerLength : %lf",it_trig_iter->GetTriggerTime(),it_trig_iter->GetTriggerLength());
	while( nrCausalTriggers < it_SMT.size() ){
	  unsigned short int causalPairFound = 0; //IT_SMT - II_SMT pairs (different InIce can match with same IT)
	  for(std::list<I3Trigger>::const_iterator ii_trig_iter= ii_SMT.begin(); ii_trig_iter != ii_SMT.end(); ++ii_trig_iter){
	    log_trace("InIce : Trigger Time : %lf, TriggerLength : %lf",ii_trig_iter->GetTriggerTime(),ii_trig_iter->GetTriggerLength());
	    
	    double timediff = ii_trig_iter->GetTriggerTime() - it_trig_iter->GetTriggerTime();   
	    log_debug("The time difference between the InIce SMT and the IT SMT is %.1lf",timediff);

	    if(( (timediff >= causalWindow_t_min) && (timediff < causalWindow_t_max) ) ||
	       ( (method_ == "method2") && (ii_trig_iter->GetTriggerLength() > (- timediff + causalWindow_t_min+300.) ) 
		 &&  (timediff >= 0.) && (timediff < causalWindow_t_max) )
	       ){ 

	      // Did we find a special one (method2 : before the basic window) or normal one?
	      if( timediff < causalWindow_t_min ) {
		if(method_ != "method2"){
		  log_warn("The causal trigger search found an illegal match, this kind of events are only for method2");
		  continue;
		} else {
		  // In this case the event duration could be longer than clean_max_
		  // Like two events with the same II SMT, gives an extended II SMT
		  clean_max_ += (causalWindow_t_min - timediff);
		}
	      }
	      
	      ++causalPairFound;
	      // Count nr of pair with a unique IT SMT
	      if(causalPairFound == 1)
		++nrCausalTriggers;
	      
	      //If this is the first causal InIce SMT we've found, should be closest to tWindowMin_ ns, should be best one
	      if((nrCausalTriggers == 1) && (causalPairFound == 1)){
		
		t_min = ii_trig_iter->GetTriggerTime() - epsilon_min_; 
		t_max = ii_trig_iter->GetTriggerTime() + ii_trig_iter->GetTriggerLength() + epsilon_max_;
		log_debug("Setting the cleaning time window to [%lf,%lf]",t_min,t_max);
		
	      } else {
		if(keepMultipleCausalTriggers_) { 
		  double new_t_max = ii_trig_iter->GetTriggerTime() + ii_trig_iter->GetTriggerLength() + epsilon_max_; 
		  if(new_t_max > t_max){
		    t_max = new_t_max;
		    log_debug("Extending the cleaning time window to [%lf,%lf]",t_min,t_max);
		  }
		}
		else
		  break;
	      }
	    }// end timediff check
	  }//end for-loop ii triggers
	  
	  //What if none were found for this IT SMT ?
	  // -> Extend window by 0.5 us both sides and start over
	  if(keepLooking_ && (nrCausalTriggers == 0)){  // Keep looking until there is at least ONE for all IT SMTs, NOT for each IT SMT, in this case forced for the first IT SMT
	    
	    causalWindow_t_min -= 500.;
	    causalWindow_t_max += 500.;
	    log_debug("No Causal InIce SMT found in this window, extending it to [%lf,%lf]",causalWindow_t_min,causalWindow_t_max);
	  } else
	    break; // if we don't want to keep looking, or we've found one
	}//end while
		
      }
    } 
    
    // Restrict maximum cleaning, to avoid triggerLength which enclose two IT_SMT triggers.
    if( clean_max_ > 0 ){
      if( (t_max - t_min) > clean_max_)
	t_max = t_min + clean_max_;
    }
    
    // input response map
    const I3Map<OMKey,Response>& response = frame->template Get<I3Map<OMKey,Response> >(inputResponse_);
    
    if(ii_SMT.size() == 1 && !singleSMT_){
      boost::shared_ptr<I3Map<OMKey,Response > >  output(new I3Map<OMKey,Response > (response));

      AddToFrame(frame,output);
      PushFrame(frame,"OutBox");
    } 
    
    // output response map
    boost::shared_ptr<I3Map<OMKey,Response > >  output(new I3Map<OMKey,Response > ());
  
    if(nrCausalTriggers){
      
      for(typename I3Map<OMKey,Response>::const_iterator iter = response.begin(); iter != response.end();  ++iter) {
	Response newResponse; // a new response (launch or pulse series) to store kept launches/pulses
        
        for(typename Response::const_iterator thisHit = iter->second.begin(); thisHit != iter->second.end();  ++thisHit) {
          const double hitTime = GetTime(*thisHit);
          
          if( (hitTime > t_min) && (hitTime < t_max) ) {
            newResponse.push_back(*thisHit);
            log_trace("Kept DOM launch/pulse (%d,%d) t=%f ", iter->first.GetString(), iter->first.GetOM(), hitTime);
          } else {
            log_trace("Cleaned DOM launch/pulse (%d,%d) t=%f ", iter->first.GetString(), iter->first.GetOM(), hitTime);
          }
        }
        
        if( !newResponse.empty()) (*output)[iter->first] = newResponse;
      }
      AddToFrame(frame,output); // ELSE : PUT uncleaned INPUT back in FRAME
    }
    PushFrame(frame,"OutBox");
    
    
  }
    
  //See template below for details
  void AddToFrame(I3FramePtr frame, boost::shared_ptr<I3Map<OMKey,Response > > output) 
  { 
    frame->Put(outputResponse_, output); 
  }
  
  void Finish()
  {
    if( numberOfWarnings_ > 0 ) {
      log_warn("In total, %d events had none of the requested triggers!", numberOfWarnings_);  // this is not correct
    }
    if( numberOfITs_ > 0 )
      log_info("In total, %d events had multiple IceTop SMT triggers with coincident InIce SMT!", numberOfITs_);
    if( numberOfIIs_ > 0 )
      log_info("In total, %d events had multiple InIce SMT triggers!", numberOfIIs_);
  }
  
private:
  
  /* 
   * This function selects the IceTop SMT which has already been selected by TopEventBuilder
   */
  bool SelectIceTopTrigger(std::list<I3Trigger> &itSMTs,I3RecoPulseSeriesMap &TopPulses){
    // Loop over map and take earliest time
    double pulseTime = DBL_MAX;
    for(I3RecoPulseSeriesMap::const_iterator iter = TopPulses.begin(); iter != TopPulses.end(); ++iter){
      I3RecoPulseSeries pulseSeries = iter->second;
      for(I3RecoPulseSeries::const_iterator pulseIter = pulseSeries.begin(); pulseIter != pulseSeries.end(); ++pulseIter){
	double time = pulseIter->GetTime();
	if(time < pulseTime)
	  pulseTime = time;
      }
    }
    
    I3TriggerPtr outputTrigger; 
    // Choose trigger with smallest timedifference wrt pulseTime.
    double previous_timediff = DBL_MAX;
    for(std::list<I3Trigger>::const_iterator trig_it = itSMTs.begin(); trig_it != itSMTs.end(); ++trig_it){
      double timediff = std::abs(pulseTime - trig_it->GetTriggerTime());
      log_trace("Time difference between earliest pulseTime and triggertime is %lf",timediff);
      if(timediff < previous_timediff){
	previous_timediff = timediff;
	outputTrigger = I3TriggerPtr(new I3Trigger(*trig_it));
      }
    }
    log_debug("The time difference between the found IceTop trigger and the pulses is %lf",previous_timediff);
    if(!outputTrigger) {
      log_fatal("The function didn't find an IceTop Trigger from the VEMPulses although there were %i IceTop SMT triggers",(int)itSMTs.size());
    }
    else {
      if(previous_timediff > maxTimeDiff_) {
	log_warn("The IceTop SMT which is closest to these IceTopPulses still triggered too late or early to be actually related to these pulses : timediff is %lf",previous_timediff);
	numberOfWarnings_++;
	return false;
      } 
      else {
	// Clear list and push back found IceTop trigger
	itSMTs.clear();
	itSMTs.push_back(*outputTrigger);
	return true;
      }
    }
    
  }

  std::string inputResponse_;
  std::string outputResponse_;
  std::string triggerName_;
  double tWindowMin_;
  double tWindowMax_;  
  double epsilon_min_;
  double epsilon_max_;
  double clean_max_;
  bool keepMultipleCausalTriggers_;
  bool keepLooking_;
  std::string itPulsesName_;
  bool singleSMT_;
  bool useSMT3_;
  bool useSMT8_;
  //TriggerConfigIDs
  int SMT3_;
  int SMT8_;

  /// counts warnings for events without relevant triggers; only 20 warnings are displayed
  int numberOfWarnings_;

  // counters to see how much multiple IT SMTs (with coincident trigger) or multiple II SMTs we have
  int numberOfITs_;
  int numberOfIIs_;

  double maxTimeDiff_;  // maximum difference between time of IT pulses and IT SMT trigger time to be still related 

  std::string method_; // method1 : look for causal trigger in window between [tWindowMin_, tWindowMax_]
                       // method2 : 'open the door': add the search in a region above a line in the triggertime-triggerlength -plane (see docs)
  SET_LOGGER("I3CoincTWC");
};

//Helper class for the GetTime method
template <class Response> double GetTime(const Response&);
template <> double GetTime<I3RecoPulse>(const I3RecoPulse &pulse) {
  return pulse.GetTime();
}
template <> double GetTime<I3DOMLaunch>(const I3DOMLaunch &launch) {
  return launch.GetStartTime();
}

template <>
void I3CoincTWC<I3RecoPulseSeries>::AddToFrame(I3FramePtr frame, I3RecoPulseSeriesMapPtr output)
{
      // add a mask and put into the frame
      I3RecoPulseSeriesMapMaskPtr mask(new I3RecoPulseSeriesMapMask(*frame,inputResponse_, *output));
      frame->Put(outputResponse_,mask); 
}



I3_MODULE(I3CoincTWC<I3RecoPulseSeries>);
I3_MODULE(I3CoincTWC<I3DOMLaunchSeries>);

