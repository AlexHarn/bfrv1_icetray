/*
 * copyright  (C) 2012
 * Nathan Whitehorn, Claudio Kopper
 * The Icecube Collaboration: http://www.icecube.wisc.edu
 *
 * $Id$
 *
 * @version $Revision$
 * @date $LastChangedDate$
 * @author $LastChangedBy$
 */

#include <icetray/I3ConditionalModule.h>
#include <icetray/I3Bool.h>
#include <icetray/I3Units.h>
#include <dataclasses/geometry/I3Geometry.h>
#include <dataclasses/status/I3DetectorStatus.h>
#include <dataclasses/physics/I3RecoPulse.h>
#include <dataclasses/I3Double.h>

#include <boost/foreach.hpp>
#include <algorithm>

#include <VHESelfVeto/VHESelfVetoUtils.h>

class VHESelfVeto : public I3ConditionalModule {
	public:
		VHESelfVeto(const I3Context &);

		void Configure();
		void Physics(I3FramePtr frame);
		void Geometry(I3FramePtr frame);
		void DetectorStatus(I3FramePtr frame) { Geometry(frame); }

	private:
		bool IsPositionBad(const I3Position &position);

		double topBoundaryWidth_, bottomBoundaryWidth_;
		double dustLayer_, dustLayerWidth_;
		double timeWindow_;
		double vertexThreshold_;
		double vetoThreshold_;

		std::string pulses_;
		std::string outputBoolName_;
		std::string outputTimeName_;
		std::string outputPosName_;

		VHESelfVetoUtils::reducedgeo_t reducedgeo_;

		std::string geoName_;
		I3GeometryConstPtr geo_;
};

I3_MODULE(VHESelfVeto);

VHESelfVeto::VHESelfVeto(const I3Context &context)
    : I3ConditionalModule(context)
{
	AddParameter("TopBoundaryWidth", "Distance from the top and side "
	    "detector edge where there should be no hits.", 90.);
	AddParameter("BottomBoundaryWidth", "Distance from the bottom detector "
	    "edge where there should be no hits.", 10.);
	AddParameter("DustLayer", "Mid-detector layer that should contain no "
	    "hits.", -135);
	AddParameter("DustLayerWidth", "Mid-detector layer boundary width -- "
	    "only applies below the dust layer", 80.);
	AddParameter("TimeWindow", "Size of sliding time window to examine.",
	    3000 * I3Units::ns);
	AddParameter("VertexThreshold", "Number of PE in the sliding time "
	    "window that constitudes a vertex detection", 250.);
	AddParameter("VetoThreshold", "Maximum number of PE permitted within "
	    "the detector boundary before the vertex time", 3.);
	AddParameter("OutputBool", "Name of I3Bool containing the veto pass "
	    "decision", "VHESelfVeto");
	AddParameter("OutputVertexTime", "Name of I3Double containing the "
	    "vertex time guess", "VHESelfVetoVertexTime");
	AddParameter("OutputVertexPos", "Name of I3Position containing the "
	    "vertex position guess", "VHESelfVetoVertexPos");
	AddParameter("Pulses", "Name of pulse series to use", "OfflinePulses");
	AddParameter("Geometry", "Name of geometry object to use",
	    I3DefaultName<I3Geometry>::value());
}

void
VHESelfVeto::Configure()
{
	GetParameter("TopBoundaryWidth", topBoundaryWidth_);
	GetParameter("BottomBoundaryWidth", bottomBoundaryWidth_);
	GetParameter("DustLayer", dustLayer_);
	GetParameter("DustLayerWidth", dustLayerWidth_);
	GetParameter("TimeWindow", timeWindow_);
	GetParameter("VertexThreshold", vertexThreshold_);
	GetParameter("VetoThreshold", vetoThreshold_);
	GetParameter("OutputBool", outputBoolName_);
	GetParameter("OutputVertexTime", outputTimeName_);
	GetParameter("OutputVertexPos", outputPosName_);
	GetParameter("Pulses", pulses_);
	GetParameter("Geometry", geoName_);
}

void
VHESelfVeto::Geometry(I3FramePtr frame)
{
	geo_ = frame->Get<I3GeometryConstPtr>(geoName_);
	
	if (!geo_)
		log_fatal("No I3Geometry object found with name \"%s\"",
		    geoName_.c_str());
	
	I3DetectorStatusConstPtr status =
	    frame->Get<I3DetectorStatusConstPtr>();

	reducedgeo_ = VHESelfVetoUtils::FindReducedGeometry(*geo_, status,
	    topBoundaryWidth_, bottomBoundaryWidth_, topBoundaryWidth_);

	PushFrame(frame);
}

bool VHESelfVeto::IsPositionBad(const I3Position &pos)
{
	// Is it in the vetoed dust region?
	if (pos.GetZ() < dustLayer_ && pos.GetZ() >
	    dustLayer_ - dustLayerWidth_)
		return true;

	// Positions that are not inside the reduced geometry are bad
	if (!VHESelfVetoUtils::IsInReducedGeometry(pos, reducedgeo_))
		return true;
	
	return false;
}

