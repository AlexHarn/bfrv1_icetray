/*
 * copyright  (C) 2014
 * Claudio Kopper
 * The Icecube Collaboration: http://www.icecube.wisc.edu
 *
 * $Id
 *
 * @version $Revision
 * @date $LastChangedDate
 * @author $LastChangedBy
 */

#include <icetray/I3PacketModule.h>
#include <icetray/I3Bool.h>
#include <icetray/I3Int.h>
#include <icetray/I3Units.h>
#include <dataclasses/geometry/I3Geometry.h>
#include <dataclasses/status/I3DetectorStatus.h>
#include <dataclasses/physics/I3RecoPulse.h>
#include <dataclasses/I3MapOMKeyMask.h>
#include <dataclasses/I3Double.h>

#include <boost/foreach.hpp>
#include <algorithm>

#include <VHESelfVeto/VHESelfVetoUtils.h>

// this will ignore DeepCore strings internally (see VHESelfVetoUtils) as default
// set flag IgnoreDeepCoreStrings to False to change behaviour 
class BadStringRemover : public I3PacketModule {
	public:
		BadStringRemover(const I3Context &);

		void Configure();
		void FramePacket(std::vector<I3FramePtr> &frames);
		void Physics(I3FramePtr frame);

	private:
		I3GeometryConstPtr currentGeo_;

		std::string badDOMs_, pulses_, outpulses_, outgeo_, nBadStrings_;
		double badDOMFraction_;
		bool ignoreDeepCore_;
};

I3_MODULE(BadStringRemover);

BadStringRemover::BadStringRemover(const I3Context &context)
    : I3PacketModule(context, I3Frame::Geometry)
{
  AddParameter("BadDOMList", "Name of bad DOM list to use",
			"BadDomsList");
	AddParameter("BadDOMFractionForBadString", "Fraction of bad DOMs on a "
			"string for that string to be flagged as bad", 0.3);
			
	AddParameter("Pulses", "Name of pulse series to use", "SplitOfflinePulses");
	AddParameter("OutPulses", "Name of trimmed pulse series to write out",
	    "SplitOfflinePulsesTrimmed");
	AddParameter("OutGeometry", "Name of trimmed geometry to write out",
	    "I3GeometryWithoutBadStrings");
	AddParameter("IgnoreDeepCoreStrings","flag to ignore DeepCore strings, defaulting " 
			"to true for original functionality", true);

  AddParameter("OutNBadStrings", "optional: I3Int with number of bad strings",
			"");

	packet_types.push_back(I3Frame::Geometry);
	packet_types.push_back(I3Frame::Calibration);
	packet_types.push_back(I3Frame::DetectorStatus);

	AddOutBox("OutBox");
}

void
BadStringRemover::Configure()
{
  GetParameter("BadDOMList", badDOMs_);
  GetParameter("BadDOMFractionForBadString", badDOMFraction_);

	GetParameter("Pulses", pulses_);
	GetParameter("OutPulses", outpulses_);
	GetParameter("OutGeometry", outgeo_);
	
	GetParameter("OutNBadStrings", nBadStrings_);
	
	GetParameter("IgnoreDeepCoreStrings", ignoreDeepCore_);
}

void
BadStringRemover::FramePacket(std::vector<I3FramePtr> &frames)
{
	I3FramePtr gframe, dframe;
	BOOST_FOREACH(I3FramePtr frame, frames) {
		if (frame->GetStop() == I3Frame::Geometry)
			gframe = frame;
		if (frame->GetStop() == I3Frame::DetectorStatus)
			dframe = frame;
	}

	I3GeometryConstPtr geo = gframe->Get<I3GeometryConstPtr>();
	if (!geo)
		log_fatal("no input geometry found");
	
	I3DetectorStatusConstPtr status =
	    dframe->Get<I3DetectorStatusConstPtr>();

	I3VectorOMKeyConstPtr bdlist = dframe->Get<I3VectorOMKeyConstPtr>(badDOMs_);
	if (!bdlist)
		log_fatal("no bad DOM list in D-frame");

	const std::set<int> badStrings = 
    VHESelfVetoUtils::FindMissingStrings(*bdlist, *geo, badDOMFraction_, ignoreDeepCore_);

	currentGeo_ = VHESelfVetoUtils::TrimI3Geometry(*geo, badStrings);
	gframe->Put(outgeo_, currentGeo_);

  if (nBadStrings_ != "")
		dframe->Put(nBadStrings_, I3IntConstPtr(new I3Int(badStrings.size())));

	BOOST_FOREACH(I3FramePtr frame, frames)
		PushFrame(frame);
}

void
BadStringRemover::Physics(I3FramePtr frame)
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
