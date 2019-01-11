/**
 * @file I3MuonEnergyCascadeParams.h
 * @brief Definition of the I3MuonEnergyCascadeParams class
 *
 */
#ifndef _I3MUONENERGYCASCADEPARAMS_H_INCLUDED
#define _I3MUONENERGYCASCADEPARAMS_H_INCLUDED

#include <icetray/I3FrameObject.h>
#include "dataclasses/Utility.h"
#include "dataclasses/physics/I3Particle.h"


/**
 * @brief Class to save the results of a fit of the energy loss distribution
 * to an I3Frame.
 */
class I3MuonEnergyCascadeParams : public I3FrameObject
{
	public:
		/// Number of DOMs used to determine the cascade energy.
		double nDOMsCascade;

		/// Reconstructed cascade energy.
		double cascade_energy;

		/// Statistical error on the reconstructed cascade energy, 
		/// calculated from the standard deviation of the distribution of the 
		/// reconstructed energies for all DOMs used for the cascade energy
		/// reconstruction.
		double cascade_energy_sigma;

		/// Position of the leading casascade on the track.
		I3Position cascade_position;

		/// Slant depth of the leading cascade.
		double cascade_slant_depth;

		/// Method for printing the Params
		std::string Dump();

		I3MuonEnergyCascadeParams() :
			nDOMsCascade(NAN), cascade_energy(NAN),
			cascade_energy_sigma(NAN),
			cascade_position(I3Position()),
			cascade_slant_depth(NAN) {}

		virtual ~I3MuonEnergyCascadeParams() {}

	private:
		friend class icecube::serialization::access;
		  template <class Archive> void serialize(Archive& ar, unsigned version);
};

I3_POINTER_TYPEDEFS(I3MuonEnergyCascadeParams);

#endif
