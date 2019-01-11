/*
 *  HuberFit.h
 *  HuberFit
 *
 *  Created by Mark Wellons on 8/2/11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 * This fits a track using a Huber loss function. 
 */

#ifndef I3HUBERFIT_H_INCLUDED
#define I3HUBERFIT_H_INCLUDED

#include "icetray/I3ConditionalModule.h"

#include "boost/numeric/ublas/vector.hpp"

#include <fstream>

using namespace boost::numeric;
using namespace std;

class HuberFit : public I3ConditionalModule
{
public:
	
	/**
	 * Constructor:  builds an instance of the module, with the
	 * context provided by IceTray. 
	 */ 
	HuberFit(const I3Context& ctx);
	
	/**
	 * Destructor: deletes the module
	 */ 
	~HuberFit();
	
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
	string fitname_;
	size_t static const minHits_ = 2;
	const static size_t maxIter = 50;
	
	size_t numDoms;//Number of DOMs that recorded pulses
	const std::string& GetName() const;/// tell your name
	double residual (ublas::vector<double> const para, ublas::vector<double>
	const r_i, double const t); //debugging function
	
    
	//Times and positions of the DOM hits
	std::vector<double> delta_ts;
	std::vector<double> xs;
	std::vector<double> ys;
	std::vector<double> zs;
    
    //Converts the I3 data into c arrays
	void getData(I3FramePtr frame);
    
    SET_LOGGER("HuberFit");
};

#endif
