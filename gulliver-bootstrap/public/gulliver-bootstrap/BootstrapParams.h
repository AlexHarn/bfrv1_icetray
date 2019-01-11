#ifndef GULLIVER_BOOTSTRAP_BOOTSTRAPPARAMS_H
#define GULLIVER_BOOTSTRAP_BOOTSTRAPPARAMS_H

#include <icetray/I3FrameObject.h>
#include <icetray/serialization.h>

class BootstrapParams : public I3FrameObject{
public:
	enum ResultStatus{
		///The estimate was computed as expected
		OK=0,
		///The requested containment level was inside all successful fits.
		///The resulting estimate will be an overestimate.
		Underflow=1,
		///The requested containment level was outside all successful fits.
		///The resulting estimate will be an underestimate.
		Overflow=2,
		///No fits were successful so no estimate could be produced
		NoValidFits=3
	};
	
	BootstrapParams();
	BootstrapParams(ResultStatus status, uint32_t successfulFits, uint32_t totalFits);
	~BootstrapParams();
	
	ResultStatus status;
	///The number of input fits which succeeded
	uint32_t successfulFits;
	///The total number of input fits which were run
	uint32_t totalFits;
	
protected:
    friend class icecube::serialization::access;
	
    template <class Archive>
    void serialize(Archive& ar, unsigned version);
};

#endif //GULLIVER_BOOTSTRAP_BOOTSTRAPPARAMS_H
