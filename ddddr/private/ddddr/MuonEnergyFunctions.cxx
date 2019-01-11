#include "dataclasses/I3Map.h"
#include "dataclasses/physics/I3RecoPulse.h"
#include "ddddr/I3MuonEnergy.h"
#include "ddddr/MuonEnergyFunctions.h"
#include "phys-services/I3Calculator.h"

#include "dataclasses/physics/I3MCTree.h"
#include "dataclasses/physics/I3MCTreeUtils.h"

#include <iostream>
#include <fstream>

using namespace std;

/**
 *
 * @param charge	Charge measured by DOM
 * @param impact	Impact parameter of DOM relative to track
 * @param lambda	Attenuation parameter lambda at depth of the DOM
 * @param levelDist	Flattening parameter of the energy loss function	
 *
 * @return Energy loss, s. https://wiki.icecube.wisc.edu/index.php/IC79_Atmospheric_Muons/DDDDR#Fit_Results
 */
double MuonEnergyFunctions::eLoss(double charge, double impact, double lambda, 
		double z, double domEff, double levelDist, double fscale)
{
	//if (impact>levelDist)
	//	return charge*impact*exp((impact-levelDist)/lambda);
	//else
	//	return charge*levelDist;
	// The above is not really the same as in Patrick's paper
	double dnull = levelDist + 0.01 * z;
	double eloss = charge * fscale / domEff;
	if (impact < dnull)
		eloss *= dnull * 1.;//exp(dnull/lambda);
	else
		eloss *= impact * exp((impact - dnull)/lambda);

	return eloss;
	//if (eloss == 0) 
	//{
	//	eloss = 0.1;
	//}
	//return log10(eloss);
}

double MuonEnergyFunctions::eLossCascade(double charge, double distance, double lambda,
		double z, double domEff, double levelDist, double fscale)
{
	double rnull = levelDist + 0.01 * z;
	double eloss = charge * fscale / domEff;
	if (distance < rnull)
		eloss *= rnull * distance * 1.;// exp(rnull/lambda);
	else
		eloss *= distance * distance * exp((distance - rnull)/lambda);
	return eloss;
	//if (eloss == 0) 
	//{
	//	eloss = 0.1;
	//}
	//return log10(eloss);
}

/**
 * @param depth		Depth in the ice
 * @param fin		String stream of the input file with the lambda table
 *
 * @return			Ice attenuation parameter lambda
 */
double MuonEnergyFunctions::getLambda(double depth, stringstream & fin)
{
	//fin.clear();
	fin.seekg(0, ios::beg);
	std::string line;
	double zmax, zmin, l;
	while (getline(fin, line))
	{
		if (line[0] == '#') continue;
		istringstream line_stream(line);
		line_stream >> zmax >> zmin >> l;

		if (depth>=zmin && depth<zmax)
		{
			log_warn("Found Lambda: depth %f, zmax %f, zmin %f, l %f", depth, zmax, zmin, l);
			return l;
		}
	}

	log_warn("Did Not Find Lambda: depth %f, zmax %f, zmin %f, l %f", depth, zmax, zmin, l);
	return 0;

}

/**
 * @param depth			Depth in the ice
 * @param lambdaFile	Vector of table rows (vectors of depth and lamdba) representing lambda vs. depth
 *
 * @return				Ice attenuation parameter lambda
 */
double MuonEnergyFunctions::getLambda(double depth, std::vector<std::vector<double> > lambdaFile)
{
	int zmaxi = 0;
	int zmini = 1;
	int li = 2;

	for (std::vector<std::vector<double> >::size_type i = 0; i < lambdaFile.size(); i++)
	{
		if (depth >= lambdaFile[i][zmini] && depth < lambdaFile[i][zmaxi])
		{
			return lambdaFile[i][li];
		}
	}

	return 0.;
}

/**
 * Slant depth of the muon (bundle) as seen from the position of a DOM at a given position.
 *
 * @param track	Muon (bundle) track
 * @param po	Position of the DOM
 * @param top	Z position of the ice surface
 *
 * @return		Slant depth along the track at the closest approach position of the muon
 *				(bundle) relative to the position of the DOM
 */
