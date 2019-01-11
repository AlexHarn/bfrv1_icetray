/**
 *  Implementation of I3MuonEnergy (DDDDR)
 *
 *  @file I3TrueMuonEnergy.h
 *  @date $Date: 14/01/23 
 *  @author Hans-Peter Bretz (hbretz@icecube.wisc.edu) 
 *  @author Patrick Berghaus
 */
#ifndef _I3TRUEMUONENERGY_H_INCLUDED
#define _I3TRUEMUONENERGY_H_INCLUDED

#include "icetray/I3ConditionalModule.h"
#include "dataclasses/physics/I3Particle.h"
#include "dataclasses/geometry/I3Geometry.h"
#include "dataclasses/physics/I3MCTree.h"
#include "dataclasses/physics/I3MCTreeUtils.h"
#include "simclasses/I3MMCTrack.h"

#include <icetray/I3Logging.h>
#include <string>
#include <map>
#include <vector>
#include <algorithm>

#include "ddddr/I3MuonEnergyParams.h"
#include "ddddr/I3MuonEnergyCascadeParams.h"
#include "ddddr/MuonEnergyMinuit2.h"
#include "ddddr/MuonEnergyFCNBase.h"
#include "ddddr/FitParameterSpecs.h"
#include "ddddr/ExpoFcn.h"

/**
 * @brief Struct that contains the energy of a muon bundle at a certain position
 * along its track.
 */
struct BundleBin { double depth, slant, energy, energyloss; };

/**
 * @brief This module calculates the total energy of a muon bundle by summing up
 * the contribution of each muon for each bin along the track of the whole bundle. 
 * 
 * It also performs a fit of an exponential function to either the distribution of the 
 * total energy or the energy loss.
 */
class I3TrueMuonEnergy : public I3ConditionalModule
{
	private:
		/// The I3MCTree containing the air shower.
		std::string mcTreeName_;

		/// The MMCTrackList containing the muon bundle.
		std::string mmcName_;
		
		/// The bin width along the track.
		double slantBinWidth_;

		/// Number of bins to use.
		int slantBinNo_;

		/// If true, the energy loss is used for the fit and saved to the frame.
		bool energyLoss_;

		/// If true, save the energy losses as vectors in the frame
		bool saveEnergyLosses_;

		/// Max radius of the cylinder around the detector. Muons outside of it will
		/// not be considered.
		double maxRadius_;

		/// Prefix for the objects to be stored in the frame
		std::string framePrefix_;

		double minuitTolerance_;
		unsigned int minuitMaxIterations_;
		int minuitPrintLevel_;
		int minuitStrategy_;
		std::string minuitAlgorithm_;

		/// Fits an exponential function to the energy (loss) distribution, using
		/// MuonEnergyMinuit2.
		I3MuonEnergyParamsPtr getBundleEnergyDistribution(std::vector<BundleBin>&);
		I3MuonEnergyCascadeParamsPtr getCascadeEnergyParams(I3ParticleConstPtr&, 
				std::vector<BundleBin>);
		double getMedianOfVector(std::vector<double>);

	public:
		I3TrueMuonEnergy(const I3Context &);
		~I3TrueMuonEnergy();
		void Configure();
		void Physics(I3FramePtr);

		SET_LOGGER("I3TrueMuonEnergy");
};

#endif
