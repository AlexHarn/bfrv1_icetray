
#include <vector>
#include "snowstorm/Distribution.h"

namespace snowstorm {

class Composite : public Distribution {
public:
    Composite();
    virtual ~Composite();

    void add(const DistributionPtr &element);

    virtual std::vector<double> Sample(I3RandomService&) const override;
    virtual double LogPDF(const std::vector<double>&) const override;
    virtual size_t size() const override;
private:

    friend class icecube::serialization::access;
    template <class Archive> void serialize(Archive & ar, unsigned version);
    
    std::vector<DistributionConstPtr> elements_;
};

I3_POINTER_TYPEDEFS(Composite);

}

I3_CLASS_VERSION(snowstorm::Composite, 0);