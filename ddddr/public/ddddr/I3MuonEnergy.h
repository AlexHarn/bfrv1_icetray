/**
 *  Implementation of I3MuonEnergy (DDDDR)
 *
 *  @file I3MuonEnergy.h
 *  @date $Date: 14/01/23 
 *  @author Hans-Peter Bretz (hbretz@icecube.wisc.edu) 
 *  @author Patrick Berghaus
 */
#ifndef _I3MUONENERGY_H_INCLUDED
#define _I3MUONENERGY_H_INCLUDED

#include "icetray/I3ConditionalModule.h"
#include "dataclasses/physics/I3Particle.h"
#include "dataclasses/geometry/I3Geometry.h"
#include "dataclasses/calibration/I3Calibration.h"
#include "dataclasses/calibration/I3DOMCalibration.h"
#include <icetray/serialization.h>

#include <iostream>
#include <fstream>
#include <cstdlib>

#include <icetray/I3Logging.h>
#include <string>
#include <map>
#include <vector>
#include <algorithm>

#include "ddddr/I3MuonEnergyProfile.h"
#include "ddddr/I3MuonEnergyParams.h"
#include "ddddr/I3MuonEnergyCascadeParams.h"

////// Fitting related classes
#include "ddddr/MuonEnergyMinuit2.h"
#include "ddddr/MuonEnergyFCNBase.h"
#include "ddddr/FitParameterSpecs.h"
#include "ddddr/TomFFcn.h"
#include "ddddr/ExpoFcn.h"
//////

/**
 * @class I3MuonEnergy
 * @brief This module estimates and fits the energy loss along a given track.
 *
 * The module is the implementation of the Data-Derived Differential Deposition Reconstruction
 * (DDDDR) algorithm, described in detail in the IceCube wiki at 
 * https://wiki.icecube.wisc.edu/index.php/IC79_Atmospheric_Muons/DDDDR. It takes any kind of 
 * reconstructed track as input and estimates the energy loss around the track up to a specified 
 * impact parameter, using a table of depth-dependent values for the light attenuation coefficient lambda.
 * The estimated (binned) energy loss for the individual DOMs along the track can be saved optionally, 
 * together with the result of the fit to the energy loss.
 *
 * The result of the fit is saved together with some properties of the energy loss distribution
 * in an object of the type I3MuonEnergyParams. More about the track parameters at 
 * https://wiki.icecube.wisc.edu/index.php/Muon_Bundle_Suppression#Stochasticity.
 *
 * The parameters I3MuonEnergyParams#outputDOMs_ and I3MuonEnergyParams#saveDomResults_ can be used to save the full energy loss distribution
 * used for the fit. OutputDOMsName+Slant/Impact/Depth are vectors with the slant depth, distance to the 
 * track or depth of each individual DOM while OutputDOMsName+(e)dEdX contain the (errors on the) 
 * energy losses. The OutputDOMsNamebinned vectors contain the same information for the binned
 * energy loss distribution. In this case, the depth corresponds to the bincenters of the slant depth bins.
 */
class I3MuonEnergy : public I3ConditionalModule
{
	private:
		/// input track for the reconstruction.
		std::string trackName_;

		/// Pulses to use for the reconstruction.
		std::string inIcePulseName_;

		/// Name of the file containing the attenuation length of the ice.
		std::string iceFileName_;

		/// the table containing the lambda table.
		std::vector<std::vector<double> > lambdaFile_;

		/// Only needed if I3MuonEnergy#useMonteCarloTrack_ is true.
		std::string mcTreeName_;

		/// Name of the MMCTracklist frame object. Only needed if 
		/// I3MuonEnergy#useMonteCarloTrack_ is true.
		std::string mmcName_;

		/// Name prefix for the frame objects containing the DOM data.
		std::string outputDOMs_;

		/// Name for the I3MuonEnergyParams frame object.
		std::string outputPars_;

		/// Name for the I3MuonEnergyCascadeParams frame object.
		std::string outputCascadePars_;

