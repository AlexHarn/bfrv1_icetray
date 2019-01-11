#include "ddddr/I3MuonEnergyProfile.h"

using namespace boost::python;

void registerI3MuonEnergyProfile()
{
	class_<I3MuonEnergyProfile>("I3MuonEnergyProfile", 
			init<std::vector<double>, std::vector<double>, double>())
		.def(init<double, double, double>())
		.def("Fill", &I3MuonEnergyProfile::Fill)
		.def("FindBin", &I3MuonEnergyProfile::FindBin)
		.def("GetBinCenter", &I3MuonEnergyProfile::GetBinCenter)
		.def("GetBinContent", &I3MuonEnergyProfile::GetBinContent)
		.def("GetBinError", &I3MuonEnergyProfile::GetBinError)
		.def("GetMeanEnergyLoss", &I3MuonEnergyProfile::GetMeanEnergyLoss)
		.def("GetMedianEnergyLoss", &I3MuonEnergyProfile::GetMedianEnergyLoss)
		.def("GetMaxBin", &I3MuonEnergyProfile::GetMaxBin)

#define RO_PROPERTIES (BinCenters)(BinContents)(BinWidth)(BinEdges)(BinEntries)(NBins)(NEntries)(XMin)(XMax)
		BOOST_PP_SEQ_FOR_EACH(WRAP_PROP_RO, I3MuonEnergyProfile, RO_PROPERTIES)
#undef RO_PROPERTIES

		;
}
