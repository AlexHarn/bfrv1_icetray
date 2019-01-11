#include "ddddr/I3MuonEnergyCascadeParams.h"

using namespace boost::python;

void registerI3MuonEnergyCascadeParams()
{
	class_<I3MuonEnergyCascadeParams, bases<I3FrameObject>,
		I3MuonEnergyCascadeParamsPtr>("I3MuonEnergyCascadeParams")
			.def_readwrite("nDOMsCascade", &I3MuonEnergyCascadeParams::nDOMsCascade)
			.def_readwrite("cascade_energy", &I3MuonEnergyCascadeParams::cascade_energy)
			.def_readwrite("cascade_energy_sigma", &I3MuonEnergyCascadeParams::cascade_energy_sigma)
			.def_readwrite("cascade_position", &I3MuonEnergyCascadeParams::cascade_position)
			.def_readwrite("cascade_slant_depth", &I3MuonEnergyCascadeParams::cascade_slant_depth)
			.def("__str__", &I3MuonEnergyCascadeParams::Dump)
			;

	register_pointer_conversions<I3MuonEnergyCascadeParams>();

}

