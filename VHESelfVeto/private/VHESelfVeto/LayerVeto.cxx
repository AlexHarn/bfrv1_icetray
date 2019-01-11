
// Claudio and Nathan's containment cut, Chinese Wall edition.
//
// For each event, slide a window through a time-ordered series of pulses
// until a given charge threshold is reached, indicating the start of the
// real physics in the event. This defines the veto window. Then, count
// the charge collected in any of the DOMs in the veto layer(s) during
// the veto window.
//
// Subtleties:
// - The time series is constructed without DOMs that would contribute
//   more than 90% to the total charge in the event. This is intended
//   to mitigate distortions caused by balloon hits.
// - The charge threshold is proportional to the total charge between
//   set limits. By default, it merges smoothly with the preset maximum
//   above a total charge of 6000 PE.

#include <icetray/I3ConditionalModule.h>
#include <icetray/I3Units.h>
#include <dataclasses/geometry/I3Geometry.h>
#include <dataclasses/calibration/I3Calibration.h>
#include <dataclasses/status/I3DetectorStatus.h>
#include <dataclasses/physics/I3RecoPulse.h>
#include <dataclasses/I3MapOMKeyMask.h>
#include <dataclasses/I3Constants.h>
#include <dataclasses/I3Vector.h>
#include <dataclasses/I3Double.h>

#include <boost/python/object.hpp>
#include <boost/foreach.hpp>
#include <boost/make_shared.hpp>
#include <boost/next_prior.hpp>

namespace python = boost::python;

class LayerVeto : public I3ConditionalModule {
    public:
	    LayerVeto(const I3Context&);
	    void Configure();
	    void Physics(I3FramePtr);
	    void Geometry(I3FramePtr);
	    void Calibration(I3FramePtr);
	    void DetectorStatus(I3FramePtr);
		
    private:
	    typedef std::set<OMKey> DOMSet;
	    std::vector<DOMSet> vetoLayers_;
	
	    std::set<OMKey> badDOMList_;
	    I3GeometryConstPtr geometry_;
	    I3CalibrationConstPtr calibration_;
	    I3DetectorStatusConstPtr status_;
	    std::string pulseName_, badDOMListName_, geometryName_, outputName_;
	    double qmax_, qmin_, qscale_, timeWindow_, tagThreshold_;
	    python::object layerConstructor_;
	
	    double GetVetoCharge(const I3RecoPulseSeriesMap&, std::vector<I3RecoPulseSeriesMap> &, double &);
	    void ConstructVetoLayers();
};

I3_MODULE(LayerVeto);

LayerVeto::LayerVeto(const I3Context &ctx) : I3ConditionalModule(ctx)
{
	AddOutBox("OutBox");
	
	AddParameter("VetoLayers", "Callback to define veto layers", layerConstructor_);
	
	pulseName_ = "OfflinePulsesHLCNoDeepCore";
	AddParameter("Pulses", "Name of the pulse series to examine for veto hits", pulseName_);

	qmax_ = 250.;
	AddParameter("QMax", "Maximum charge required to end the veto window", qmax_);
	
	qmin_ = 3.;
	AddParameter("QMin", "Minimum charge required to end the veto window", qmin_);
	
	qscale_ = 24.;
	AddParameter("QScale", "For total charges between QMin and QMax, require QTotal/QScale charge to end the veto window", qscale_);

	tagThreshold_ = std::numeric_limits<double>::infinity();
	AddParameter("TagThreshold", "Charge threshold for tagging events on outer layer", tagThreshold_);

	timeWindow_ = 3*I3Units::microsecond;
	AddParameter("TimeWindow", "Maximum length of the veto window", timeWindow_);

	badDOMListName_ = "BadDomsList";
	AddParameter("BadDOMList", "Name of the list of DOMs that do not produce data", badDOMListName_);

	geometryName_ = "I3Geometry";
	AddParameter("Geometry", "Name of geometry to use", geometryName_);

	outputName_ = "LayerVeto";
	AddParameter("Output", "Prefix for output quantities", outputName_);
}

