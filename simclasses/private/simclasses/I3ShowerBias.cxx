#include <icetray/serialization.h>
#include <simclasses/I3ShowerBias.h>

template <class Archive>
void 
I3ShowerBias::serialize(Archive& ar, unsigned version)
{
  ar & make_nvp("I3FrameObject", base_object<I3FrameObject>(*this));
  ar & make_nvp("type",type);
  ar & make_nvp("target",target);
}

I3_SERIALIZABLE(I3ShowerBias);
I3_SERIALIZABLE(I3ShowerBiasMap);
