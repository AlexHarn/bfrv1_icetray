#ifndef _I3MUONENERGYFUNCTIONS_H_INCLUDED
#define _I3MUONENERGYFUNCTIONS_H_INCLUDED

#include <iostream>
#include <fstream>

#include "dataclasses/physics/I3Particle.h"
#include "simclasses/I3MMCTrack.h"

/**
 * @brief Contains set of static helper functions used in 
 * I3MuonEnergy and I3TrueMuonEnergy.
 */
namespace MuonEnergyFunctions
{
	/// Energy loss measured by a given DOM.
	double eLoss(double, double, double, double, double domEff = 1., 
			double levelDist = 25., double fscale = 0.02);

	/// Energy loss around a certain DOM.
	double eLossCascade(double, double, double, double, double domEff = 1., 
			double levelDist = 25., double fscale = 0.02);

	/// Read attenuation parameter lambda for a given depth from a text stream.
	double getLambda(double, std::stringstream& );

	/// Read attenuation parameter lambda for a given depth from a table.
	double getLambda(double, std::vector<std::vector<double> >);

	/// Get slant depth of a particle at a given position.
	double getSlantDepth(I3Particle&, I3Position& , double top = 1950.);

	/// Get vertical depth corresponding to given slant depth along a 
	/// particle track.
	double getVerticalDepthZ(I3Particle&, double, double top = 1950.);

	double ln_poisson(double, double, double min_exp = 1e-30);

	std::vector<double> getBinEdges(I3MMCTrackListConstPtr&, double, double, double);

	I3ParticlePtr getWeightedPrimary(I3FramePtr&, std::string);
};

#endif
