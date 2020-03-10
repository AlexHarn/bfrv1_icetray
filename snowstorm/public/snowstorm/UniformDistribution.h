
#include <vector>
#include <boost/numeric/ublas/triangular.hpp>
#include <dataclasses/I3Matrix.h>
#include "snowstorm/Distribution.h"

namespace snowstorm {

class UniformDistribution : public Distribution {
public:
    UniformDistribution(const std::vector<std::pair<double, double>>& limits) : limits_(limits){};
    virtual ~UniformDistribution();

    std::vector<double> Sample(I3RandomService&) const override; 
    double LogPDF(const std::vector<double>&) const override;
    size_t size() const override { return limits_.size(); };
private:
    UniformDistribution();

    friend class icecube::serialization::access;
    template <class Archive> void serialize(Archive & ar, unsigned version);
    
    std::vector<std::pair<double, double>> limits_;
};

I3_POINTER_TYPEDEFS(UniformDistribution);

}

I3_CLASS_VERSION(snowstorm::UniformDistribution, 0);
