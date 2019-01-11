/**
 *  Definition of I3MuonEnergyProfile (DDDDR)
 *
 *  @file I3MuonEnergyProfile.h
 *  @date $Date: 14/01/23 
 *  @author Hans-Peter Bretz (hbretz@icecube.wisc.edu) 
 */
#ifndef _I3MUONENERGYPROFILE_H_INCLUDED
#define _I3MUONENERGYPROFILE_H_INCLUDED

#include "dataclasses/I3Vector.h"
#include <icetray/I3Logging.h>

/**
 * @brief Struct that holds all data of a certain DOM needed for the energy
 * loss distribution.
 */
struct DOMDATA { double charge, time, depth, slant, lambda, impact, dEdX, x, y, z, eff; };

/**
 * @brief A class used to create a binned energy loss distribution from the 
 * individual energy losses of all DOMs.
 *
 * I3MuonEnergyProfile is a reimplementation of some basic features of ROOT's
 * TProfile class. Each bin holds the average value and statistical error of 
 * all data points in this bin. The binning is determined by the bin width and
 * the min. and max. values for the slant. Instead of the limits of slant depth, data 
 * points of slant depth and energy loss can be given to the constructor to
 * determine the limits automatically.
 */
class I3MuonEnergyProfile
{

	private:
		/// Sum of all entries per bin.
		std::vector<double> binContent_;

		/// Sum of the squares of the entries for each bin.
		std::vector<double> binContent2_;

		/// Number of entries per bin.
		std::vector<int> binEntries_;

		/// Bin edges.
		std::vector<double> binEdges_;

		/// Type of error to be returned when calling I3MuonEnergyProfile#GetBinError.
		std::string errors_;

		/// Number of bins.
		double nBins_;

		/// Total number of bin entries.
		int nBinEntries_;

		/// Bin width.
		double binWidth_;
		
		/// Total sum of all bin entries.
		double sumWeightsy_;

		/// Upper limit for slant depth.
		double xmax_;

		/// Lower limit for slant depth.
		double xmin_;

		/// Number of DOMs that had energy loss
		double nEnergyLosses_;

		void AddBinContent(int, double);
		double medianOfVector(std::vector<double>);
		void createEmptyBins(int);
		void createEmptyBins(std::vector<double>);

	public:
		std::vector<double> GetBins();
		I3MuonEnergyProfile(std::vector<DOMDATA>, double, 
				const std::string errors = "default");
		I3MuonEnergyProfile(std::vector<double>, std::vector<double>, 
				double, const std::string errors = "default");
		I3MuonEnergyProfile(double, double, double, 
				const std::string errors = "default");
		I3MuonEnergyProfile(std::vector<double>, double, 
				const std::string errors = "default");
		~I3MuonEnergyProfile();
		
		/// Fill a pair of slant depth and energy loss in the histogram.
		int Fill(double, double);

		/// Find the bin that contains the input energy loss.
		int FindBin(double);

		/// Get position of given bin center.
		double GetBinCenter(int);

		/// Get vector of all Bin Centers.
		std::vector<double> GetBinCenters();

		/// Get content of given bin.
		double GetBinContent(int);

		/// Get vector of all Bin Contents.
		std::vector<double> GetBinContents();

		/// Get error of given bin.
		double GetBinError(int);

		/// Get vector of all bin edges. 
		std::vector<double> GetBinEdges();

		/// Get vector of bin entries.
		std::vector<int> GetBinEntries();

		/// Get bin width.
		double GetBinWidth();

		/// Get bin with max. energy loss.
		int GetMaxBin();

		/// Get the mean energy loss.
		double GetMeanEnergyLoss();

		/// Get median energy loss.
		double GetMedianEnergyLoss();
		int GetNBins();
		int GetNNonZeroBins();
		int GetNEntries();
		int GetNEnergyLosses();
		double GetXMin();

		/// Higher edge of the last bin.
		double GetXMax();
};
#endif
