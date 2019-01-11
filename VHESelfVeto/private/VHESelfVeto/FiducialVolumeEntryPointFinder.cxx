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
#include <dataclasses/I3Double.h>
#include <icetray/I3Units.h>
#include <dataclasses/geometry/I3Geometry.h>
#include <dataclasses/status/I3DetectorStatus.h>
#include <dataclasses/physics/I3MCTree.h>
#include <dataclasses/physics/I3MCTreeUtils.h>

#include <boost/foreach.hpp>
#include <algorithm>

#include "recclasses/I3ParticleIntersections.h"

#include "VHESelfVeto/VHESelfVetoUtils.h"

class FiducialVolumeEntryPointFinder : public I3ConditionalModule {
	public:
		FiducialVolumeEntryPointFinder(const I3Context &);

		void Configure();
		void DAQ(I3FramePtr frame);
		void Geometry(I3FramePtr frame);
		void DetectorStatus(I3FramePtr frame) { Geometry(frame); }

	private:
		double topBoundaryWidth_, bottomBoundaryWidth_, sideBoundaryWidth_;

		bool assumeMuonTracksAreInfinite_;

		std::string mcTreeName_;
		std::string particleIntersectionsName_;

		VHESelfVetoUtils::reducedgeo_t reducedgeoNoBorders_;

		std::string geoName_;
		I3GeometryConstPtr geo_;
};

I3_MODULE(FiducialVolumeEntryPointFinder);

FiducialVolumeEntryPointFinder::FiducialVolumeEntryPointFinder(const I3Context &context)
    : I3ConditionalModule(context)
{
	AddParameter("TopBoundaryWidth", "Distance from the top "
	    "detector edge where there should be no vertices.", 90.);
	AddParameter("BottomBoundaryWidth", "Distance from the bottom detector "
	    "edge where there should be no vertices.", 10.);
	AddParameter("SideBoundaryWidth", "Distance from the side "
	    "edge where there should be no vertices.", 70.);
	AddParameter("AssumeMuonTracksAreInfinite", "Assume muon tracks "
	    "have an infinite length. If this is false, then the real "
	    "starting poisition of the muon will be used", false);
	AddParameter("MCTreeName", "Name of I3MCTree containing Monte Carlo"
	    "truth information", "I3MCTree");
	AddParameter("ParticleIntersectionsName", "Name of the output vector "
	    "containing all intersecting particles", "IntersectingParticles");
	AddParameter("Geometry", "Name of geometry object to use",
	    I3DefaultName<I3Geometry>::value());
}

void
FiducialVolumeEntryPointFinder::Configure()
{
	GetParameter("TopBoundaryWidth", topBoundaryWidth_);
	GetParameter("BottomBoundaryWidth", bottomBoundaryWidth_);
	GetParameter("SideBoundaryWidth", sideBoundaryWidth_);
	GetParameter("AssumeMuonTracksAreInfinite", assumeMuonTracksAreInfinite_);
	GetParameter("MCTreeName", mcTreeName_);
	GetParameter("ParticleIntersectionsName", particleIntersectionsName_);
	GetParameter("Geometry", geoName_);
}

void
FiducialVolumeEntryPointFinder::Geometry(I3FramePtr frame)
{
	geo_ = frame->Get<I3GeometryConstPtr>(geoName_);
	I3DetectorStatusConstPtr status =
	    frame->Get<I3DetectorStatusConstPtr>();

	VHESelfVetoUtils::reducedgeo_t reducedgeoWithBorders =
	VHESelfVetoUtils::FindReducedGeometry(*geo_, status,
	    topBoundaryWidth_, bottomBoundaryWidth_, sideBoundaryWidth_);

	// remove top/bottom and side borders
	reducedgeoNoBorders_ =
	    VHESelfVetoUtils::RemoveBoundariesFromReducedGeometry(
	    reducedgeoWithBorders);

	PushFrame(frame);
}

void
FiducialVolumeEntryPointFinder::DAQ(I3FramePtr frame)
{
	I3MCTreeConstPtr mcTree =
	    frame->Get<I3MCTreeConstPtr>(mcTreeName_);
	if (!mcTree) {
		log_fatal("could not find I3MCTree in frame");
	}

	// prepare the output vector
	I3ParticleIntersectionsSeriesPtr output(
	    new I3ParticleIntersectionsSeries);

	for (I3MCTree::const_iterator it=mcTree->begin();
	     it!=mcTree->end(); ++it)
	{
		// find non-dark tracks
		if (!it->IsTrack()) continue;
		if (it->GetShape() == I3Particle::Dark) continue;
		if (it->GetLocationType() != I3Particle::InIce) continue;
		
		// skip invalid positions
		I3Position from = it->GetPos();
		if (std::isnan(from.GetX()) || std::isnan(from.GetY()) || std::isnan(from.GetZ()))
			continue;
		
		// skip particles without lengths
		double length = it->GetLength();
		if (std::isnan(length) || (length<=0.)) continue;
		
		// skip particles that start inside the volume
		if (VHESelfVetoUtils::IsInReducedGeometryPolygon(
		    from, reducedgeoNoBorders_))
		{
			if (!assumeMuonTracksAreInfinite_) {
				continue;
			} else {
				const double backTrackLength = 1000.*I3Units::m;
				
				from.SetX(from.GetX() - it->GetDir().GetX()*backTrackLength);
				from.SetY(from.GetY() - it->GetDir().GetY()*backTrackLength);
				from.SetZ(from.GetZ() - it->GetDir().GetZ()*backTrackLength);
				length += backTrackLength*2.; // also track forward
			}
		}

		// calculate the end position
		const I3Position to(
		    from.GetX() + it->GetDir().GetX()*length,
		    from.GetY() + it->GetDir().GetY()*length,
		    from.GetZ() + it->GetDir().GetZ()*length);
		
		// find all intersections
		std::vector<I3Position> intersections =
		VHESelfVetoUtils::FindIntersectionsWithReducedGeometryBoundary(
		    reducedgeoNoBorders_, from, to);
		
		// do nothing if there are no intersections
		if (intersections.empty()) continue;
		
		// save the particle and its first intersection
		
		output->push_back(I3ParticleIntersections());
		I3ParticleIntersections &new_entry = output->back();
		
		new_entry.SetParticle(*it);
		new_entry.SetIntersections(intersections);
	}
	
	frame->Put(particleIntersectionsName_, output);

	PushFrame(frame);
}
