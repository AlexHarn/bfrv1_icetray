// 
// copyright  (C) 2012
//    Jakob van Santen <vansanten@wisc.edu>
//    and the IceCube Collaboration
// @version $Id$
// @file I3FillRatioLite.cxx
// @brief An efficient implementation of the FillRatio algorithm.
// @date $Date$

#include "icetray/I3ConditionalModule.h"
#include "dataclasses/I3Double.h"
#include "dataclasses/geometry/I3Geometry.h"
#include "dataclasses/physics/I3RecoPulse.h"
#include "dataclasses/physics/I3Particle.h"
#include "fill-ratio/FastMinBall.h"

#include <boost/foreach.hpp>

class I3FillRatioLite : public I3ConditionalModule {
public:
	I3FillRatioLite(const I3Context&);
	virtual ~I3FillRatioLite();
	void Configure();
	void Geometry(I3FramePtr);
	void DetectorStatus(I3FramePtr);
	void Physics(I3FramePtr);
private:
	void FillSubGeometry();
	
        std::string badDOMListName_;
        std::vector<OMKey> staticBadDOMList_;
        std::set<OMKey> badOMs_;
   
        I3GeometryConstPtr geometry_;
        FastMinBall *minballer_;
	
	std::string vertex_name_, pulses_name_, output_name_;
	double distance_scale_, weighting_power_;
};

I3_MODULE(I3FillRatioLite);

I3FillRatioLite::I3FillRatioLite(const I3Context &ctx) :
    I3ConditionalModule(ctx), minballer_(NULL), distance_scale_(1.), weighting_power_(0.)
{
	AddOutBox("OutBox");
	
	AddParameter("Vertex", "Name of the particle to use as a vertex",
	    vertex_name_);
	AddParameter("Pulses", "Name of the pulse series in the frame",
	    pulses_name_);
	AddParameter("Output", "Name of the I3Double to put in the frame",
	    output_name_);
	AddParameter("Scale", "Factor by which to scale the mean vertex-DOM distance",
	    distance_scale_);
	AddParameter("AmplitudeWeightingPower", "Weight hits by the charge to this "
	    "power", weighting_power_);
	AddParameter("ExcludeDOMs", "List of DOMs to exclude from the calculation "
	    "(e.g. all DeepCore DOMs)", staticBadDOMList_);
	AddParameter("BadDOMListName", "Name of the dynamic BadDOMList in the "
	    "DetectorStatus frame", badDOMListName_);
	
}

I3FillRatioLite::~I3FillRatioLite()
{
	if (minballer_ != NULL)
		delete minballer_;
}

void I3FillRatioLite::Configure()
{
	GetParameter("Vertex", vertex_name_);
	GetParameter("Pulses", pulses_name_);
	GetParameter("Output", output_name_);
	GetParameter("Scale", distance_scale_);
	GetParameter("AmplitudeWeightingPower", weighting_power_);
	GetParameter("ExcludeDOMs", staticBadDOMList_);
	GetParameter("BadDOMListName", badDOMListName_);
	
        std::copy(staticBadDOMList_.begin(), staticBadDOMList_.end(),
	    std::inserter(badOMs_, badOMs_.begin()));
}

void I3FillRatioLite::Geometry(I3FramePtr frame)
{
	geometry_ = frame->Get<I3GeometryConstPtr>();
	if (geometry_ && badOMs_.size())
		FillSubGeometry();
	PushFrame(frame);
}

void I3FillRatioLite::DetectorStatus(I3FramePtr frame)
{
	I3VectorOMKeyConstPtr badDOMList =
	    frame->Get<I3VectorOMKeyConstPtr>(badDOMListName_);
	if (badDOMList) {
		badOMs_.clear();
	        std::copy(staticBadDOMList_.begin(), staticBadDOMList_.end(),
		    std::inserter(badOMs_, badOMs_.begin()));
	        std::copy(badDOMList->begin(), badDOMList->end(),
		    std::inserter(badOMs_, badOMs_.begin()));
	} else if (badDOMListName_.size() > 0)
		log_error("Got a DetectorStatus frame without BadDOMList '%s'!",
		    badDOMListName_.c_str());
	if (geometry_)
		FillSubGeometry();
	PushFrame(frame);
}

void I3FillRatioLite::FillSubGeometry()
{
	I3OMGeoMap subgeo(geometry_->omgeo);
	I3OMGeoMap::iterator geoit = subgeo.begin();
	for ( ; geoit != subgeo.end(); )
		if (badOMs_.find(geoit->first) != badOMs_.end())
			subgeo.erase(geoit++);
		else
			++geoit;
	
	if (minballer_ != NULL)
		delete minballer_;
	minballer_ = new FastMinBall(subgeo);
}

void I3FillRatioLite::Physics(I3FramePtr frame)
{
	I3ParticleConstPtr particle =
	    frame->Get<I3ParticleConstPtr>(vertex_name_);
	I3RecoPulseSeriesMapConstPtr pulses =
	    frame->Get<I3RecoPulseSeriesMapConstPtr>(pulses_name_);

	if (!geometry_)
		log_fatal("I haven't seen a Geometry frame yet!");
	else if (!minballer_)
		FillSubGeometry();
	
	if (!pulses || !particle || particle->GetFitStatus() != I3Particle::OK) {
		PushFrame(frame);
		return;
	}
	
	FastMinBall::geo_iter_t geopos;
	const I3Position &vertex = particle->GetPos();
	
	double total_distance = 0.;
	double total_weight = 0.;
	BOOST_FOREACH(const I3RecoPulseSeriesMap::value_type &hitpair, *pulses) {
		geopos = minballer_->LowerBound(hitpair.first, geopos);
		if (geopos->first != hitpair.first)
			continue;
		
		double charge = 0.;
		BOOST_FOREACH(const I3RecoPulse &pulse, hitpair.second)
			charge += pulse.GetCharge();
		
		double weight = (charge > 0.) ?
		    ((weighting_power_ == 0.) ? 1. : pow(charge, weighting_power_)): 0.;
		total_distance += weight*(vertex-geopos->second.position).Magnitude();
		total_weight += weight;
	}
	
	double mean_distance = distance_scale_*total_distance/total_weight;
	
	int n_hit(0), n_doms(0);
	BOOST_FOREACH(const I3OMGeoMap::value_type &geopair,
	    minballer_->GetMinBallGeometry(vertex, mean_distance)) {
		n_doms++;
		if (pulses->find(geopair.first) != pulses->end())
			n_hit++;
	}
		
	frame->Put(output_name_,
	    I3DoublePtr(new I3Double(double(n_hit)/double(n_doms))));
	
	PushFrame(frame);
}


