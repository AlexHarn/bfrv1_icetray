#include "ddddr/I3MuonEnergyParams.h"

using namespace boost::python;

void registerI3MuonEnergyParams()
{
	class_<I3MuonEnergyParams, bases<I3FrameObject>,
		I3MuonEnergyParamsPtr>("I3MuonEnergyParams")
			.def_readwrite("N", &I3MuonEnergyParams::N)
			.def_readwrite("N_err", &I3MuonEnergyParams::N_err)
			.def_readwrite("b", &I3MuonEnergyParams::b)
			.def_readwrite("b_err", &I3MuonEnergyParams::b_err)
			.def_readwrite("gamma", &I3MuonEnergyParams::gamma)
			.def_readwrite("gamma_err", &I3MuonEnergyParams::gamma_err)
			.def_readwrite("nDOMs", &I3MuonEnergyParams::nDOMs)
			.def_readwrite("rllh", &I3MuonEnergyParams::rllh)
			.def_readwrite("chi2", &I3MuonEnergyParams::chi2)
			.def_readwrite("chi2ndof", &I3MuonEnergyParams::chi2ndof)
			.def_readwrite("peak_energy", &I3MuonEnergyParams::peak_energy)
			.def_readwrite("peak_sigma", &I3MuonEnergyParams::peak_sigma)
			.def_readwrite("mean", &I3MuonEnergyParams::mean)
			.def_readwrite("median", &I3MuonEnergyParams::median)
			.def_readwrite("bin_width", &I3MuonEnergyParams::bin_width)
			.def_readwrite("status", &I3MuonEnergyParams::status)
			.def("__str__", &I3MuonEnergyParams::Dump)
			;

	register_pointer_conversions<I3MuonEnergyParams>();

}

