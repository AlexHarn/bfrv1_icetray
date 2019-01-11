/** $Id: Track.cxx 128654 2015-02-04 18:34:51Z jvansanten $
 * @file
 * @author Jakob van Santen <vansanten@wisc.edu>
 *
 * $Revision: 128654 $
 * $Date: 2015-02-04 19:34:51 +0100 (Wed, 04 Feb 2015) $
 */

#include <ddddr/MuonGunTrack.h>
#include <simclasses/I3MMCTrack.h>

#include <boost/foreach.hpp>

namespace I3MuonGun {

Track::Track(const I3MMCTrack &mmctrack,
    const I3MCTree::sibling_const_iterator &sbegin,
    const I3MCTree::sibling_const_iterator &send) : I3Particle(mmctrack.GetI3Particle())
{
	// In the beginning, the particle is at its vertex with the given energy
	checkpoints_.push_back(Checkpoint(0., I3Particle::GetEnergy(), 0u));
	losses_.push_back(LossSum(checkpoints_.back().length, 0.));
	if (mmctrack.GetEi() > 0) {
		// Track started outside the MMC volume; we get an extra
		// measurement point (but no stochastics)
		double d = (I3Position(mmctrack.GetXi(), mmctrack.GetYi(),
		    mmctrack.GetZi())-I3Particle::GetPos()).Magnitude();
		checkpoints_.push_back(Checkpoint(d, mmctrack.GetEi(), losses_.size()));
	}
	
	// Sum energy losses between entry and exit
	double elost = 0;
	BOOST_FOREACH(const I3Particle &p, std::make_pair(sbegin, send)) {
		if (p.GetShape() == I3Particle::Dark)
			continue;
		elost += p.GetEnergy();
		double d = (p.GetPos()-I3Particle::GetPos()).Magnitude();
		losses_.push_back(LossSum(d, elost));
	}
	
	if (mmctrack.GetEf() > 0) {
		// Track made it to the edge of the MMC volume
		losses_.push_back(LossSum(checkpoints_.back().length, elost));
		double d = (I3Position(mmctrack.GetXf(), mmctrack.GetYf(),
		    mmctrack.GetZf())-I3Particle::GetPos()).Magnitude();
		checkpoints_.push_back(Checkpoint(d, mmctrack.GetEf(), losses_.size()));
		elost = 0.;
	}
	
	losses_.push_back(LossSum(I3Particle::GetLength(), elost));
	checkpoints_.push_back(Checkpoint(I3Particle::GetLength(), 0., losses_.size()));
}

namespace {

template <typename T>
inline bool Sort(const T &a, const T &b)
{
	return a.length < b.length;
}

}

double
Track::GetEnergy(double length) const
{
	if (!std::isfinite(length) || length >= GetLength())
		return 0.;
	else if (length <= 0)
		return GetEnergy();
	// Find an energy checkpoint. The above if() guarantees that
	// that both cp and cp+1 are valid.
	std::vector<Checkpoint>::const_iterator cp =
	    std::max(std::lower_bound(checkpoints_.begin(), checkpoints_.end(),
	    Checkpoint(length), Sort<Checkpoint>)-1, checkpoints_.begin());
	// Store iterators the mark the records of stochastic losses
	// between the checkpoints
	std::vector<LossSum>::const_iterator l1(losses_.begin()+(cp->offset > 0 ? cp->offset-1 : 0)),
	    l2(losses_.begin()+(cp+1)->offset);
	// Find the cumulative energy loss since the last checkpoint
	std::vector<LossSum>::const_iterator ls = std::max(std::lower_bound(l1, l2,
	    LossSum(length), Sort<LossSum>)-1, l1);
	
	// Estimate continuous loss rate
	double conti_rate = (cp->energy - (cp+1)->energy - (l2-1)->energy)
	    /((cp+1)->length - cp->length);
	
	return cp->energy - ls->energy - conti_rate*(length-cp->length);
}

I3Position
Track::GetPos(double length) const
{
	if (!std::isfinite(length) || length < 0 || length >= GetLength())
		return I3Position(NAN, NAN, NAN);
	const I3Position &pos = I3Particle::GetPos();
	const I3Direction &dir = I3Particle::GetDir();
	return I3Position(pos.GetX() + length*dir.GetX(),
	                  pos.GetY() + length*dir.GetY(),
	                  pos.GetZ() + length*dir.GetZ());
}

double
Track::GetTime(double length) const
{
	if (!std::isfinite(length) || length < 0 || length >= GetLength())
		return NAN;
	else
		return I3Particle::GetTime() + length/I3Particle::GetSpeed();
}

namespace {

inline bool
operator!=(const I3MMCTrack &track, const I3Particle &p)
{
	return (track.GetI3Particle().GetMajorID() != p.GetMajorID() ||
	    track.GetI3Particle().GetMinorID() != p.GetMinorID());
}

/**
 * Ensure that the MMCTrack has the same time reference as
 * the associated I3MCTree.
 */
inline I3MMCTrack
TimeShift(const I3Particle &p, const I3MMCTrack mmctrack)
{
	I3MMCTrack shifted(mmctrack);
	double d = (p.GetPos()-I3Position(mmctrack.GetXi(), mmctrack.GetYi(),
	    mmctrack.GetZi())).Magnitude();
	double dt = p.GetTime() + d/p.GetSpeed() - mmctrack.GetTi();
	shifted.SetEnter( mmctrack.GetXi(), mmctrack.GetYi(), mmctrack.GetZi(), mmctrack.GetTi() + dt, mmctrack.GetEi());
	shifted.SetCenter(mmctrack.GetXc(), mmctrack.GetYc(), mmctrack.GetZc(), mmctrack.GetTc() + dt, mmctrack.GetEc());
	shifted.SetExit(  mmctrack.GetXf(), mmctrack.GetYf(), mmctrack.GetZf(), mmctrack.GetTf() + dt, mmctrack.GetEf());
	shifted.GetParticle().SetTime(p.GetTime());
	
	return shifted;
}

}

std::list<Track>
Track::Harvest(const I3MCTree &mctree, const I3MMCTrackList &mmctracks)
{
    std::list<Track> tracks;
    I3MCTree::const_iterator p = mctree.end();
    I3MCTree::sibling_const_iterator daughters = mctree.end_sibling();
    BOOST_FOREACH(const I3MMCTrack &mmctrack, mmctracks) {
        // For each track, find the particle it corresponds to
        p = mctree.find(mmctrack.GetI3Particle());
        if (p != mctree.end()) {
            // Get energy checkpoints from the MMCTrack and stochastic losses
            // from the direct daughters of the corresponding MCTree node
            daughters = mctree.children(p);
            // Make sure we have daughters first
            if (daughters != mctree.end_sibling())
                // Timeshift the particle the track corresponds to, then make a track with that
                // timeshifted particle and its daughters
                tracks.push_back(Track(TimeShift(*p, mmctrack), daughters, mctree.end_sibling() ) );
        }
    }
	
	return tracks;
}

};
