#ifndef I3DIPOLEFITEXTRAS_H
#define I3DIPOLEFITEXTRAS_H

namespace dipolefit
{
  // Local convenience struct for holding hits.
  struct Hit { double x, y, z, t, amp; };
  
  // Freestanding comparison operator for local convenience Hits.
  bool operator<(Hit l, Hit r) { return l.t < r.t; }
  
  // Local function to extract hits/pulses from frame.
  void Fill(I3RecoPulseSeriesMapConstPtr hitmap, 
	    const I3Geometry& geometry, std::vector<Hit>& hitvect)
    {
      I3RecoPulseSeriesMap::const_iterator selector;
      for(selector=hitmap->begin(); selector!=hitmap->end(); selector++) {
	// get the position of the hit OM (OMKey is "first" record of map)
	OMKey om = selector->first;
	I3OMGeoMap::const_iterator geom = geometry.omgeo.find(om);
	if (geom==geometry.omgeo.end()) {
	  log_trace("Didn't find the current OMKey in InIceGeometry");
	  continue;
	}
	const I3Position& ompos = geom->second.position;
	
	log_trace("Looking at OM %i on string %i.", 
		  selector->first.GetOM(), selector->first.GetString());
	const I3RecoPulseSeries& hits = selector->second;
	
	if(!hits.size()) continue; // make sure the pulse series is not empty
	
	const I3RecoPulse& pulse = hits.front(); // only use first pulse...
	Hit hit;
	hit.t = pulse.GetTime();
	hit.x = ompos.GetX();
	hit.y = ompos.GetY();
	hit.z = ompos.GetZ();
	hit.amp = pulse.GetCharge();
	
	if (!std::isnan(hit.t)) {
	  hitvect.push_back(hit);
	  log_debug("Got time %f at x=%f, y=%f, z=%f", hit.t ,hit.x ,hit.y ,hit.z);
	}
      }
    };
}
#endif
