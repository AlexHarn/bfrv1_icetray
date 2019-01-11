#ifndef I3EHESpotVelocity_H
#define I3EHESpotVelocity_H

#include "icetray/I3ConditionalModule.h"
#include "recclasses/I3OpheliaFirstGuessTrack.h"

//portia
#include "recclasses/I3PortiaPulse.h"

// dataclass
#include "dataclasses/geometry/I3Geometry.h"
#include "dataclasses/physics/I3Waveform.h"

class I3EHESpotVelocity : public I3ConditionalModule {

public:

  // constructor and destructor
  I3EHESpotVelocity(const I3Context& ctx);
  ~I3EHESpotVelocity();
  
  void Configure();
  void Physics(I3FramePtr frame);
  SET_LOGGER("I3EHESpotVelocity");

private:
  
  /* 
   * Default constructor, copy constructor, and assignment operator
   * declared private to prevent use
   */ 
  I3EHESpotVelocity();
  I3EHESpotVelocity(const I3EHESpotVelocity& source);
  I3EHESpotVelocity& operator=(const I3EHESpotVelocity& source);


  std::string inputPortiaEventName_;
  std::string inputFadcPulseName_;
  std::string inFADCWaveformName_;
  std::string outputSpotTrackName_;

  int LCOption_;

  /**
   * Minimum number of hits requested to calculate firstguess values
   * if not satisfied, results are filled with un-reasonable values
   */
  int minNumberDomWithPulse_;


  I3OpheliaFirstGuessTrackPtr GetSpotTrack(const I3PortiaPulseMap&     fadcPulseMap, 
					   const I3WaveformSeriesMap&  fadcWaveMap,
					   const I3OMGeoMap& omgeo,
					   double earlistTime, double lastTime);

};
#endif //I3EHESpotVelocity_H
