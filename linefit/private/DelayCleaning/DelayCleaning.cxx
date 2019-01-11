/**
 *Author: Mark Wellons
 *
 */

#include "DelayCleaning/DelayCleaning.h"

I3_MODULE(DelayCleaning);

#include "dataclasses/physics/I3Particle.h"
#include "dataclasses/geometry/I3Geometry.h"
#include "icetray/I3PhysicsTimer.h"
#include "icetray/I3TrayHeaders.h"
#include "dataclasses/physics/I3RecoPulse.h"
#include "dataclasses/I3MapOMKeyMask.h"
#include "boost/make_shared.hpp"

#include "icetray/OMKey.h"
#include "dataclasses/I3Map.h"

#include<vector>
#include<iostream>
#include<cfloat>
#include<algorithm>


using namespace std;


DelayCleaning::DelayCleaning(const I3Context& context) : I3ConditionalModule(context)
{	
	AddOutBox("OutBox");
		
	inputRecoPulses_ = "NFEMergedPulses";
	AddParameter("InputResponse",
				 "HitSeriesMap to use for input",
				 inputRecoPulses_);
	
	distance_ = 156*I3Units::m;
	AddParameter("Distance",
				 "Radius of sphere in which we look for other hits",
				 distance_);
				 
	timeWindow_ = 778*I3Units::ns;
	AddParameter("TimeWindow", 
				  "The window that we consider local.",
				 timeWindow_);

	outputResponse_ = "debiasedPulses";
	AddParameter("OutputResponse",
				 "Name to give the new pulse series.  ",
				 outputResponse_);
	
}


//No memory management, so no clean up required.  
DelayCleaning::~DelayCleaning()
{}

void DelayCleaning::Configure()
{
	GetParameter("InputResponse",inputRecoPulses_);
	GetParameter("OutputResponse",outputResponse_);
	GetParameter("Distance",distance_);
	if(distance_ < 0)
	{
		log_warn("Distance cannot be less than zero.  "
		"This was almost certainly misconfigured.  Setting to zero");
		distance_ = 0;
	}
	
	GetParameter("TimeWindow",timeWindow_);
	if(timeWindow_ < 0)
	{
		log_warn("The time window cannot be less than zero.  "
		"This was almost certainly misconfigured.  Setting to zero");
		timeWindow_ = 0;
	}
	
}

void DelayCleaning::WriteOutput(I3FramePtr frame, I3RecoPulseSeriesMapConstPtr
selection)
{
	I3RecoPulseSeriesMapMaskPtr mask = 
	    boost::make_shared<I3RecoPulseSeriesMapMask>(*frame,
	    inputRecoPulses_, *selection);
	frame->Put(outputResponse_, mask);
	PushFrame(frame);
}

void DelayCleaning::Physics(I3FramePtr frame)
{

	int numHits = 0;
		
	
	I3RecoPulseSeriesMapConstPtr oldMap =
	frame->Get<I3RecoPulseSeriesMapConstPtr>(inputRecoPulses_);
	if (!oldMap) {
		log_error("Couldn't find pulses '%s' in the frame!",
		inputRecoPulses_.c_str());
		PushFrame(frame);
		return;
	}

	//New data set with cleaned hits
	I3RecoPulseSeriesMapPtr newMap(new I3RecoPulseSeriesMap); 
	const I3Geometry& geometry = frame->Get<I3Geometry>();

	vector<DOMhit_pos> firstHits;
	firstHits.reserve(20);   // Reserve some space
	
	//Get all the first hits
	log_debug("extracting hits in DelayCleaning");
	for(I3RecoPulseSeriesMap::const_iterator iter = oldMap -> begin(); iter !=
	oldMap ->end(); ++iter)
	{
	
		const vector<I3RecoPulse>& hits = iter->second;
		if(!hits.size()) {
			log_debug("The 'hit' series was found but is empty.");
			continue;
		}
	
		const OMKey om = iter->first;
		I3OMGeoMap::const_iterator geom = geometry.omgeo.find(om);
		if (geom == geometry.omgeo.end())
		{
			log_info("Didn't find the OMKey (%i,%i) in I3Geometry", 
			om.GetString(), om.GetOM());
			continue;
		}
	
		const I3Position& ompos = geom->second.position;
	
		//We assume the pulses are in chronological order
		DOMhit_pos temp(hits[0].GetTime(), ompos);
		firstHits.push_back( temp );
	}
  
	if(firstHits.empty())
	{
		log_debug("Frame does not contain pulses.");
		WriteOutput(frame, newMap);
		return;
	}

	//Figure out which hits we want to filter out
	for(I3RecoPulseSeriesMap::const_iterator iter = oldMap -> begin(); iter !=
	oldMap ->end(); ++iter)
	{
	
		const OMKey om = iter->first;
		I3OMGeoMap::const_iterator geom = geometry.omgeo.find(om);
		const I3Position& ompos = geom->second.position;
		
		//Collect the earliest time in the local area
		double t0 = DBL_MAX; //Earliest Time
		vector<DOMhit_pos>::iterator earlyHit;
		for( earlyHit = firstHits.begin(); earlyHit < firstHits.end(); earlyHit++)
		{
			 //Find hits in the local area
			if((earlyHit -> position - ompos).Magnitude() < distance_)
			{
				if(t0 > earlyHit -> time)
				{
					t0 = earlyHit -> time;
				}
			}
		}
		
		//Is this hit within an acceptable time window of the earliest hit?
		const vector<I3RecoPulse>& oldhits = iter->second;
		vector<I3RecoPulse> newhits;
		numHits += oldhits.size();
		for(unsigned int i=0; i<oldhits.size(); i++) 
		{
			/*We assume the pulses are in chronological order, though we don't
			exploit that here*/
			if(oldhits[i].GetTime() <= t0 + timeWindow_ )
			{
				newhits.push_back(oldhits[i]);
			}
		}
		if(!newhits.empty())
		{
			(*newMap)[om] = newhits;
		}
	}
	// new hit series in frame
	if(newMap -> size() < minHits_)
	{
		log_debug("Too many hits were removed.  Delay filter not applied. ");
		WriteOutput(frame, oldMap);
	}
	else
	{
		WriteOutput(frame, newMap);
	}
	log_debug("Exiting DelayCleaning Physics.");
}
