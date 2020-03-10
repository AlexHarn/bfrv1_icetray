
#include <icetray/I3Logging.h>
#include "snowstorm/Composite.h"

namespace snowstorm {

void
Composite::add(const DistributionPtr &element)
{
    i3_assert(element);
    elements_.push_back(element);
}

std::vector<double>
Composite::Sample(I3RandomService &rng) const
{
    std::vector<double> x;
    for (const auto &element: elements_) {
        auto sample = element->Sample(rng);
        std::copy(sample.begin(), sample.end(), std::back_inserter(x));
    }

    return x;
}

size_t
Composite::size() const
{
    size_t size = 0;
    for (const auto &element: elements_) {
        size += element->size();
    }
    return size;
}

double
Composite::LogPDF(const std::vector<double> &x) const
{
    i3_assert(x.size() == size());
    double result = 0;
    auto head = x.cbegin();
    for (const auto &element: elements_) {
        result += element->LogPDF(std::vector<double>(head, head+element->size()));
        head += element->size();
    }
    return result;
}

template <class Archive>
void Composite::serialize(Archive & ar, unsigned version)
{
    if (version > 0)
        log_fatal_stream("Version "<<version<<" is from the future");

    ar & make_nvp("snowstorm::Distribution", base_object<Distribution>(*this));
    ar & make_nvp("elements", elements_);
}

Composite::Composite() {}
Composite::~Composite() {}

}

I3_SERIALIZABLE(snowstorm::Composite);
