/**
 * @file private/ddddr/I3MuonEnergyParams.cxx
 * @brief Implementation of the I3MuonEnergyParams class
 *
 */
#include <icetray/serialization.h>
#include "ddddr/I3MuonEnergyParams.h"

std::string I3MuonEnergyParams::Dump()
{
	std::stringstream s;
	s << "N: " << N << std::endl;
	s << "N_err: " << N_err << std::endl;
	s << "b: " << b << std::endl;
	s << "b_err: " << b_err << std::endl;
	s << "gamma: " << gamma << std::endl;
	s << "nDOMs: " << nDOMs << std::endl;
	s << "rllh: " << rllh << std::endl;
	s << "chi2: " << chi2 << std::endl;
	s << "chi2ndof: " << chi2ndof << std::endl;
	s << "peak_energy: " << peak_energy << " GeV" << std::endl;
	s << "peak_sigma: " << peak_sigma << " GeV" << std::endl;
	s << "mean: " << mean << " GeV" << std::endl;
	s << "median: " << median << " GeV" << std::endl;
	s << "peak/median: " << peak_energy/median << std::endl;
	s << "bin_width: " << bin_width << " m" << std::endl;
	s << "status: " << status << std::endl;

	return s.str();
}

std::ostream& operator<<(std::ostream &s, const I3MuonEnergyParams &params) {
	s << "N: " << params.N << std::endl;
	s << "N_err: " << params.N_err << std::endl;
	s << "b: " << params.b << std::endl;
	s << "b_err: " << params.b_err << std::endl;
	s << "gamma: " << params.gamma << std::endl;
	s << "nDOMs: " << params.nDOMs << std::endl;
	s << "rllh: " << params.rllh << std::endl;
	s << "chi2: " << params.chi2 << std::endl;
	s << "chi2ndof: " << params.chi2ndof << std::endl;
	s << "peak_energy: " << params.peak_energy << std::endl;
	s << "peak_sigma: " << params.peak_sigma << std::endl;
	s << "mean: " << params.mean << std::endl;
	s << "median: " << params.median << std::endl;
	s << "bin_width: " << params.bin_width << std::endl;
	s << "status: " << params.status << std::endl;

	return s;
}

template <class Archive>
void I3MuonEnergyParams::serialize(Archive & ar, unsigned version)
{
	ar & make_nvp("I3MuonEnergyParams", base_object<I3FrameObject>(*this));
	ar & make_nvp("N", N);
	ar & make_nvp("N_err", N_err);
	ar & make_nvp("b", b);
	ar & make_nvp("b_err", b_err);
	ar & make_nvp("gamma", gamma);
	ar & make_nvp("gamma_err", gamma_err);
	ar & make_nvp("nDOMs", nDOMs);
	ar & make_nvp("rllh", rllh);
	ar & make_nvp("chi2", chi2);
	ar & make_nvp("chi2ndof", chi2ndof);
	ar & make_nvp("peak_energy", peak_energy);
	ar & make_nvp("peak_sigma", peak_sigma);
	ar & make_nvp("mean", mean);
	ar & make_nvp("median", median);
	ar & make_nvp("bin_width", bin_width);
	ar & make_nvp("status", status);
}

I3_SERIALIZABLE(I3MuonEnergyParams);
