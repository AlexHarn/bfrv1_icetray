#ifndef DST_I3DSTMODULE_16_H_INCLUDED
#define DST_I3DSTMODULE_16_H_INCLUDED

#include <fstream>
#include <string>
#include <set>
#include <dst/HealPixCoordinates.h>
#include <icetray/I3ConditionalModule.h>
#include <icetray/I3TrayHeaders.h>
#include <icetray/I3Logging.h>
#include <icetray/I3Units.h>
#include <dataclasses/I3Position.h>
#include "dataclasses/geometry/I3Geometry.h"
#include <dataclasses/physics/I3Particle.h>
#include "dataclasses/physics/I3RecoHit.h"
#include "dataclasses/physics/I3RecoPulse.h"
#include "dataclasses/physics/I3DOMLaunch.h"
#include <phys-services/I3Calculator.h>
#include <cmath>
#include <assert.h>
#include <recclasses/I3DST16.h>
#include <recclasses/I3DSTHeader16.h>

using namespace std;

/**
 * copyright  (C) 2007
 * the icecube collaboration
 *
 * @brief I3DSTModule16 is part of the dst project. I3DSTModule16 extracts key
 * information about physics events and generates an I3DST objec which is a
 * very compact representation of an event.
 *
 * @date $Date: 2007-07-30 11:36:03 $
 * @author juancarlos@icecube.wisc.edu
 *
 * @todo Add a list of recontructions to search on a priority basis
 *
 */


class I3DSTModule16 : public I3ConditionalModule
{
  /**
   *Constructor
   */
  I3DSTModule16();
  I3DSTModule16(const I3DSTModule16&);

  /**
   *Assignment operator
   */
  I3DSTModule16& operator=(const I3DSTModule16&);


  bool init_; // have initial values been initialized
  bool dstHeaderWritten_; // have initial values been initialized

  // name under which DST objects will be stored in frame
  std::string dstName_;
  std::string dstHeaderName_;
  std::string dstRecoName_;

  int eventIndex_;
  int dstHeaderPrescale_;
  unsigned mjd_;

  // variables to keep track of time
  double startTime_;

  // names of objects to read from frame
  vector<std::string> i3recoList_;
  std::string trigger_name_;
  vector<unsigned> utriggerIDs_;
  vector<uint16_t> triggerIDs_;
  std::string eventheader_name_;
  HealPixCoordinatePtr dstcoord_;
  std::string i3DirectHitsName_;
  bool ignoreDirectHits_;
  std::string icrecoseries_name_;
  std::string inIceRaw_;
  std::string fitParamsName_;
  std::string energyEstimateName_;

  // cartesian coordinates of center of detector
  double centerX_;
  double centerY_;
  double centerZ_;
  uint16_t hpix_nside_;
  std::string pickKey_;
  bool use_pick_;
  I3IcePickPtr pick_;
  vector<double> directHitsTimeRange_;

  I3PositionPtr detectorCenter_;

  public:

  /**
   *Constructor
   */
  I3DSTModule16(const I3Context& ctx);

  /**
   *Destructor
   */
  virtual ~I3DSTModule16() { }

  /**
   * Initializes I3DSTHeader16 and coordinate sky map
   */
  void Configure();

  /**
   */
  void Finish();

  /**
   * Writes dst header
   */
  void DAQ(I3FramePtr frame);

  /**
   * Extracts information from each event and writes an I3DST object to the
   * frame.
   */
  void Physics(I3FramePtr frame);

  /**
   * @brief Overrides the function where frames are sent to IcePicks installed in the context
   * and forces DAQ to ignore condition.
   */
  bool ShouldDoProcess(I3FramePtr frame);

  double CalculateEventDuration( I3DOMLaunchSeriesMapConstPtr domLaunches);


  SET_LOGGER("I3DSTModule16");
};

#endif
