#ifndef I3EHEFirstGuess_H
#define I3EHEFirstGuess_H

#include "icetray/I3ConditionalModule.h"
#include "dataclasses/geometry/I3Geometry.h"
#include "dataclasses/I3Map.h"
//input
#include "recclasses/I3PortiaPulse.h"

class I3OpheliaFirstGuessTrack;

class I3EHEFirstGuess : public I3ConditionalModule {

public:

  // constructor and destructor
  I3EHEFirstGuess(const I3Context& ctx);
  ~I3EHEFirstGuess();
  
  void Configure();
  void Physics(I3FramePtr frame);
  SET_LOGGER("I3EHEFirstGuess");

private:
  
  /* 
   * Default constructor, copy constructor, and assignment operator
   * declared private to prevent use
   */ 
  I3EHEFirstGuess();
  I3EHEFirstGuess(const I3EHEFirstGuess& source);
  I3EHEFirstGuess& operator=(const I3EHEFirstGuess& source);


  int eventCounter_;

  std::string inputPulseName1_;
  std::string inputPulseName2_;
  std::string inputPortiaEventName_;
  std::string inputLaunchName_;
  std::string inputDOMMapName_;
  std::string outputTrackName_;
  std::string outputTrackNameBtw_;
  std::string outputParticleName_;
  std::string outputParticleNameBtw_;

  /**
   * Option to choose charge estimated by I3Portia.
   * 0 bigger(ATWD|FADC) [default] 1 charge intagrating ATWD pulse 2 charge integrating FADC pulse
   */
  int chargeOption_;
  int LCOption_;

  double recoNPEThres_;

  /**
   * Minimum number of hits requested to calculate firstguess values
   * if not satisfied, results are filled with un-reasonable values
   */
  int minNumberDomWithPulse_;

  void MakeBestPulseMap(I3MapKeyVectorDouble& launchMap, 
			I3PortiaPulseMapConstPtr pulseMap1_ptr, 
			I3PortiaPulseMapConstPtr pulseMap2_ptr, 
			I3PortiaPulseMapPtr bestPulseMap_ptr,
			bool baseTimeWindowEvent = false);

  void GetLineFit(I3MapKeyVectorDouble& launchMap, 
		  I3PortiaPulseMapConstPtr                bestPulseMap_ptr,
		  const I3OMGeoMap& omgeo,
		  boost::shared_ptr<I3OpheliaFirstGuessTrack>    result_ptr,
		  bool baseTimeWindowEvent = false,
		  double largestNPETime = 0.0);

  double GetFitQuality(I3MapKeyVectorDouble& launchMap, 
		       I3PortiaPulseMapConstPtr               pulseMap_ptr, 
		       boost::shared_ptr<I3OpheliaFirstGuessTrack>   result_ptr,
		       double t0);
};
#endif //I3EHEFirstGuess_H