double MuonEnergyFunctions::getSlantDepth(I3Particle & track, I3Position & po, double top) 
{
	I3Position closest_app_pos = I3Calculator::ClosestApproachPosition(track, po);
	double vetical_depth = top - closest_app_pos.GetZ();
	return vetical_depth/cos(track.GetZenith());
}

/**
 * @param track		Muon (bundle) track
 * @param slant		Sland depth along the track
 * @param top		Z position of the ice surface
 *
 * @return			Vertical depth in z
 */
double MuonEnergyFunctions::getVerticalDepthZ(I3Particle & track, double slant, double top)
{
	double vertical_depth = slant * cos(track.GetZenith());
	return top - vertical_depth;
}

double MuonEnergyFunctions::ln_poisson(double meas, double exp, double min_exp) 
{
	if (exp<min_exp) 
		exp= min_exp;
	double poisson;
	if (meas>=1)
		poisson = meas*log(exp)-exp+meas-meas*log(meas);
	else
		poisson = -exp;

	return poisson;
}

/**
 * @brief Method that finds the primary particle of a simulated event. Code is taken 
 * from the weighting module 
 * http://code.icecube.wisc.edu/svn/projects/weighting/trunk/python/__init__.py
 *
 * @param frame The frame containing the event
 * @param mctree The I3MCTree containing the primary particle
 *
 * @return the weighted primary of the event
 */
I3ParticlePtr MuonEnergyFunctions::getWeightedPrimary(I3FramePtr &frame, std::string mcTreeName)
{
	I3MCTreeConstPtr mctree = frame->Get<I3MCTreeConstPtr>(mcTreeName);
	std::vector<I3Particle> primaries = I3MCTreeUtils::GetPrimaries(*mctree);
	int idx = 0;

	// first primary if there is only one
	if (primaries.size() == 1)
	{
		idx = 0;
	}
	// first neutrino primary if it's a neutrino simulation
    else if (frame->Has("I3MCWeightDict"))
	{
		for(std::vector<I3Particle>::size_type i=0; i < primaries.size(); i++)
		{
			if (primaries[i].IsNeutrino())
			{
				idx = i;
				break;
			}
		}
	}
	// CORSIKA event with coincident primaries. Take the one that corresponds to the 
	// primary energy calculated from the weight map
	else if (frame->Has("CorsikaWeightMap"))
	{
		I3MapStringDoubleConstPtr wmap = frame->Get<I3MapStringDoubleConstPtr>("CorsikaWeightMap");
		double prim_index = wmap->find("PrimarySpectralIndex")->second;
		double prim_weight = wmap->find("Weight")->second;
		double prim_e = pow(prim_weight, -1./prim_index);
		std::vector<double> prim_energies;
		for(std::vector<I3Particle>::size_type i=0; i < primaries.size(); i++)
		{
			prim_energies.push_back(std::abs(primaries[i].GetEnergy() - prim_e));
		}
		idx = std::min_element(prim_energies.begin(), prim_energies.end()) - prim_energies.begin();
	}
	I3ParticlePtr prim_particle(new I3Particle(primaries.at(idx)));
	return prim_particle;
}

std::vector<double> MuonEnergyFunctions::getBinEdges(I3MMCTrackListConstPtr &mmctracklist, double surface_height, 
		double prim_zen, double bin_width)
{
	double initial_depth = surface_height;
	double final_depth = 0.;
	for (I3MMCTrackList::const_iterator mmc_iter = mmctracklist->begin();
		 mmc_iter != mmctracklist->end(); ++mmc_iter)
	{
		double zi = mmc_iter->GetZi();
		if (zi < initial_depth)
			initial_depth = zi;
		double zf = mmc_iter->GetZf();
		if (zf < final_depth)
			final_depth = zf;
	}

	double final_slant = (surface_height-final_depth)/cos(prim_zen);

	double initial_slant = (surface_height-initial_depth)/cos(prim_zen);

	int slantbins = int((final_slant - initial_slant)/bin_width) + 1; 
	std::vector<double> bin_edges;
	for (int i = 0; i <= slantbins; i++)
	{
		double this_slant = initial_slant + i*bin_width;
		bin_edges.push_back(this_slant);
	}
	return bin_edges;
}