		/// Prefix for all frame objects
		std::string framePrefix_;

		/// Coefficient resp. for flattening of the exponential function used to
		/// estimate the energy loss for a DOM.
		double levelDist_;

		/// Either exponential function or Tom Feusels's function to
		/// fit the energy loss.
		Minuit2FitFCN fitFunction_;

		int fitFunctionInt_;

		/// fix parameter b in fit with Tom Feusels's function
		bool fixB_;

		/// fix parameter gamma in fit with Tom Feusels's function
		bool fixGamma_;

		/// Save energy loss, impact factor, depth and slant depth both
		/// per DOM and binned as vectors to the frame.
		bool saveDomResults_;

		/// If set to true, I3MuonEnergy#trackName_ is ignored.
		bool useMonteCarloTrack_;

		/// Number of bins to divide the track into. Only needed if 
		/// I3MuonEnergy#useMonteCarloTrack_ is true.
		int slantBinNo_;

		/// Decrease in the atten. length at the top
		double purityAdjust_;

		/// Max. distance between DOM and track.
		double maxImpact_;

		/// Bin width of slant depth along the track. Only needed if 
		/// module is used with reconstructed track as input.
		double fitBinWidth_;

		/// Number of degrees of freedom for the fit
		int nDoF_;
		double minuitTolerance_;
		unsigned int minuitMaxIterations_;
		int minuitPrintLevel_;
		int minuitStrategy_;
		I3GeometryConstPtr geometry_;
		I3OMGeoMapPtr omGeo_;
		I3DOMCalibrationMapPtr domCal_;
		std::string badDomListName_;
		I3VectorOMKeyConstPtr badDomList_;
		std::string minuitAlgorithm_;
		int tracksFitted_;
		double surface_height_;

		
		/// Determines the track for the energy loss estimation, either reconstructed
		/// track specified in I3MuonEnergy#trackName_ or true Monte Carlo track.
		I3ParticlePtr getSeedTrack(I3FramePtr&);

		/// Determines bins for the energy distribution through the DOMs along the 
		/// track or the length of the Monte Carlo track and fills it with the 
		/// estimated energy losses.
		boost::shared_ptr<I3MuonEnergyProfile> getMuonEnergyProfile(I3FramePtr&, double, double, 
				std::vector<DOMDATA>&);

		/// Creates parameter for the chosen fit function.
		std::vector<FitParameterSpecs> getFitParameterSpecs(Minuit2FitFCN&);

		/// Saves results from fit into an object of type I3MuonEnergyParams.
		I3MuonEnergyParamsPtr getMuonEnergyParams(boost::shared_ptr<I3MuonEnergyProfile>, 
				std::vector<double>, std::vector<double>);

		I3ParticlePtr getWeightedPrimary(I3FramePtr&);

		std::vector<std::vector<double> > readLambdaFile(std::string);

		/// Saves the binned energy profile and the individual DOMs considered
		/// for the profile in the frame.
		void saveDomResultsEnergyProfile(boost::shared_ptr<I3MuonEnergyProfile> , I3FramePtr& , 
				boost::shared_ptr<I3Vector<double> > , boost::shared_ptr<I3Vector<double> >, 
				boost::shared_ptr<I3Vector<double> >, I3Particle&);

		/// Saves the results from the cascade energy reconstruction into an
		/// object of type I3MuonEnergyCascadeParams
		I3MuonEnergyCascadeParamsPtr getCascadeEnergyParams(boost::shared_ptr<I3Vector<double> >,
				I3Position, I3Particle&);

	public:
  
		I3MuonEnergy(const I3Context &);
		~I3MuonEnergy();
		void Configure();
		void Geometry(I3FramePtr);
		void Calibration(I3FramePtr);
		void DetectorStatus(I3FramePtr);
		void Physics(I3FramePtr);
		void Finish();

		SET_LOGGER("I3MuonEnergy");
};

#endif
