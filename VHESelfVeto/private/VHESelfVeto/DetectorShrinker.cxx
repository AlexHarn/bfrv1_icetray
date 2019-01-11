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

#include <icetray/I3PacketModule.h>
#include <icetray/I3Bool.h>
#include <icetray/I3Units.h>
#include <dataclasses/geometry/I3Geometry.h>
#include <dataclasses/status/I3DetectorStatus.h>
#include <dataclasses/physics/I3RecoPulse.h>
#include <dataclasses/I3MapOMKeyMask.h>
#include <dataclasses/I3Double.h>

#include <boost/foreach.hpp>
#include <algorithm>

#include <VHESelfVeto/VHESelfVetoUtils.h>

class DetectorShrinker : public I3PacketModule {
	public:
		DetectorShrinker(const I3Context &);

		void Configure();
		void FramePacket(std::vector<I3FramePtr> &frames);
		void Physics(I3FramePtr frame);

	private:
		double topBoundaryWidth_, bottomBoundaryWidth_;
		//  double dustLayer_, dustLayerWidth_;
		I3GeometryConstPtr currentGeo_;

		std::string pulses_, outpulses_, outgeo_, ingeo_;
};

I3_MODULE(DetectorShrinker);

DetectorShrinker::DetectorShrinker(const I3Context &context)
    : I3PacketModule(context, I3Frame::Geometry)
{
	AddParameter("TopBoundaryWidth", "Distance from the top and side "
	    "detector edge where there should be no hits.", 90.);
	AddParameter("BottomBoundaryWidth", "Distance from the bottom detector "
	    "edge where there should be no hits.", 10.);
	AddParameter("Pulses", "Name of pulse series to use", "OfflinePulses");
	AddParameter("OutPulses", "Name of trimmed pulse series to write out",
	    "OfflinePulsesTrimmed");
	AddParameter("InGeometry", "Name of original geometry to read",
			"I3Geometry");
	AddParameter("OutGeometry", "Name of trimmed geometry to write out",
	    "I3GeometryTrimmed");

	packet_types.push_back(I3Frame::Geometry);
	packet_types.push_back(I3Frame::Calibration);
	packet_types.push_back(I3Frame::DetectorStatus);

	AddOutBox("OutBox");
}

void
DetectorShrinker::Configure()
{
	GetParameter("TopBoundaryWidth", topBoundaryWidth_);
	GetParameter("BottomBoundaryWidth", bottomBoundaryWidth_);
	GetParameter("Pulses", pulses_);
	GetParameter("OutPulses", outpulses_);
  GetParameter("InGeometry", ingeo_);
	GetParameter("OutGeometry", outgeo_);
}

void
DetectorShrinker::FramePacket(std::vector<I3FramePtr> &frames)
{
	I3FramePtr gframe, dframe;
	BOOST_FOREACH(I3FramePtr frame, frames) {
		if (frame->GetStop() == I3Frame::Geometry)
			gframe = frame;
		if (frame->GetStop() == I3Frame::DetectorStatus)
			dframe = frame;
	}

	I3GeometryConstPtr geo = gframe->Get<I3GeometryConstPtr>(ingeo_);
	I3DetectorStatusConstPtr status =
	    dframe->Get<I3DetectorStatusConstPtr>();

	VHESelfVetoUtils::reducedgeo_t reducedgeo =
	    VHESelfVetoUtils::FindReducedGeometry(*geo, status,
	    topBoundaryWidth_, bottomBoundaryWidth_, topBoundaryWidth_);

	currentGeo_ = VHESelfVetoUtils::TrimI3Geometry(*geo, reducedgeo);
	gframe->Put(outgeo_, currentGeo_);

	BOOST_FOREACH(I3FramePtr frame, frames)
		PushFrame(frame);
}

void
DetectorShrinker::Physics(I3FramePtr frame)
{
	I3RecoPulseSeriesMapConstPtr pulses =
	    frame->Get<I3RecoPulseSeriesMapConstPtr>(pulses_);
	if (!pulses) {
		PushFrame(frame);
		return;
	}

	I3RecoPulseSeriesMapMaskPtr only_in_geo(new
	    I3RecoPulseSeriesMapMask(*frame, pulses_));

	for (I3RecoPulseSeriesMap::const_iterator i = pulses->begin();
	    i != pulses->end(); i++) {
		if (currentGeo_->omgeo.find(i->first) ==
		    currentGeo_->omgeo.end())
			only_in_geo->Set(i->first, false);
	}

	frame->Put(outpulses_, only_in_geo);

	PushFrame(frame);
}
