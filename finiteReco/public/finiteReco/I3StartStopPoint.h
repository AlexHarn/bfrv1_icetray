/**
 * @brief declaration of the I3StartStopPoint class
 *
 * @file I3StartStopPoint.h
 * @version $Revision: 48479 $
 * @date $Date: 2008-08-22 11:18:02 +0200 (Fr, 22 Aug 2008) $
 * @author Sebastian Euler <sebastian.euler@icecube.wisc.edu>
 * 
 * This Module estimates the start and stop point of a given track.
 * I3FiniteCalc is called. An I3Particle of the given shape and I3FiniteCuts are
 * returned to the frame.
 * (see <https://wiki.icecube.wisc.edu/index.php/FiniteReco.I3StartStopPoint>)
 */

#ifndef I3STOPPOINT_H_INCLUDED
#define I3STOPPOINT_H_INCLUDED

#include "icetray/I3ConditionalModule.h"
#include "dataclasses/physics/I3Particle.h"
#include "dataclasses/physics/I3RecoPulse.h"


class I3StartStopPoint : public I3ConditionalModule
{
 public:
  I3StartStopPoint(const I3Context& ctx);
  ~I3StartStopPoint();
  void Configure();
  void Physics(I3FramePtr frame);
  void Finish();

 private:
  I3StartStopPoint();
  I3StartStopPoint(const I3StartStopPoint& source);
  I3StartStopPoint& operator=(const I3StartStopPoint& source);
  
   
  void CalculateStartStopPoint(I3FramePtr frame,
                               I3RecoPulseSeriesMapConstPtr pulsemap,
                               const I3Particle& particle);
  std::string fitName_;
  std::string inputRecoPulses_;
  double cylinderRadius_;
  I3Particle::ParticleShape shape_ ;
  int shapeInt_;
  /// counter variable for events where the module succeeds in finding vertices
  unsigned int recoOKCounter_;
  /// counter variable for events where the input fit has status OK, but does not even come close to a launched DOM
  unsigned int recoFailedCounter_;
  /// counter variable for events where the input fit does not have status OK
  unsigned int inputRecoFailedCounter_;
  
  SET_LOGGER("I3StopPoint");

};

#endif