typedef boost::tuple<OMKey, I3Position, I3RecoPulse>
    OMKeyPulsePair;
typedef std::vector<OMKeyPulsePair> OMKeyPulseVector;

static bool
operator <(const OMKeyPulsePair &a, const OMKeyPulsePair &b)
{
	return a.get<2>().GetTime() < b.get<2>().GetTime();
}

void
VHESelfVeto::Physics(I3FramePtr frame)
{
	I3RecoPulseSeriesMapConstPtr pulses =
	    frame->Get<I3RecoPulseSeriesMapConstPtr>(pulses_);
	if (!pulses) {
		PushFrame(frame);
		return;
	}
	if (!geo_) {
		log_fatal("No Geometry frame seen yet.");
	}
	OMKeyPulseVector pulse_list;
	std::vector<OMKey> balloon_doms;
	double qtot = 0;

	for (I3RecoPulseSeriesMap::const_iterator i = pulses->begin();
	    i != pulses->end(); i++) {
		BOOST_FOREACH(const I3RecoPulse &pulse, i->second)
			qtot += pulse.GetCharge();
	}
	
	for (I3RecoPulseSeriesMap::const_iterator i = pulses->begin();
	    i != pulses->end(); i++) {
		double charge_this_dom = 0;
		BOOST_FOREACH(const I3RecoPulse &pulse, i->second) {
			I3OMGeoMap::const_iterator geo =
			    geo_->omgeo.find(i->first);
			if (geo == geo_->omgeo.end())
				log_fatal("OM (%d, %d) not in geometry!",
				    i->first.GetString(), i->first.GetOM());
			pulse_list.push_back(OMKeyPulsePair(i->first,
			    geo->second.position, pulse));
			charge_this_dom += pulse.GetCharge();
		}
		if (charge_this_dom/qtot > 0.9)
			balloon_doms.push_back(i->first);
	}

	std::sort(pulse_list.begin(), pulse_list.end());

	double vertexTime = NAN;
	I3PositionPtr outputVertexPosition(new I3Position());
	bool vetoed = false;
	for (OMKeyPulseVector::const_iterator i = pulse_list.begin(), 
	    j = pulse_list.begin(); j != pulse_list.end(); j++) {
		while (j->get<2>().GetTime() - i->get<2>().GetTime() > 
		    timeWindow_) i++;

		double charge = 0, bad_charge = 0;
		int bad_multiplicity = 0;
		I3Position vertex_position(0, 0, 0);
		for (OMKeyPulseVector::const_iterator k = i; k != j+1; k++) {
			// Do not use balloon DOMs for vertex finding
			if (std::find(balloon_doms.begin(), balloon_doms.end(),
			    k->get<0>()) != balloon_doms.end())
				continue;

			double q = k->get<2>().GetCharge();
			charge += q;
			vertex_position.SetX(vertex_position.GetX() +
			    q*k->get<1>().GetX());
			vertex_position.SetY(vertex_position.GetY() +
			    q*k->get<1>().GetY());
			vertex_position.SetZ(vertex_position.GetZ() +
			    q*k->get<1>().GetZ());
		}
		vertex_position.SetX(vertex_position.GetX()/charge);
		vertex_position.SetY(vertex_position.GetY()/charge);
		vertex_position.SetZ(vertex_position.GetZ()/charge);

		if (charge > vertexThreshold_) {
			vertexTime = j->get<2>().GetTime();
			// Check time window (i-j) for charge in veto region
			for (OMKeyPulseVector::const_iterator k = i;
			    k != pulse_list.end(); k++) {
				// Allow a margin of slightly less than the
				// causal margin (17 m/c = 56 ns). Photons
				// arriving before this can only be from outside
				// the fiducial volume.
				if (k->get<2>().GetTime() - vertexTime > 50.)
					break;
	
				// Check for causal connectivity within the
				// time window.
				if ((vertex_position-k->get<1>()).Magnitude()
				    > I3Constants::c*timeWindow_)
					continue;

				if (IsPositionBad(k->get<1>())) {
					bad_charge += k->get<2>().GetCharge();
					bad_multiplicity++;
				}
			}

			if (bad_charge > vetoThreshold_ ||
			    bad_multiplicity >= vetoThreshold_)
				vetoed = true;
			
			*outputVertexPosition = vertex_position;
			break;
		}
	}

	if (std::isfinite(vertexTime)) {
		frame->Put(outputTimeName_,
		    I3DoublePtr(new I3Double(vertexTime)));
		frame->Put(outputPosName_,
		    outputVertexPosition);
		frame->Put(outputBoolName_,
		    I3BoolPtr(new I3Bool(vetoed)));
	}

	PushFrame(frame);
}