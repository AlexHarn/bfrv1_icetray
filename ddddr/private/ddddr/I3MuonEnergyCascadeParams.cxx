/**
 * @file private/ddddr/I3MuonEnergyCascadeParams.cxx
 * @brief Implementation of the I3MuonEnergyCascadeParams class
 *
 */
#include <icetray/serialization.h>
#include "ddddr/I3MuonEnergyCascadeParams.h"

std::string I3MuonEnergyCascadeParams::Dump()
{
	std::stringstream s;
	s << "nDOMsCascade: " << nDOMsCascade << std::endl;
	s << "cascade_energy: " << cascade_energy << " GeV" << std::endl;
	s << "cascade_energy_sigma: " << cascade_energy_sigma << " GeV" << std::endl;
	s << "cascade_position: " << cascade_position << std::endl;
	s << "cascade_slant_depth: " << cascade_slant_depth << " m" << std::endl;

	return s.str();
}

template <class Archive>
void I3MuonEnergyCascadeParams::serialize(Archive & ar, unsigned version)
{
	ar & make_nvp("I3MuonEnergyCascadeParams", base_object<I3FrameObject>(*this));
	ar & make_nvp("nDOMsCascade", nDOMsCascade);
	ar & make_nvp("cascade_energy", cascade_energy);
	ar & make_nvp("cascade_energy_sigma", cascade_energy_sigma);
	ar & make_nvp("cascade_position", cascade_position);
	ar & make_nvp("cascade_slant_depth", cascade_slant_depth);
}

I3_SERIALIZABLE(I3MuonEnergyCascadeParams);
