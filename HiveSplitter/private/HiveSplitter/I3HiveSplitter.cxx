/**
 * \file HiveSplitter.cxx
 *
 * (c) 2012 the IceCube Collaboration
 *
 * $Id: HiveSplitter.cxx 110768 2013-09-16 17:27:13Z mzoll $
 * \version $Revision: 110768 $
 * \date $Date: 2013-09-16 19:27:13 +0200 (Mon, 16 Sep 2013) $
 * \author Marcel Zoll <marcel.zoll@fysik.su.se>
 */

#include <HiveSplitter/I3HiveSplitter.h>

#include "icetray/I3Units.h"
#include "icetray/I3Int.h"
#include "dataclasses/I3Double.h"
#include "dataclasses/I3Constants.h"
#include "dataclasses/I3MapOMKeyMask.h"
#include "dataclasses/physics/I3EventHeader.h"
#include "dataclasses/I3TimeWindow.h"

//===============class I3HiveSplitter=================================
I3HiveSplitter::I3HiveSplitter(const I3Context& context):
  I3ConditionalModule(context),
  I3Splitter(configuration_),
  inputName_(""),
  outputName_(""),
  saveSplitCount_(false),
  noSplitOpt_(false),
  modifyObjectsOpt_(false),
  param_set_(),
  splitter_configured_(false)
  //hiveSplitter_()
{
  log_debug("Creating HiveSplitter instance");
  AddParameter("InputName", "Name of the input pulses in the Q-frame", inputName_);
  AddParameter("OutputName", "OutputName of the processed pulses", outputName_);
  AddParameter("SaveSplitCount", "Whether to save an integer in the frame indicating the number of "
		"subevents generated", saveSplitCount_);
  AddParameter("NoSplitMode", "Do not issue splits than rater a cleaned PulseSeries with the same clusters", noSplitOpt_);
  AddParameter("Multiplicity", "Required multiplicity in causal connected hits to form a subevent", param_set_.multiplicity_);
  AddParameter("TimeWindow", "Time span within which the multiplicity requirement must be met [in nsec]", param_set_.timeWindow_);
  AddParameter("TimeConeMinus", "Maximum negative deviation from speed of light travel time which will "
		"allow a pair of hits to be considered connected [in nsec]", param_set_.timeConeMinus_);
  AddParameter("TimeConePlus", "Maximum positive deviation from speed of light travel time which will "
		"allow a pair of hits to be considered connected [in nsec]", param_set_.timeConePlus_);
  //AddParameter("UseDOMspacings", "The RingLimits are parsed as inter-DOM distances instead of meters", domSpacingsOpt_);
  AddParameter("SingleDenseRingLimits", "Limits for IceCube on-ring distances in pairs Minus/Plus [in meter]", param_set_.SingleDenseRingLimits_);
  AddParameter("DoubleDenseRingLimits", "Limits for DeepCore on-ring distances in pairs Minus/Plus [in meter]", param_set_.DoubleDenseRingLimits_);
  AddParameter("TrippleDenseRingLimits", "Limits for Pingu on-ring distances in pairs Minus/Plus [in meter]", param_set_.TrippleDenseRingLimits_);
  AddParameter("Mode", "The Mode to use: 1=Lorentz [Default], 2=Euclidean, 3=Static", param_set_.modeOpt_);
  //AddParameter("ConnectTroughDust", "2 DOMs above and below the Dustlayer will always be eligible", connectTroughDustOpt_);
  AddParameter("ModifyObjects", "Modify Objects in the Subframes so that they exhibit the timing of the Subeevent (True)", modifyObjectsOpt_);
  AddParameter("SubEventStreamName",
	       "The name of the SubEvent stream.",
	       configuration_.InstanceName());
  AddOutBox("OutBox");
  
  log_info("This is HiveSplitter! While still functional, this project will be phased out. Please consider switching to IceHive (or IceHiveZ)!");
  log_debug("Leaving Init()");
}

void I3HiveSplitter::Configure() {
  log_debug("Entering Configure()");
  GetParameter("InputName", inputName_);
  GetParameter("OutputName", outputName_);
  GetParameter("SaveSplitCount", saveSplitCount_);
  GetParameter("NoSplitMode", noSplitOpt_);
  GetParameter("Multiplicity", param_set_.multiplicity_);
  GetParameter("TimeConeMinus", param_set_.timeConeMinus_);
  GetParameter("TimeConePlus", param_set_.timeConePlus_);
  //GetParameter("UseDOMspacings", domSpacingsOpt_);
  GetParameter("SingleDenseRingLimits", param_set_.SingleDenseRingLimits_);
  GetParameter("DoubleDenseRingLimits", param_set_.DoubleDenseRingLimits_);
  GetParameter("TrippleDenseRingLimits", param_set_.TrippleDenseRingLimits_);
  GetParameter("Mode", param_set_.modeOpt_);
  //GetParameter("ConnectTroughDust", connectTroughDustOpt_);
  GetParameter("ModifyObjects", modifyObjectsOpt_);
  GetParameter("SubEventStreamName", sub_event_stream_name_);

  if (inputName_=="")
    log_fatal("Configure the Name of the Input");
  if (outputName_=="")
    log_fatal("Configure the Name of the Output");

  if(modifyObjectsOpt_)
    log_warn("! DANGER ! This mode is currently experimental and leads to blown up SubFrames; so beware");
  
  //gethered enouh information to instantize and configure the HiveSplitter
  hiveSplitter_.Configure(param_set_);
  log_debug("Leaving Configure()");
}