void
LayerVeto::Configure()
{
	GetParameter("VetoLayers", layerConstructor_);
	if (!PyObject_HasAttrString(layerConstructor_.ptr(), "__call__"))
		log_fatal("VetoLayers must be a callable object!");
	
	GetParameter("Pulses", pulseName_);
	GetParameter("QMax", qmax_);
	GetParameter("QMin", qmin_);
	GetParameter("QScale", qscale_);
	GetParameter("TagThreshold", tagThreshold_);
	GetParameter("TimeWindow", timeWindow_);
	GetParameter("BadDOMList", badDOMListName_);
	GetParameter("Geometry", geometryName_);
	GetParameter("Output", outputName_);
}

void
LayerVeto::ConstructVetoLayers()
{
	// Provide the layer-constructing callback with a dictionary of GCD information
	// for the DOMs that are actually configured and turned on and not marked bad.
	python::dict domMap;
	BOOST_FOREACH(const I3OMGeoMap::value_type &pair, geometry_->omgeo) {
		if (badDOMList_.find(pair.first) != badDOMList_.end())
			continue;
		I3DOMCalibrationMap::const_iterator cal = calibration_->domCal.find(pair.first);
		I3DOMStatusMap::const_iterator stat = status_->domStatus.find(pair.first);
		if (cal == calibration_->domCal.end() || stat == status_->domStatus.end()
		    || !std::isfinite(stat->second.pmtHV) || stat->second.pmtHV < 500*I3Units::volt)
			continue;
		domMap[pair.first] = python::make_tuple(pair.second, cal->second, stat->second);
	}
	
	I3FramePtr vframe(new I3Frame('V'));
	vetoLayers_.clear();
	python::list pylayers = python::extract<python::list>(layerConstructor_(domMap));
	for (python::ssize_t i = 0; i < python::len(pylayers); i++) {
		std::vector<OMKey> keys = python::extract<std::vector<OMKey> >(pylayers[i]);
		vetoLayers_.push_back(std::set<OMKey>(keys.begin(), keys.end()));
		std::ostringstream label;
		label << outputName_ << i;
		vframe->Put(label.str(), I3VectorOMKeyPtr(new I3VectorOMKey(keys.begin(), keys.end())));
	}
	
	PushFrame(vframe);
}

void
LayerVeto::Geometry(I3FramePtr frame)
{
	geometry_ = frame->Get<I3GeometryConstPtr>(geometryName_);
	if (geometry_ && calibration_ && status_)
		ConstructVetoLayers();
	
	PushFrame(frame);
}

void
LayerVeto::Calibration(I3FramePtr frame)
{
	calibration_ = frame->Get<I3CalibrationConstPtr>();
	if (geometry_ && calibration_ && status_)
		ConstructVetoLayers();
	
	PushFrame(frame);
}

void
LayerVeto::DetectorStatus(I3FramePtr frame)
{
	status_ = frame->Get<I3DetectorStatusConstPtr>();
	if (I3VectorOMKeyConstPtr bdlist =
	    frame->Get<I3VectorOMKeyConstPtr>(badDOMListName_))
		badDOMList_ = std::set<OMKey>(bdlist->begin(), bdlist->end());
	if (geometry_ && calibration_ && status_)
		ConstructVetoLayers();
	
	PushFrame(frame);
}

double
TotalCharge(const I3RecoPulseSeriesMap &pmap)
{
	double qtot = 0.;
	BOOST_FOREACH(const I3RecoPulseSeriesMap::value_type &pair, pmap)
		BOOST_FOREACH(const I3RecoPulse &p, pair.second)
			qtot += p.GetCharge();
	
	return qtot;
}

I3RecoPulseSeriesMapConstPtr
RemovePulses(I3FramePtr frame, const std::string &key, const I3RecoPulseSeriesMap &selection)
{
	I3RecoPulseSeriesMapMask selmask(*frame, key, selection);
	return I3RecoPulseSeriesMapMask(*frame, key).Remove(selmask).Apply(*frame);
}

