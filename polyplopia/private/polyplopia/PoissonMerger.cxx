#include <icetray/I3Tray.h>
#include <icetray/Utility.h>
#include "polyplopia/PoissonMerger.h"
#include "polyplopia/PolyplopiaUtils.h"
#include "dataclasses/I3String.h"
#include "dataclasses/physics/I3Particle.h"
#include "dataclasses/I3Time.h"
#include <phys-services/I3SummaryService.h>
#include <algorithm>

 
I3_MODULE(PoissonMerger);

PoissonMerger::PoissonMerger(const I3Context& ctx) : 
  I3Module(ctx),
  mcTreeName_("I3MCTree"),
  separateTree_(""),
  primaryType_("corsika"),
  timeWindow_(40.0*I3Units::microsecond),
  corsikaRate_(NAN),
  counter_(0),
  corsika_primary_(0),
  randomServiceName_("I3RandomService")
{
  AddParameter("PrimaryType",
                  "Primary Type (corsika, nugen, etc). To avoid overcounting with corsika.", 
                  primaryType_);

  AddParameter("CoincidentEventService",
                  "Service to inject background CR events from", 
                  backgroundService_);

  AddParameter("MCTreeName",
				  "Name of I3MCTree objects to merge",
				  mcTreeName_);
  AddParameter("SeparateMCTree",
				  "Put coincident events into this separate MCTree",
				  separateTree_);

  AddParameter("TimeWindow",
                  "Coincident event time window",
                  timeWindow_);

  AddParameter("Rate", 
                  "Event rate (if NaN polyplopia should get this from background file)", 
                  corsikaRate_);
  
  AddParameter("RandomServiceName",
               "The name of the random service to use",
               randomServiceName_);

  AddOutBox("OutBox");

}


void 
PoissonMerger::Configure()
{ 
        GetParameter("PrimaryType", primaryType_);
        GetParameter("CoincidentEventService",backgroundService_); 
        GetParameter("TimeWindow",timeWindow_); 
        GetParameter("MCTreeName", mcTreeName_); 
        GetParameter("SeparateMCTree", separateTree_);
        GetParameter("Rate", corsikaRate_);
        GetParameter("RandomServiceName", randomServiceName_);
  
        // check if this is a corsika primary
        std::transform(
                primaryType_.begin(), 
                primaryType_.end(), 
                primaryType_.begin(), ::tolower);
        std::size_t found = primaryType_.find("corsika"); 
        corsika_primary_ = (found!=std::string::npos);

        if (corsika_primary_)
           log_info("Assuming CORSIKA primary");
        else
           log_info("Assuming non-CORSIKA primary");
        
        randomService_  = context_.Get<I3RandomServicePtr>(randomServiceName_);
        if (!randomService_)
                log_fatal_stream("I3RandomService " << randomServiceName_ << " not found in context");

        if (!backgroundService_) {
            log_fatal("No CoincidentEventService found'");
        }
        if (std::isnan(corsikaRate_)) {
            corsikaRate_ = backgroundService_->GetRate();
        }

        log_notice("Rate=%fHz, Time window=%fus, Poisson mean=%f", 
                        corsikaRate_/I3Units::hertz, 
                        timeWindow_/I3Units::microsecond,
                        timeWindow_*corsikaRate_);
}
                

void 
PoissonMerger::DAQ(I3FramePtr frame)
{ 
        // determine how many CR muons in static window
        double mean = timeWindow_*corsikaRate_;
        int poisson_value = -1;
        while (poisson_value < 0)
        { 
                // avoid overcouting if primary and background 
                // are both cosmic rays
                poisson_value = randomService_->Poisson(mean); 
                if (corsika_primary_) 
                        poisson_value -= 1;
        } 
        unsigned int events_to_merge = poisson_value;

        log_trace("merging %d events" , events_to_merge);
        coincidenceHistogram_[events_to_merge]++;

        I3MCTree mctree = frame->Get<I3MCTree>(mcTreeName_);
        I3MCTreePtr main_tree;
        if (separateTree_.empty()) {
           main_tree = I3MCTreePtr(new I3MCTree(mctree));
	} else { // create empty tree
           main_tree = I3MCTreePtr(new I3MCTree());
	}

        const I3Particle primary = I3MCTreeUtils::GetPrimaries(mctree)[0];
        frame->Put("PolyplopiaPrimary",I3ParticlePtr(new I3Particle(primary)));

        double mean_time = PolyplopiaUtils::GetEarliestInIceTime(mctree);
	//log_notice_stream("Original tree time: " << mean_time/I3Units::microsecond << " us");

        for (unsigned int event_i = 0;event_i < events_to_merge; event_i++) 
        { 
                I3MCTreePtr ctree   = backgroundService_->GetNextEvent();
                if (!ctree) { 
                        log_fatal("Ran out of background events");
                }

                // Get a time centered around start of event
                double ti = randomService_->Uniform(
                                mean_time-0.5*timeWindow_,
                                mean_time+0.5*timeWindow_);

                // get earliest time for each coincident event 
                double ctime = PolyplopiaUtils::GetEarliestInIceTime(*ctree);
                if ( std::isnan(ctime)  ) { 
			log_fatal("Unable to find a valid time in coincident events"); 
		} 
	      	// Make sure that the time ti is relative to time time1 of the main tree
		PolyplopiaUtils::MergeMCTrees(*main_tree,*ctree, ti-ctime);
		    
        } 

	if (separateTree_.empty()) { 
		frame->Delete(mcTreeName_); 
		frame->Put(mcTreeName_,main_tree);
	} else { 
		frame->Put(separateTree_,main_tree);
	} 
	PushFrame(frame); 
	counter_ += 1;
}

void 
PoissonMerger::Finish()
{ 
      log_info("Coincidence multiplicities:");
      typedef I3Map<unsigned int, unsigned int> hist_t;
      BOOST_FOREACH(hist_t::value_type &i, coincidenceHistogram_)
         log_notice("%u coincidences: %u",i.first,i.second);
      log_notice("processed %d events" , counter_);
}
          

