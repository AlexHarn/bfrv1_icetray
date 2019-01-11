// $Id$

#ifndef NOISEENGINE_H_INCLUDED
#define NOISEENGINE_H_INCLUDED

#include <algorithm>
#include <vector>
#include <string>

#include <sys/time.h>

#include "icetray/I3ConditionalModule.h"
#include "icetray/I3Frame.h"
#include "icetray/I3Bool.h"
#include "icetray/I3Units.h"

#include "dataclasses/I3Double.h"
#include "dataclasses/I3Constants.h"
#include "dataclasses/I3Vector.h"
#include "dataclasses/geometry/I3Geometry.h"
#include "dataclasses/physics/I3RecoPulse.h"

#ifndef HEALPIX_CXX_ON_FEDORA
#include "healpix_cxx/healpix_base.h"
#else
#include "healpix/healpix_base.h"
#endif

#include "NoiseEngine/HitPair.h"

/**
 * @brief IceTray 
 */
class NoiseEngine: public I3ConditionalModule
{
 public:
  /**
   * \brief Constructor:  builds an instance of the module, with the
   *         context provided by IceTray.
   */
  NoiseEngine(const I3Context& ctx);

  /**
   * \brief Destructor: deletes the module
   */
  ~NoiseEngine();

  /**
   * \brief Configure: Grabs options from the python processing script.
   */
  void Configure();

  /**
   * \brief Physics: Process the event if the input series are in the physics frame
   */
  void Physics(I3FramePtr frame);

  /**
   * \brief GetHitPairs: Create all of the possible hit pairs using the time, velocity constraints
   */
  std::vector<HitPair> GetHitPairs(const I3Geometry& geometry,
				   I3RecoPulseSeriesMapConstPtr hitmap, 
				   double startTime,
				   std::vector<double>& velocities);
  
  /**
   * \brief PickStartTime: Choose a start time that maximizes the number of hits within a time window
   */
  double PickStartTime(I3RecoPulseSeriesMapConstPtr hitmap);

  /**
   * \brief CheckTrigger: Check all of the healpix bins to identify any bins over threshold
   */
  std::vector<double> CheckTrigger(std::vector<HitPair> TWPairs,
				   Healpix_Base hp_base,
				   double threshold,
				   bool& decision
				   );  
  
 private:
  double optStartVelocity_;
  double optEndVelocity_;
  double optTimeWindowLength_;
  
  int optHealpixOrder_;
  int optNChLimit_;
  double optThreshold_;

  bool optChargeWeight_;

  std::string optHitSeriesName_;
  std::string optOutputName_;
  std::string optTimerName_;
  std::string optBinsName_;
  std::string optVelocityName_;
  
  SET_LOGGER("NoiseEngine");
};

#endif

