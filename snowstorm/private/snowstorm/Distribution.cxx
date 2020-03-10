
#include <icetray/I3Logging.h>
#include <dataclasses/I3Vector.h>

#include "snowstorm/Distribution.h"

namespace snowstorm {

template <class Archive>
void Distribution::serialize(Archive & ar, unsigned version)
{
    if (version > 0)
        log_fatal_stream("Version "<<version<<" is from the future");

    ar & make_nvp("I3FrameObject", base_object<I3FrameObject>(*this));
}

Distribution::~Distribution() {}

}

I3_SERIALIZABLE(snowstorm::Distribution);
