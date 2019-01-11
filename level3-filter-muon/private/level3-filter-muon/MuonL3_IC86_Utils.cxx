#include <level3-filter-muon/MuonL3_IC86_Utils.h>

MuonL3_IC86_Utils::OverlapType
MuonL3_IC86_Utils::GetOverlapType(const I3TimeWindow &first, const I3TimeWindow &second){
	if (first.GetStart() > second.GetStart()){
		if (first.GetStart() > second.GetStop())
			return NONE;
		else if (first.GetStop() <= second.GetStop())
			return WITHIN; // first within second
		else
			return AFTER; // first after second
	} else {
		if (first.GetStop() < second.GetStart())
			return NONE;
		else if (first.GetStop() <= second.GetStop())
			return BEFORE; // first after second
		else
			return CONTAINS; // first containes second
	}
}

bool MuonL3_IC86_Utils::contains(const I3TimeWindow& w, double t){
	return (t>=w.GetStart() && t<=w.GetStop());
}