void
AddPulses(I3FramePtr frame, const std::string &key, I3RecoPulseSeriesMap &lhs, const I3RecoPulseSeriesMap &rhs)
{
	I3RecoPulseSeriesMapMask lmask(*frame, key, lhs);
	I3RecoPulseSeriesMapMask rmask(*frame, key, rhs);
	lhs = *((lmask|rmask).Apply(*frame));
}

void
LayerVeto::Physics(I3FramePtr frame)
{
	I3RecoPulseSeriesMapConstPtr pulses =
	    frame->Get<I3RecoPulseSeriesMapConstPtr>(pulseName_);
	if (!pulses) {
		log_warn("No pulses named '%s' in the frame!", pulseName_.c_str());
		PushFrame(frame);
		return;
	}
	
	std::vector<I3RecoPulseSeriesMap> vetoPulses(vetoLayers_.size());
	std::vector<double> vetoCharge(vetoLayers_.size());
	
	double vertexTime = 0;
	double qtot = GetVetoCharge(*pulses, vetoPulses, vertexTime);
	std::transform(vetoPulses.begin(), vetoPulses.end(), vetoCharge.begin(), &TotalCharge);
	
	// The pulse masks put in the frame should be not just the pulses that
	// tripped the veto, but the complete set that could have. Apply the
	// veto repeatedly, stripping off any pulses that trip the veto each
	// time.
	if (vetoCharge[0] > tagThreshold_) {
		double dummyTime;
		std::vector<I3RecoPulseSeriesMap> subVetoPulses(vetoLayers_.size());
		
		int iter = 0;
		do {
			BOOST_FOREACH(I3RecoPulseSeriesMap &pmap, subVetoPulses)
				pmap.clear();
			
			I3RecoPulseSeriesMapConstPtr stripped =
			    RemovePulses(frame, pulseName_, vetoPulses[0]);
			log_trace_stream(iter << ": removed " << TotalCharge(vetoPulses[0]) << " of " << TotalCharge(*pulses) << " PE");
			GetVetoCharge(*stripped, subVetoPulses, dummyTime);
			AddPulses(frame, pulseName_, vetoPulses[0], subVetoPulses[0]);
			iter++;
		} while (iter < 10000 && TotalCharge(subVetoPulses[0]) > tagThreshold_);
		if (iter > 1)
			log_debug("Stripped out all veto pulses after %d iterations", iter);
	}

	// Lazy output mode: one double for each layer
	{
		std::vector<double>::const_iterator q = vetoCharge.begin();
		std::vector<I3RecoPulseSeriesMap>::const_iterator pulses = vetoPulses.begin();
		for (int i=0; q < vetoCharge.end(); i++, q++, pulses++) {
			{
				std::ostringstream label;
				label << outputName_ << i;
				frame->Put(label.str(), boost::make_shared<I3Double>(*q));
			}
			{
				std::ostringstream label;
				label << outputName_ << "Pulses" << i;
				frame->Put(label.str(), boost::make_shared<I3RecoPulseSeriesMapMask>(*frame, pulseName_, *pulses));
			}
		}
	}
	frame->Put(outputName_ + "QTot", boost::make_shared<I3Double>(qtot));
	frame->Put(outputName_ + "VertexTime", boost::make_shared<I3Double>(vertexTime));
	
	PushFrame(frame);
}

