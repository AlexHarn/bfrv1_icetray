/**
 * Copyright (c) 2011, 2012
 * Claudio Kopper <claudio.kopper@icecube.wisc.edu>
 * and the IceCube Collaboration <http://www.icecube.wisc.edu>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 *
 * $Id$
 *
 * @file I3Photon.cxx
 * @version $Revision$
 * @date $Date$
 * @author Claudio Kopper
 */

#include <icetray/serialization.h>
#include <simclasses/I3Photon.h>
#include <boost/foreach.hpp>

I3Photon::~I3Photon() { }

void I3Photon::SetParticleID(const I3Particle& p) { 
    particleID_ = p.GetMinorID(); 
    particleMajorID_ = p.GetMajorID();
}

I3ParticleID I3Photon::GetParticleID() const {
    I3ParticleID partID;
    partID.majorID = particleMajorID_;
    partID.minorID = particleID_;
    return partID;
}

template <class Archive>
void I3Photon::serialize (Archive &ar, unsigned version)
{
    if (version > i3photon_version_)
        log_fatal("Attempting to read version %u from file but running version %u of I3Photon class.",version,i3photon_version_);
    
    ar & make_nvp("time", time_);
    ar & make_nvp("weight", weight_);
    ar & make_nvp("ID",ID_);
    ar & make_nvp("particleID", particleID_);
    ar & make_nvp("particleMajorID", particleMajorID_);
    ar & make_nvp("cherenkovDist", cherenkovDist_);
    ar & make_nvp("wavelength", wavelength_);
    ar & make_nvp("groupVelocity", groupVelocity_);
    ar & make_nvp("dir", direction_);
    ar & make_nvp("pos", position_);
    ar & make_nvp("startTime", startTime_);
    ar & make_nvp("startDir", startDirection_);
    ar & make_nvp("startPos", startPosition_);
    ar & make_nvp("numScattered", numScattered_);
    if (version >= 2) {
        ar & make_nvp("distanceInAbsorptionLengths", distanceInAbsorptionLengths_);
    }
    
    if (version == 1) {
        std::vector<I3Position> intermediatePositionsOrig;
        ar & make_nvp("intermediatePositions", intermediatePositionsOrig);
        intermediatePositions_.clear();
        BOOST_FOREACH(const I3Position &pos, intermediatePositionsOrig)
        {
            intermediatePositions_.push_back(std::make_pair(pos, NAN));
        }
    } else if (version >= 2) {
        ar & make_nvp("intermediatePositions", intermediatePositions_);
    }
        
    
}     

I3_SERIALIZABLE(I3Photon);
I3_SERIALIZABLE(I3PhotonSeriesMap);

std::ostream& I3Photon::Print(std::ostream& os) const{
    os << "[I3CompressedPhoton: \n"
     << "             Time: " << time_ << '\n'
     << "           Weight: " << weight_ << '\n'
     << "  Cherenkov Dist.: " << cherenkovDist_ << '\n'
     << "       Wavelength: " << wavelength_ << '\n'
     << "       Group Vel.: " << groupVelocity_ << '\n'
     << "        Direction: " << direction_ << '\n'
     << "         Position: " << position_ << '\n'
     << "       Start Time: " << startTime_ << '\n'
     << "       Start Dir.: " << startDirection_ << '\n'
     << "       Start Pos.: " << startPosition_ << '\n'
     << "         Scatters: " << numScattered_ << '\n'
     << "     Abs. Lengths: " << distanceInAbsorptionLengths_ << '\n'
     << "               ID: " << ID_ << '\n'
     << "       ParticleID: " << particleMajorID_ << ',' << particleID_ << ")]";
  return os;
}

std::ostream& operator<<(std::ostream& os, const I3Photon& p){
  return(p.Print(os));
}
