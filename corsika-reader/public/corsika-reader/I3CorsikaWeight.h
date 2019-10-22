
#include "dataclasses/physics/I3Particle.h"

struct I3CorsikaWeight : public I3FrameObject
{
  I3Particle primary;
  double bias_factor;
  int bias_target;
  double weight;
  double max_x;

private:
  friend class icecube::serialization::access;
  
  template <class Archive>
  void serialize(Archive& ar, unsigned version);
};

I3_POINTER_TYPEDEFS(I3CorsikaWeight);
