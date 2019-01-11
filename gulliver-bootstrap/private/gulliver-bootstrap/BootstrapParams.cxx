#include <gulliver-bootstrap/BootstrapParams.h>

BootstrapParams::BootstrapParams():
status(NoValidFits),successfulFits(0),totalFits(0){}

BootstrapParams::BootstrapParams(ResultStatus status, uint32_t successfulFits, uint32_t totalFits):
status(status),successfulFits(successfulFits),totalFits(totalFits){}

BootstrapParams::~BootstrapParams(){}

template <class Archive>
void BootstrapParams::serialize(Archive& ar, unsigned version){
	ar & make_nvp("I3FrameObject", base_object<I3FrameObject>(*this));
	ar & make_nvp("status", status);
	ar & make_nvp("successfulFits", successfulFits);
	ar & make_nvp("totalFits", totalFits);
}

I3_SERIALIZABLE(BootstrapParams);
