#ifndef MONOPOLE_PROPAGATOR_I3MONOPOLEPROPAGATOR_H_INCLUDED
#define MONOPOLE_PROPAGATOR_I3MONOPOLEPROPAGATOR_H_INCLUDED
/**
 * class: I3MonopolePropagator.h
 * (c) 2008 IceCube Collaboration
 * Version $Id: I3MonopolePropagator.h 124417 2014-10-10 15:16:00Z jacobi $
 *
 * Date 06 Feb 2008
 * @version $Revision: 124417 $
 * @date $Date: 2014-10-10 10:16:00 -0500 (Fr, 10. Okt 2014) $
 * @author Brian Christy <bchristy@icecube.umd.edu>
 * @author Alex Olivas <olivas@icecube.umd.edu>
 *
 * @brief A module to convert Monopoles into a chain of propagated particles through the ice
 *
 */

#include "icetray/I3Module.h"
#include <string>
#include "dataclasses/physics/I3MCTreeUtils.h"

class I3MonopolePropagator : public I3Module
{
 public:

  I3MonopolePropagator(const I3Context& ctx);
  ~I3MonopolePropagator();

  void Configure();
  void DAQ(I3FramePtr frame);
  void Finish();

 private:
  I3MonopolePropagator();
  I3MonopolePropagator(const I3MonopolePropagator&);
  I3MonopolePropagator& operator=(const I3MonopolePropagator&);

  std::string inputTreeName_;
  std::string outputTreeName_;
  std::string infoName_;
  double betaThreshold_;

  // fast
  bool calcEn_;
  bool calcDen_;
  double stepSize_;
  bool checkParticle_;
  double speedmin_;
  double maxlength_;
  double minlength_;
  double maxdistfromcenter_;
  bool profiling_;

  // slow
  double meanFreePath_;
  bool useCorrectDecay_;
  bool scaleEnergy_;
  double energyScaleFactor_;
  double deltaEnergy_;

  SET_LOGGER("I3MonopolePropagator");

};

#endif //MONOPOLE_PROPAGATOR_I3MONOPOLEPROPAGATOR_H_INCLUDED
