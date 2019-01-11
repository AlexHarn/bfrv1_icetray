/*
 *
 *  Created by Mark Wellons on 8/2/11.
 *	This module takes an input set of data points and an input track, and
 computes the distance each data point is in phase space from the track.  It
 then removes the data points that exceeded some the input cutoff.    
 */

#ifndef I3DEBIASING_H_INCLUDED
#define I3DEBIASING_H_INCLUDED

#include "icetray/I3ConditionalModule.h"
#include "dataclasses/physics/I3Particle.h"

#include <iostream>
#include <fstream>

using namespace std;


class DebiasingEHE : public I3ConditionalModule
{
public:
	
	/**
	 * Constructor:  builds an instance of the module, with the
	 * context provided by IceTray. 
	 */ 
	DebiasingEHE(const I3Context& ctx);
	
	/**
	 * This module takes a configuration parameter and so it must be configured
	 */
	void Configure();
	
	/** 
	 * If event readout data comes by, we fit it.
	 */ 
	void Physics(I3FramePtr frame);
	
	
private:


	string inputRecoPulses_;
	double distance_;
	string seedname_;
	string outputResponseName_;
	
    const double computeResidual(const I3ParticleConstPtr seed_, const
    I3Position &hitPos, const double hitTime) const;     
	
    size_t static const minHits_ = 8;
    
    SET_LOGGER("DebiasingEHE");
};

#endif
