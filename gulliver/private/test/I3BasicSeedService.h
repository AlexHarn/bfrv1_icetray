#ifndef GULLIVER_I3BASICSEEDSERVICE_H_INCLUDED
#define GULLIVER_I3BASICSEEDSERVICE_H_INCLUDED

#include <gulliver/I3SeedServiceBase.h>

class I3BasicSeedService: public I3SeedServiceBase {
 public:
  I3BasicSeedService(): I3SeedServiceBase() {}
  ~I3BasicSeedService() {}

  virtual unsigned int SetEvent(const I3Frame &f) {return 42;}
  virtual I3EventHypothesis GetSeed(unsigned int iseed) const {
    I3Particle particle;
    particle.SetEnergy(42);
    return I3EventHypothesis(particle);
  }
};
#endif