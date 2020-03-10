#include <icetray/I3Logging.h>
#include <phys-services/I3RandomService.h>
#include "snowstorm/UniformDistribution.h"

namespace snowstorm {

std::vector<double>
UniformDistribution::Sample(I3RandomService &rng) const
{
    // draw samples from a uniform distribution
    std::vector<double> x(limits_.size());
    for (unsigned i=0; i < limits_.size(); i++)
        x[i] = rng.Uniform(limits_[i].first, limits_[i].second);

    return x;
}

double
UniformDistribution::LogPDF(const std::vector<double> &x) const
{
    i3_assert(x.size() == limits_.size());
    
    double result = 1;
    for (unsigned i=0; i < limits_.size(); i++){
        if(x[i] > limits_[i].first and x[i] < limits_[i].second){
            result *= 1.0/(limits_[i].second - limits_[i].first);
        }
        else{
            return -std::numeric_limits<double>::infinity();
        }
    }
    return std::log(result);
}


template <class Archive>
void UniformDistribution::serialize(Archive & ar, unsigned version)
{
    if (version > 0)
        log_fatal_stream("Version "<<version<<" is from the future");

    ar & make_nvp("snowstorm::Distribution", base_object<Distribution>(*this));
    ar & make_nvp("limits", limits_);
}

UniformDistribution::UniformDistribution() {}
UniformDistribution::~UniformDistribution() {}

}

I3_SERIALIZABLE(snowstorm::UniformDistribution);
