
#include <vector>
#include <boost/numeric/ublas/triangular.hpp>
#include <dataclasses/I3Matrix.h>
#include "snowstorm/Distribution.h"

namespace snowstorm {

class DeltaDistribution : public Distribution {
public:
    DeltaDistribution(const std::vector<double>& x0) : x0_(x0){};
    virtual ~DeltaDistribution();

    std::vector<double> Sample(I3RandomService&) const override; 
    double LogPDF(const std::vector<double>&) const override;
    size_t size() const override { return x0_.size(); };
private:
    DeltaDistribution();

    friend class icecube::serialization::access;
    template <class Archive> void serialize(Archive & ar, unsigned version);
    
    std::vector<double> x0_;
};

I3_POINTER_TYPEDEFS(DeltaDistribution);

}

I3_CLASS_VERSION(snowstorm::DeltaDistribution, 0);
