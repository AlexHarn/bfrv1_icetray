#include <icetray/I3Logging.h>
#include "snowstorm/DeltaDistribution.h"

namespace snowstorm {

std::vector<double>
DeltaDistribution::Sample(I3RandomService &rng) const
{
    return x0_;
}

double
DeltaDistribution::LogPDF(const std::vector<double> &x) const
{
    i3_assert(x.size() == x0_.size());
    if(std::equal(x.begin(), x.end(), x0_.begin())){
        return 0;
    }
    else
    {
        return -std::numeric_limits<double>::infinity();
    }
}


template <class Archive>
void DeltaDistribution::serialize(Archive & ar, unsigned version)
{
    if (version > 0)
        log_fatal_stream("Version "<<version<<" is from the future");

    ar & make_nvp("snowstorm::Distribution", base_object<Distribution>(*this));
    ar & make_nvp("x0", x0_);
}

DeltaDistribution::DeltaDistribution() {}
DeltaDistribution::~DeltaDistribution() {}

}

I3_SERIALIZABLE(snowstorm::DeltaDistribution);
