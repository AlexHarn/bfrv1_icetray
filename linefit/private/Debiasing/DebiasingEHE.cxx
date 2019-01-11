/**
 *Author: Mark Wellons
 *This module takes an input set of data points and an input track, and computes
 the distance each data point is in phase space from the track.  It then removes
 the data points that exceeded some value.    
 */

#include "Debiasing/DebiasingEHE.h"

I3_MODULE(DebiasingEHE);

#include "dataclasses/physics/I3Particle.h"
#include "dataclasses/geometry/I3Geometry.h"
#include "icetray/I3PhysicsTimer.h"
#include "icetray/I3TrayHeaders.h"
#include "dataclasses/physics/I3RecoPulse.h"
#include "phys-services/I3Calculator.h"


#include "icetray/OMKey.h"
#include "dataclasses/I3Map.h"

#include<vector>
#include<iostream>
#include<cmath>

using namespace std;

inline double square(double const x)
{
	return x*x;
}

/*takes a hit Position and computes the residual with respect to the input track
*/
const double DebiasingEHE::computeResidual(const I3ParticleConstPtr seed, const
I3Position &hitPos, const double hitTime) const
{
	const I3Position *pos = &(seed ->GetPos());
	const I3Direction *dir = &(seed ->GetDir());
	const double s = seed ->GetSpeed();
	const double t0 = seed -> GetTime(); 
	
	const double e_x = dir->GetX();
	const double e_y = dir->GetY();
	const double e_z = dir->GetZ();
	
	const double r0_x = pos->GetX();
	const double r0_y = pos->GetY();
	const double r0_z = pos->GetZ();
	
	const double t = hitTime - t0;
	
	return sqrt( square(e_x*s*t+ r0_x - hitPos.GetX()) + square(e_y*s*t+ r0_y -
	hitPos.GetY()) + square(e_z*s*t+ r0_z - hitPos.GetZ()) );
	
}
 

DebiasingEHE::DebiasingEHE(const I3Context& context) : I3ConditionalModule(context)
{	
	AddOutBox("OutBox");
		
	inputRecoPulses_ = "Pulses_delay_cleaned";
	AddParameter("InputResponse",
				 "HitSeriesMap to use for input",
				 inputRecoPulses_);
	
	distance_ = 116*I3Units::m;
	AddParameter("Distance",
				 "Distance in phase space a hit is allowed to be from the track"
				 "before we discard it",
				 distance_);
				 
	seedname_ = "linefit";
	AddParameter("Seed", 
				  "The seed we use to determine whether hits are far or near",
				 seedname_);

	outputResponseName_ = "Pulses_delay_cleaned";
	AddParameter("OutputResponse",
				 "Name to give the new pulse series.  ",
				 outputResponseName_);
}

void DebiasingEHE::Configure()
{
	GetParameter("InputResponse",inputRecoPulses_);
	GetParameter("OutputResponse",outputResponseName_);
	GetParameter("Distance",distance_);
	GetParameter("Seed",seedname_);
}


void DebiasingEHE::Physics(I3FramePtr frame)
{
	I3RecoPulseSeriesMapConstPtr oldMap =
	frame->Get<I3RecoPulseSeriesMapConstPtr>(inputRecoPulses_);
	const I3Geometry& geometry = frame->Get<I3Geometry>();

	//New data set with cleaned hits
	I3RecoPulseSeriesMapPtr newMap(new I3RecoPulseSeriesMap);  
  
  	//This is where we handle errors
  	if(!frame->Has(inputRecoPulses_))
	{
		log_warn("Frame does not contain the specified input response.");
		PushFrame(frame,"OutBox");
		return;
	}
	else if(!frame->Has(seedname_))
	{
		log_warn("Frame does not contain the specified seed.");
		PushFrame(frame,"OutBox");
		return;
	}
  
	if(oldMap -> empty())
	{
		log_info("Frame does not contain pulses.");
		frame->Put(outputResponseName_,newMap);
		PushFrame(frame,"OutBox");
		return;
	}
	
	I3ParticleConstPtr seed = frame->Get<I3ParticleConstPtr>(seedname_);
	if(seed -> GetFitStatus() != I3Particle::OK )
	{
		log_info("Seed particle has a bad fit status. DebiasingEHE not applied.");
		log_info("Particle's fit status was reported as %s",
		(seed -> GetFitStatusString()).c_str() );
		frame->Put(outputResponseName_, oldMap);
		PushFrame(frame,"OutBox");
		return;
	}
	log_info("size of old hits %d", (int)oldMap->size());

	//Begin actual data processing
	//Throw away hits outside the maximun residual distance
	log_debug("Extracting hits in DebiasingEHE");
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
			log_warn("Didn't find the current OMKey in InIceGeometry");
			continue;
		}
		const I3Position& ompos = geom->second.position;
		const vector<I3RecoPulse>& oldhits = iter->second;
		vector<I3RecoPulse> newhits;
		
		
		for(size_t i=0; i<oldhits.size(); i++) 
		{	
			//Is this hit's residual low enough to keep?
			if(computeResidual(seed, ompos, oldhits[i].GetTime()) <=  distance_)
			{
				newhits.push_back(oldhits[i]);
				newhits[i].SetCharge(sqrt(oldhits[i].GetCharge()));
			}
		}
		//If we kept any of the pulses on this DOM, add them to the new pulsemap.
		if(!newhits.empty())
		{
			(*newMap)[om] = newhits;
		}
	}
  	//Check that we didn't throw away too many hits
  	if(newMap -> size() < minHits_ )
  	{
  		log_info("The debiasing throw away too many hits.  "
			 "DebiasingEHE was not applied. %d, %d", (int)oldMap->size(), (int)newMap->size());
  		frame->Put(outputResponseName_, oldMap);
  	}
  	else//Put new hit series in frame
	{
		frame->Put(outputResponseName_, newMap);
		log_info("size of new hits %d", (int)newMap->size());
	}
	PushFrame(frame,"OutBox");
	log_debug("Exiting DebiasingEHE Physics.");
}
