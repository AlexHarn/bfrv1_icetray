#ifndef I3LINEFITEXTRAS_H
#define I3LINEFITEXTRAS_H

namespace LineFitExtras
{
  void Fill(I3RecoPulseSeriesMapConstPtr hitmap, 
	    const I3Geometry& geometry, const std::string& leadingEdge_, 
	    const double& ampWeightPower_, double& avTime, double& avTimeSquared,
	    double avPos[3], double avTP[3], double& ampsum, int& nHits)
    {
      I3RecoPulseSeriesMap::const_iterator selector;

      for(selector = hitmap->begin(); selector != hitmap->end(); selector++){
	double amp = 1.0;
	double ampValue;
	std::vector<double> times;

	// get the position of the hit OM (OMKey is "first" record of map)
	const OMKey om = selector->first;
	I3OMGeoMap::const_iterator geom = geometry.omgeo.find(om);
	if (geom==geometry.omgeo.end()) {
	  log_trace("Didn't find the current OMKey in InIceGeometry");
	  continue;
	}
	const I3Position& ompos = geom->second.position;
	double x = ompos.GetX();
	double y = ompos.GetY();
	double z = ompos.GetZ();	

	//NOTE: Assuming first pulse/hit is same as first entry in this vector
	const I3RecoPulseSeries& hits = selector->second;
	if(!hits.size()) {
	  log_debug("The 'hit' series was found but is empty.");
	  continue;
	}

	if (leadingEdge_ == "ALL") {
	  for (unsigned int i=0; i<hits.size(); i++) 
	    times.push_back(hits[i].GetTime()); // use all pulses
	} else {
	  times.push_back(hits[0].GetTime()); // use only first pulses. 
	}
	//Taken from original code - was stated that implementation to use 
	//all pulse times would be added later.
	ampValue = hits[0].GetCharge();

	// Calculate event weights using amplitudes and parameters.
	if (ampWeightPower_ != 0.) {
	  amp = pow(ampValue,ampWeightPower_);
	  log_debug("Amplification is %f, using amplitude=%f ^Ampweightpower=%f",
		    amp, ampValue, ampWeightPower_);
	}

	for (unsigned int i=0; i<times.size(); i++) {
	  double time = times[i];
	  if (!std::isnan(time)) { // skipp NAN times...
	    nHits++; // add this hit to the list of total hits
	    avTime += amp*time;
	    avTimeSquared += amp*time*time;
	    ampsum+=amp;
	    avPos[0] += amp*x;
	    avPos[1] += amp*y;
	    avPos[2] += amp*z;
	    avTP[0] += amp*x*time;
	    avTP[1] += amp*y*time;
	    avTP[2] += amp*z*time;
	  }
	}
      }
    }

};

#endif
