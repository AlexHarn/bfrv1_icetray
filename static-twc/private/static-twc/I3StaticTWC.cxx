/**
    copyright  (C) 2010
    the IceCube Collaboration
    $Id: I3StaticTWC  $

    @version $Revision:  93.0 $
    @author Chang Hyon Ha (cuh136 @ phys . psu . edu)
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
#include <dataclasses/I3TimeWindow.h>

#include <algorithm>
#include <vector>
#include <string>
#include <boost/make_shared.hpp>

template <class Response>
class I3StaticTWC : public I3ConditionalModule
{
  public:
    
    I3StaticTWC(const I3Context& context) : I3ConditionalModule(context),
                                            inputResponse_(""),
                                            outputResponse_(""),
                                            triggerName_(""),
                                            configIDs_(std::vector<int>(1,1010)),
                                            tWindowMinus_(3500),
                                            tWindowPlus_(4000),
                                            firstTriggerOnly_(false),
                                            numberOfWarnings_(0)
    {
      AddOutBox("OutBox");
      AddParameter("InputResponse","Name of the input response map", inputResponse_);
      AddParameter("OutputResponse","Name for the output response map", outputResponse_);
      AddParameter("TriggerName","Name of the input TriggerHierarchy", triggerName_);
      AddParameter("TriggerConfigIDs","Vector of config IDs for the relevant triggers", configIDs_);
      AddParameter("WindowMinus","Time window backward from trigger (positive value!)", tWindowMinus_);
      AddParameter("WindowPlus","Time window forward from trigger", tWindowPlus_);
      AddParameter("FirstTriggerOnly","Use only the time of the first trigger", firstTriggerOnly_);
    }
    
    void Configure()
    {
      GetParameter("InputResponse", inputResponse_);
      GetParameter("OutputResponse", outputResponse_);
      GetParameter("TriggerName", triggerName_);
      GetParameter("TriggerConfigIDs", configIDs_);
      GetParameter("WindowMinus", tWindowMinus_);
      GetParameter("WindowPlus", tWindowPlus_);
      GetParameter("FirstTriggerOnly", firstTriggerOnly_);
    }
    
    void Physics(I3FramePtr frame)
    {
      log_debug("Start STWC in a new event...");

      if (!frame->Has(inputResponse_)) {
         PushFrame(frame);
         return;
      }

      
      // Get the trigger hierarchy
      I3TriggerHierarchyConstPtr trigger = frame->Get<I3TriggerHierarchyConstPtr>(triggerName_);
      if(!trigger) {
        log_debug("No I3TriggerHierarchy found!");
	PushFrame(frame);
	return;
      }
      
      std::vector<double> triggerTimes;   // save the trigger times
      I3TriggerHierarchy::iterator triggerIter;
      for(triggerIter = trigger->begin() ; triggerIter != trigger->end(); ++triggerIter) {
        if(triggerIter->GetTriggerKey().CheckConfigID()) {   // Check if the config ID is present
          
          log_debug("Got trigger %s %s %i",
                   TriggerKey::GetSourceString(triggerIter->GetTriggerKey().GetSource()),
                   TriggerKey::GetTypeString(triggerIter->GetTriggerKey().GetType()),
                   triggerIter->GetTriggerKey().GetConfigID()
                   );
          
          if( find(configIDs_.begin(), configIDs_.end(), triggerIter->GetTriggerKey().GetConfigID()) != configIDs_.end() ) {
            
            triggerTimes.push_back(triggerIter->GetTriggerTime());   // save the multiple trigger times
            
            log_debug("Got a trigger %s %s %i trigger time t=%f",
                     TriggerKey::GetSourceString(triggerIter->GetTriggerKey().GetSource()),
                     TriggerKey::GetTypeString(triggerIter->GetTriggerKey().GetType()),
                     triggerIter->GetTriggerKey().GetConfigID(),
                     triggerIter->GetTriggerTime()
                     );
          }
        } else {
          log_debug("Got trigger %s %s NA",
                   TriggerKey::GetSourceString(triggerIter->GetTriggerKey().GetSource()),
                   TriggerKey::GetTypeString(triggerIter->GetTriggerKey().GetType())
                   );
        }
      }

      // input response map
      const I3Map<OMKey,Response>& response = frame->template Get<I3Map<OMKey,Response> >(inputResponse_);
      // output response map
      boost::shared_ptr<I3Map<OMKey,Response > >  output(new I3Map<OMKey,Response > ());
      
      if( triggerTimes.empty() ) {   // check to see if there is at least one requested trigger
        ++numberOfWarnings_;
        if( numberOfWarnings_ < 20 ) {
          log_warn("None of the requested triggers was found!");
        } else if ( numberOfWarnings_ == 20 ) {
          log_warn("None of the requested triggers was found! This is the last (20th) warning of this kind.");
        }
        AddToFrame(frame, output);
        PushFrame(frame,"OutBox");
        return;
      }
      
      log_trace("%d Triggers Found!",int(triggerTimes.size()));
      
      double triggerTimeFirst = NAN;   // trigger time for the earliest trigger
      double triggerTimeLast  = NAN;   // trigger time for the latest trigger
      
      triggerTimeFirst = *(std::min_element (triggerTimes.begin(), triggerTimes.end()));
      if(firstTriggerOnly_) {
        triggerTimeLast = triggerTimeFirst;
        log_debug("%d trigger(s) found, using only the first one! t = %f", int(triggerTimes.size()), triggerTimeFirst);
      } else {
        triggerTimeLast = *(std::max_element (triggerTimes.begin(), triggerTimes.end()));
        log_debug("%d trigger(s) found! t_min = %f, t_max = %f (Delta = %f)",
                  int(triggerTimes.size()), triggerTimeFirst, triggerTimeLast, triggerTimeFirst-triggerTimeLast);
      }
      
      // reuse the triggerTimeX variables to be window start and end times
      const double windowStartTime = triggerTimeFirst - tWindowMinus_;
      const double windowEndTime = triggerTimeLast + tWindowPlus_;
      
      for(typename I3Map<OMKey,Response>::const_iterator iter = response.begin(); iter != response.end();  ++iter) {
        
        Response newResponse; // a new response (launch or pulse series) to store kept launches/pulses
        
        for(typename Response::const_iterator thisHit = iter->second.begin(); thisHit != iter->second.end();  ++thisHit) {
          const double hitTime = GetTime(*thisHit);
          
          if( (hitTime > windowStartTime) && (hitTime < windowEndTime) ) {
            newResponse.push_back(*thisHit);
            log_trace("Kept DOM launch/pulse (%d,%d) t=%f ", iter->first.GetString(), iter->first.GetOM(), hitTime);
          } else {
            log_trace("Cleaned DOM launch/pulse (%d,%d) t=%f ", iter->first.GetString(), iter->first.GetOM(), hitTime);
          }
        }
        
        if( !newResponse.empty()) (*output)[iter->first] = newResponse;
      }
      frame->Put(outputResponse_+"TimeRange", boost::make_shared<I3TimeWindow>(windowStartTime, windowEndTime));
      AddToFrame(frame, output); 
      PushFrame(frame,"OutBox");
    }
    void AddToFrame(I3FramePtr frame, boost::shared_ptr<I3Map<OMKey,Response > >
      output) { frame->Put(outputResponse_, output); }

    void Finish()
    {
      if( numberOfWarnings_ > 0 ) {
        log_warn("In total, %d events had none of the requested triggers!", numberOfWarnings_);
      }
    }
    
 private:
    
    std::string inputResponse_;
    std::string outputResponse_;
    std::string triggerName_;
    std::vector<int> configIDs_;
    double tWindowMinus_;
    double tWindowPlus_;
    bool firstTriggerOnly_;
    /// counts warnings for events without relevant triggers; only 20 warnings are displayed
    int numberOfWarnings_;
};

// Helper class for the GetTime method
template <class Response> double GetTime(const Response&);
template <> double GetTime<I3RecoPulse>(const I3RecoPulse &pulse) {
  return pulse.GetTime();
}
template <> double GetTime<I3DOMLaunch>(const I3DOMLaunch &launch) {
  return launch.GetStartTime();
}

template <>
void I3StaticTWC<I3RecoPulseSeries>::AddToFrame(I3FramePtr frame, I3RecoPulseSeriesMapPtr output)
{
      I3RecoPulseSeriesMapMaskPtr mask(new I3RecoPulseSeriesMapMask(*frame,
        inputResponse_, *output));
      frame->Put(outputResponse_,mask);
}

I3_MODULE(I3StaticTWC<I3RecoPulseSeries>);
I3_MODULE(I3StaticTWC<I3DOMLaunchSeries>);

