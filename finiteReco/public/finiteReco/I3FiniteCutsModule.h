/**
 * @brief declaration of the I3FiniteCutsModule class
 *
 * @file I3FiniteCutsModule.h
 * @version $Revision$
 * @date $Date$
 * @author Sebastian Euler <sebastian.euler@icecube.wisc.edu>
 *
 * This module derives cuts which might be used to determine whether an event is starting or not. I3FiniteCalc is called and the returned I3FiniteCuts object is placed in the frame. (see <https://wiki.icecube.wisc.edu/index.php/FiniteReco.I3FiniteCuts>)
 */

#ifndef I3FINITECUTSMODULE_H_INCLUDED
#define I3FINITECUTSMODULE_H_INCLUDED

#include "icetray/I3Module.h"
#include "dataclasses/physics/I3Particle.h"


class I3FiniteCutsModule : public I3Module
{
 public:

  I3FiniteCutsModule(const I3Context& ctx);
  ~I3FiniteCutsModule();
  void Configure();
  void Physics(I3FramePtr frame);
  
 
private:
  I3FiniteCutsModule();
  I3FiniteCutsModule(const I3FiniteCutsModule& source);
  I3FiniteCutsModule& operator=(const I3FiniteCutsModule& source);
  
  std::string fitName_;
  std::string inputRecoPulses_;
  double cylinderRadius_;
  SET_LOGGER("I3FiniteCutsModule");

};

#endif
