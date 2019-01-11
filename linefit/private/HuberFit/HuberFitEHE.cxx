/**
 *Author: Mark Wellons 
 */

#include "HuberFit/HuberFitEHE.h"
#include "HuberFit/HuberFitPhysics.h"


I3_MODULE(HuberFitEHE);


#include "icetray/I3Units.h"
#include "dataclasses/I3Constants.h"
#include "dataclasses/physics/I3Particle.h"
#include "dataclasses/geometry/I3Geometry.h"
#include "icetray/I3PhysicsTimer.h"
#include "icetray/I3TrayHeaders.h"
#include "dataclasses/physics/I3RecoPulse.h"

#include "icetray/OMKey.h"
#include "dataclasses/I3Map.h"

#include<cmath>
#include<vector>
#include<algorithm>
#include<iostream>
#include<string>
#include<fstream>

#include <boost/numeric/ublas/vector.hpp>
#include <boost/numeric/ublas/io.hpp>
#include <boost/lexical_cast.hpp>


using namespace boost::numeric;
using boost::lexical_cast;
using ublas::norm_2;
using std::string;
using std::cout;


/*Parameters should be in a six-vector in the form (r_x, r_y, r_z, v_x,v_y,
v_z).  This is a debugging function, and is not used in the Huber Fit.  */
double HuberFitEHE::residual (ublas::vector<double> const para,
ublas::vector<double> const r_i, double const t )
{
	double static d_squared = distance_*distance_;
	ublas::vector<double> v (3);
	ublas::vector<double> r_0 (3);
	
	for(int i = 0; i <3; i++)
	{
		r_0(i) = para(i);
		v(i) = para(i+3);
	}
	double res = norm_2(v*t + r_0 - r_i);
	if(res < distance_)
	{
		return res*res;
	}
	else
	{
		return 2*distance_*res - d_squared;
	}
}

//Fills in the position and time arrays with the DOM hit information
void HuberFitEHE::getData(I3FramePtr frame)
{
	
	
	/*Make sure we are working with a clean slate.  Note that these are global
	variables, so they need to be cleared for each frame. */
	delta_ts.clear();
	xs.clear();
	ys.clear();
	zs.clear();
	numDoms = 0;


	I3RecoPulseSeriesMapConstPtr pulsemap =
	frame->Get<I3RecoPulseSeriesMapConstPtr>(inputRecoPulses_);
	const I3Geometry& geometry = frame->Get<I3Geometry>();

	I3Map<OMKey, vector<I3RecoPulse> >::const_iterator selector;
	
	//Grab times and positions of the DOM hits.  
	for(selector = pulsemap->begin(); selector != pulsemap->end(); selector++)
	{
		// get the position of the hit OM (OMKey is "first" record of map)
		const OMKey om = selector->first;
		I3OMGeoMap::const_iterator geom = geometry.omgeo.find(om);
		if (geom == geometry.omgeo.end())
		{
			log_trace("Didn't find the current OMKey in InIceGeometry");
			continue;
		}
	
		const I3Position& ompos = geom->second.position;
		double x = ompos.GetX();
		double y = ompos.GetY();
		double z = ompos.GetZ();	
	
		const vector<I3RecoPulse>& hits = selector->second;
		if(!hits.size()) 
		{
			log_debug("The 'hit' series was found but is empty.");
			continue;
		}
		for (size_t i=0; i<hits.size(); i++) 
		{
			xs.push_back(x);
			ys.push_back(y);
			zs.push_back(z);
	    	delta_ts.push_back(hits[i].GetTime()); // use all pulses
		}
		numDoms +=1; 
		
	}
	
}

HuberFitEHE::HuberFitEHE(const I3Context& context) : I3ConditionalModule(context)
{	
	AddOutBox("OutBox");
	
	inputRecoPulses_ = "Pulses_delay_cleaned";
	AddParameter("InputRecoPulses",
				 "HitSeriesMap to use for input",
				 inputRecoPulses_);
	
	distance_ = 153*I3Units::m;
	AddParameter("Distance",
				 "The Huber parameter.  This is the cutoff that separates the"
				 "linear and quadradic regime.  ",
				 distance_);

	fitname_ = "HuberFitEHE"+lexical_cast<string>(distance_);
	AddParameter("Name",
				 "The name of the output fit",
				 fitname_);
	
}

//No memory management, so no clean up required.  
HuberFitEHE::~HuberFitEHE()
{}

void HuberFitEHE::Configure()
{
	GetParameter("InputRecoPulses",inputRecoPulses_);
	GetParameter("Distance",distance_);
	GetParameter("Name",fitname_);	
}

void HuberFitEHE::Physics(I3FramePtr frame)
{
	//Make sure the input even exist. Otherwise give up.
	if(!frame->Has(inputRecoPulses_))
	{
		log_warn("Frame does not contain the specified input response."
		"Huber fit was not applied");
		PushFrame(frame,"OutBox");
		return;
	}
	
	//Grab the DOM hit information
	getData(frame);

	// Create the resultant particle so we can fill it later and put it in frame.
  	I3ParticlePtr track(new I3Particle());
	

	//Check that we have enough hits.  We need at least 2 DOM to have hits
	if (numDoms < minHits_) 
	{
		log_info("Not enough DOMs were found (read %i, Min set to %i  Exiting.)",
		(int) numDoms, (int) minHits_);
		track->SetFitStatus(I3Particle::InsufficientHits);
		frame->Put(fitname_, track);
		PushFrame(frame,"OutBox");
		return;
	}

	//Compute Huber fit
	
	//This holds the position and velocity parameters for the reconstruction
	double para[6];
	double t_0; //First hit in the track
	size_t step_counter;//Number of iterations it took the minimizer to converge
 	
	step_counter= computeHuberFit_wArray(xs, ys, zs, delta_ts, para,  maxIter,
	distance_, t_0);

	log_debug("There were %i iterations before termination", (int) step_counter);

	if(step_counter >= maxIter)
	{
		log_info("Huber function took too many steps to converge. Convergence"
		"was terminated.");
	}
	
	//Set Track
	double speed = para[3]*para[3] + para[4]*para[4] + para[5]*para[5];
	speed = std::sqrt(speed);
	
	track->SetShape(I3Particle::InfiniteTrack);
	track->SetFitStatus(I3Particle::OK);
	track->SetPos(para[0],para[1],para[2]);
	track->SetDir(para[3],para[4],para[5]);
	track->SetTime(t_0);
	track->SetSpeed(speed);

	frame->Put(fitname_, track);
	PushFrame(frame,"OutBox");
	
	log_debug("Exiting Huberfit Physics.");
}
