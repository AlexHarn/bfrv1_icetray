
#include <vector>
#include <icetray/serialization.h>
#include <icetray/I3PointerTypedefs.h>
#include <icetray/I3FrameObject.h>

I3_FORWARD_DECLARATION(I3RandomService);

namespace snowstorm {

class Distribution : public I3FrameObject {
public:
    virtual ~Distribution();

    /// Draw a sample from the distribution
    virtual std::vector<double> Sample(I3RandomService&) const = 0;
    /// Compute the log density at x
    virtual double LogPDF(const std::vector<double>&) const = 0;
    /// Get the number of dimensions
    virtual size_t size() const = 0;
private:
    friend class icecube::serialization::access;
    template <class Archive> void serialize(Archive & ar, unsigned version);
};

I3_POINTER_TYPEDEFS(Distribution);

}

I3_CLASS_VERSION(snowstorm::Distribution, 0);