namespace {

struct DOMCharge {
	I3RecoPulseSeriesMap::const_iterator pos;
	double charge;
	DOMCharge(I3RecoPulseSeriesMap::const_iterator it)
	    : pos(it), charge(0.)
	{
		BOOST_FOREACH(const I3RecoPulse &p, pos->second)
			charge += p.GetCharge();
	}
	bool operator<(const DOMCharge &other) const
	{
		return other.charge < charge;
	}
};

struct Hit {
	unsigned index;
	OMKey key;
	const I3Position &pos;
	const I3RecoPulse &pulse;
	Hit(const OMKey &k, const I3Position &ompos, const I3RecoPulse &p, unsigned i) :
	    index(i), key(k), pos(ompos), pulse(p)
	{}
	double GetTime() const
	{
		return pulse.GetTime();
	}
	double GetCharge() const
	{
		return pulse.GetCharge();
	}
	const I3Position& GetPos() const
	{
		return pos;
	}
	bool operator<(const Hit &other) const
	{
		return GetTime() < other.GetTime();
	}
};

}

double
LayerVeto::GetVetoCharge(const I3RecoPulseSeriesMap &pulseMap,
    std::vector<I3RecoPulseSeriesMap> &vetoCharge, double &vertexTime)
{
	// Find total charge in all non-DeepCore DOMs
	std::list<DOMCharge> domCharge;
	double qtot = 0;
	for (I3RecoPulseSeriesMap::const_iterator it = pulseMap.begin(); it != pulseMap.end(); it++) {
		DOMCharge q(it);
		domCharge.push_back(q);
		qtot += q.charge;
	}
	
	// Sort DOMs in descending order of charge, and remove any
	// that contribute more than 90% to the total charge. Add the
	// pulses from DOMs that do contribute to a time-ordered stream
	// of pulses.
	domCharge.sort();
	std::list<Hit> hitStream;
	for (std::list<DOMCharge>::iterator it = domCharge.begin(); it != domCharge.end(); it++)
		if (it->charge/qtot > 0.9)
			qtot -= it->charge;
		else {
			I3Map<OMKey, I3OMGeo>::const_iterator geo =
			    geometry_->omgeo.find(it->pos->first);
			if (geo == geometry_->omgeo.end())
				log_fatal_stream(it->pos->first <<
				    " is not in the geometry!");
			unsigned idx = 0;
			BOOST_FOREACH(const I3RecoPulse &p, it->pos->second) {
				hitStream.push_back(Hit(it->pos->first,
				    geo->second.position, p, idx++));
			}
		}
	
	// Bail for extremely small events
	if (hitStream.size() == 0) {
		return 0.;
	}
	
	hitStream.sort();
	
	// Define the charge threshold for detection
	double qmax = std::min(qmax_, std::max(qmin_, qtot/qscale_));
	
	std::list<Hit>::const_iterator start(hitStream.begin()), stop(boost::next(start));
	// Slide a timeWindow_-wide window through the hit stream
	// until it contains qmax charge
	double qwindow = start->GetCharge();
	I3Position vertexPos = qwindow*start->GetPos();
	vertexTime = start->GetTime();
	
	for ( ; (qwindow < qmax) && (stop != hitStream.end()); stop++) {
		
		qwindow += stop->GetCharge();
		vertexPos += stop->GetCharge()*stop->GetPos();
		vertexTime = stop->GetTime();
		
		// Strip off hits that have fallen off the beginning of the
		// time window
		while ((start != hitStream.end()) && stop->GetTime() - start->GetTime() > timeWindow_) {
			qwindow -= start->GetCharge();
			vertexPos -= start->GetCharge()*start->GetPos();
			start++;
		}
		if (start == hitStream.end())
			break;
		
	}
	vertexPos /= qwindow;
	
	// Peel off the pulses in the veto region during the veto window
	std::vector<DOMSet>::const_iterator layer = vetoLayers_.begin();
	std::vector<I3RecoPulseSeriesMap>::iterator layerPulses = vetoCharge.begin();
	for ( ; layer != vetoLayers_.end(); layer++, layerPulses++) {
		for (std::list<Hit>::const_iterator it = start; it != stop; it++) {
			if ((it->GetPos()-vertexPos).Magnitude() > timeWindow_*I3Constants::c)
				continue;
			if (layer->find(it->key) != layer->end())
				(*layerPulses)[it->key].push_back(it->pulse);
		}
	}
	
	return qtot;
}
