
#include <vector>
#include <boost/numeric/ublas/triangular.hpp>
#include <dataclasses/I3Matrix.h>
#include "snowstorm/Distribution.h"

namespace snowstorm {

class MultivariateNormal : public Distribution {
public:
    MultivariateNormal(const I3Matrix &covariance, const std::vector<double> &mean);
    virtual ~MultivariateNormal();

    virtual std::vector<double> Sample(I3RandomService&) const override;
    virtual double LogPDF(const std::vector<double>&) const override;
    virtual size_t size() const override { return mean_.size(); };
private:
    MultivariateNormal();

    friend class icecube::serialization::access;
    template <class Archive> void serialize(Archive & ar, unsigned version);
    
    // lower triangle of Cholesky decomposition of covariance matrix
    I3Matrix covarianceL_;
    std::vector<double> mean_;
};

I3_POINTER_TYPEDEFS(MultivariateNormal);

}

I3_CLASS_VERSION(snowstorm::MultivariateNormal, 0);