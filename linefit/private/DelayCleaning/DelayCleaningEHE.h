/*
 *
 *  Created by Mark Wellons on 8/2/11.
 *  This module takes a hitseries as input and removes all the data points that
 are considered late.  A point is late if it arrives some time "t" after the any
 other point within a neighborhood of radius "d"
 */

#ifndef I3DELAYCLEANING_H_INCLUDED
#define I3DELAYCLEANING_H_INCLUDED

#include "icetray/I3ConditionalModule.h"
#include "icetray/OMKey.h"
#include "dataclasses/physics/I3Particle.h"
#include "dataclasses/I3Map.h"
#include "dataclasses/geometry/I3Geometry.h"
#include "dataclasses/physics/I3RecoPulse.h"

#include<fstream>

using namespace std;


/*Convenient container to keep each hit with its associated position in the
quadratic algorithm*/
struct DOMhit_pos
{

	DOMhit_pos(const double &t, const I3Position &pos)
	{
		time = t;
		position = pos;
	}

	DOMhit_pos(DOMhit_pos const & hit)
	{
		time = hit.time;
		position = hit.position;
	}

	double time;
	I3Position position;
};

struct DOMhit_string_om_npe
{

  DOMhit_string_om_npe(const double &t, const int& s, const int &o, const double &n)
  {
    time = t;
    string = s;
    om = o;
    npe = n;
  }
  
  DOMhit_string_om_npe(DOMhit_string_om_npe const & hit)
  {
    time = hit.time;
    string = hit.string;
    om = hit.om;
    npe = hit.npe;
  }

  double time;
  int    string;
  int    om;
  double npe;
};

bool operator<(const DOMhit_pos& left, const DOMhit_pos& right)
{
  return left.time < right.time ;
}

bool operator>(const DOMhit_pos& left, const DOMhit_pos& right)
{
  return left.time > right.time ;
}

bool operator<(const DOMhit_string_om_npe& left, const DOMhit_string_om_npe& right)
{
  return left.time < right.time ;
}

bool operator>(const DOMhit_string_om_npe& left, const DOMhit_string_om_npe& right)
{
  return left.time > right.time ;
}

class DelayCleaningEHE : public I3ConditionalModule
{
public:
	
	/**
	 * Constructor:  builds an instance of the module, with the
	 * context provided by IceTray. 
	 */ 
	DelayCleaningEHE(const I3Context& ctx);
	
	/**
	 * Destructor: deletes the module
	 */ 
	~DelayCleaningEHE();
	
	/**
	 * This module takes a configuration parameter and so it must be configured
	 */
	void Configure();
	
	void Physics(I3FramePtr frame);
	
	
private:
	void WriteOutput(I3FramePtr frame, I3RecoPulseSeriesMapConstPtr selection);
	
	string inputRecoPulses_;
	double distance_;
	double timeWindow_;
	double timeInterval_;
	string outputResponse_;
	static const size_t minHits_ = 8;

    SET_LOGGER("DelayCleaningEHE");
};

#endif