void I3HiveSplitter::Geometry(I3FramePtr frame){
  log_debug("Entering Geometry()");
  I3GeometryConstPtr geo = frame->Get<I3GeometryConstPtr>();
  if (!geo)
    log_fatal("Unable to find geometry data!");

  log_info_stream("BuildDistanceMap" << std::endl);
  hiveSplitter_.BuildDistanceMap(geo);
  log_info_stream("DistanceMap Built" << std::endl);

  splitter_configured_ = true;
  
  PushFrame(frame);
  log_debug("Leaving Geometry()");
}

void I3HiveSplitter::DAQ (I3FramePtr frame) {
  log_debug("Entering DAQ()");
  
  if (!splitter_configured_)
    log_fatal("Splitter was never configured with a G-frame before passing on the first Q-frame; use a GCD-file!");
  
  //fetch inputs
  I3RecoPulseSeriesMapConstPtr pulses = frame->Get<I3RecoPulseSeriesMapConstPtr>(inputName_);
  if (!pulses) {
    log_error_stream("Could not locate the <RecoPulsesSeriesMap>'"<<inputName_ <<"' in the frame; will do nothing");
    PushFrame(frame); //nothing to do here
    return;
  }

  //turn the crank
  std::vector<SubEventStartStop> subESSs = hiveSplitter_.HiveSplitting(*pulses);

  //save the splitcount
  if (saveSplitCount_) {
    //Some users are concerned with how many subevents we created, so let's save them some trouble
    unsigned int nSubEvents=subESSs.size();
    std::ostringstream countName;
    countName << GetName() << "SplitCount";
    frame->Put(countName.str(),boost::make_shared<I3Int>(nSubEvents));
  }

  //if (modifyObjectsOpt_) {
    //make a steady copy of the I3EventHeader
    //I3EventHeader eh_q = *frame->Get<I3EventHeaderConstPtr>("I3EventHeader");
    //frame->Put("QEventHeader", boost::make_shared<I3EventHeader>(eh_q));
  //}

  if (noSplitOpt_) {
    I3RecoPulseSeriesMapMask mask(*frame, inputName_, I3RecoPulseSeriesMap());
    for (std::vector<SubEventStartStop>::const_iterator subESSs_iter=subESSs.begin(); subESSs_iter!=subESSs.end(); ++subESSs_iter) {
        //convert the subevent Hit set into a proper mask on the pulses series
      mask = mask | I3RecoPulseSeriesMapMask(*frame, inputName_, subESSs_iter->subevent_);
    }
    frame->Put(outputName_, boost::make_shared<I3RecoPulseSeriesMapMask>(mask));

    PushFrame(frame);
  }
  else {
    PushFrame(frame);
    
    for (std::vector<SubEventStartStop>::const_iterator subESSs_iter=subESSs.begin(); subESSs_iter!=subESSs.end(); ++subESSs_iter) {
      //convert the subevent Hit set into a proper mask on the pulses series
      const I3RecoPulseSeriesMap &split_pulses = subESSs_iter->subevent_;
      const double &rel_start_time = subESSs_iter->start_time_;
      const double &rel_end_time = subESSs_iter->stop_time_;

      I3FramePtr subframe = GetNextSubEvent(frame);
      if (modifyObjectsOpt_) {
	I3EventHeader mod_eh = *subframe->Get<I3EventHeaderConstPtr>("I3EventHeader");
	I3Time abs_start_time = mod_eh.GetStartTime();
	mod_eh.SetStartTime(abs_start_time+rel_start_time);
	mod_eh.SetEndTime(abs_start_time+rel_end_time);
	subframe->Delete("I3EventHeader"); //remove the old one
	subframe->Put("I3EventHeader", boost::make_shared<I3EventHeader>(mod_eh)); //put in the modified one
	subframe->Put(outputName_+"TimeRange", boost::make_shared<I3TimeWindow>(rel_start_time, rel_end_time));
	I3RecoPulseSeriesMap mod_pulses = split_pulses;
// 	for(I3Map<OMKey,std::vector<I3RecoPulse> >::iterator dom=mod_pulses.begin(); dom!=mod_pulses.end(); dom++){
// 	  for(std::vector<I3RecoPulse>::iterator pulse=dom->second.begin(); pulse!=dom->second.end(); pulse++){
// 	    pulse->SetTime(pulse->GetTime()-rel_start_time);
// 	  }
// 	}
	I3RecoPulseSeriesMapMask mask(*frame, inputName_, mod_pulses);
	subframe->Put(outputName_, boost::make_shared<I3RecoPulseSeriesMapMask>(mask));
	subframe->Put(outputName_ + "TimeRange", boost::make_shared<I3TimeWindow>(subESSs_iter->start_time_,subESSs_iter->stop_time_));
      }
      else {
	I3RecoPulseSeriesMapMask mask(*frame, inputName_, split_pulses);
	subframe->Put(outputName_, boost::make_shared<I3RecoPulseSeriesMapMask>(mask));
//	subframe->Put(outputName_ + "StartTime", boost::make_shared<I3Double>(subESSs_iter->start_time_));
//	subframe->Put(outputName_ + "StopTime",  boost::make_shared<I3Double>(subESSs_iter->stop_time_));
	subframe->Put(outputName_+"TimeRange", boost::make_shared<I3TimeWindow>(rel_start_time, rel_end_time));
      }
      
      PushFrame(subframe);
    }
  }
  log_debug("Leaving DAQ()");
};
