#ifndef MUONL3_IC86_MUONL3_IC86_UTILS_H_INCLUDED
#define MUONL3_IC86_MUONL3_IC86_UTILS_H_INCLUDED

#include <dataclasses/I3TimeWindow.h>

namespace MuonL3_IC86_Utils {
	enum OverlapType { BEFORE, WITHIN, AFTER, NONE, CONTAINS};
	OverlapType GetOverlapType(const I3TimeWindow &first, const I3TimeWindow &second);
	bool contains(const I3TimeWindow& w, double t);
}

#endif // MUONL3_IC86_MUONL3_IC86_UTILS_H_INCLUDED
