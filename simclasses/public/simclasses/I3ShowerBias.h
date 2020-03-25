#ifndef I3SHOWERBIAS_H_INCLUDED
#define I3SHOWERBIAS_H_INCLUDED

#include "icetray/I3DefaultName.h"
#include "dataclasses/I3Map.h"
#include "dataclasses/physics/I3Particle.h"

struct I3ShowerBias : public I3FrameObject{

  enum BiasParticleType {
    Mu = 0,
    NuMu,
    NuE
  };

  I3ShowerBias(){ type=BiasParticleType(-1);target=NAN;}
  
  I3ShowerBias(BiasParticleType t1, double t2){
    type=t1; target=t2;
  }

  BiasParticleType type;
  double target;

  friend class icecube::serialization::access;

  template <class Archive>
  void serialize(Archive& ar, unsigned version);
};

typedef I3Map<I3ParticleID, I3ShowerBias> I3ShowerBiasMap;

I3_DEFAULT_NAME(I3ShowerBiasMap);
I3_POINTER_TYPEDEFS(I3ShowerBiasMap);

#endif
