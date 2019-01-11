/**
 * @file I3MuonEnergyParams.h
 * @brief Definition of the I3MuonEnergyParams class
 *
 */
#ifndef _I3MUONENERGYPARAMS_H_INCLUDED
#define _I3MUONENERGYPARAMS_H_INCLUDED

#include <icetray/I3FrameObject.h>
#include "dataclasses/Utility.h"
#include "dataclasses/physics/I3Particle.h"


/**
 * @brief Class to save the results of a fit of the energy loss distribution
 * to an I3Frame.
 */
class I3MuonEnergyParams : public I3FrameObject
{
	public:
		/// Normalization constant of Exp. or TomF function.
		double N, N_err;

		/// Slope of Exp. or TomF function.
		double b, b_err;

		/// Exponent for TomF function.
		double gamma, gamma_err;

		/// Number of DOMs used to determine energy distribution.
		double nDOMs;

		/// Reduced log likelihood of the fit.
		double rllh;

		/// Chi square of the fit.
		double chi2;

		/// Chi square per degree of freedom of the fit.
		double chi2ndof;

		/// Peak energy of the energy loss distribution.
		double peak_energy;

		/// Uncertainty of the peak energy.
		double peak_sigma;

		/// Mean of the energy loss distribution.
		double mean;

		/// Median of the energy loss distribution.
		double median;

		/// Bin width of the energy loss distribution.
		double bin_width;

		/// Status of the fit
		I3Particle::FitStatus status;

		/// Method for printing the Params
		std::string Dump();

		I3MuonEnergyParams() :
			N(NAN), N_err(NAN),
			b(NAN), b_err(NAN),
			gamma(NAN), gamma_err(NAN),
			nDOMs(NAN), rllh(NAN),
			chi2(NAN), chi2ndof(NAN),
			peak_energy(NAN), peak_sigma(NAN),
			mean(NAN), median(NAN), bin_width(NAN), 
            status(I3Particle::NotSet)	{}

		virtual ~I3MuonEnergyParams() {}

	private:
		private:
		friend std::ostream& operator<<(std::ostream&, const I3MuonEnergyParams&);
		friend class icecube::serialization::access;
		  template <class Archive> void serialize(Archive& ar, unsigned version);
};

I3_POINTER_TYPEDEFS(I3MuonEnergyParams);

#endif
