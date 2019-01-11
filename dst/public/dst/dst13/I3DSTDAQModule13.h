#ifndef DST_I3DSTDAQMODULE_13_H_INCLUDED
#define DST_I3DSTDAQMODULE_13_H_INCLUDED

#include <fstream>
#include <string>
#include <set>
#include <dst/HealPixCoordinates.h>
#include <icetray/I3Module.h>
#include <icetray/I3IcePick.h>
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
#include <recclasses/I3DST13.h>
#include <recclasses/I3DSTHeader13.h>

using namespace std;

/**
 * copyright  (C) 2007
 * the icecube collaboration
 *
 * @brief I3DSTDAQModule13 is part of the dst project. I3DSTDAQModule13 extracts key
 * information about physics events and generates an I3DST objec which is a
 * very compact representation of an event.
 *
 * @date $Date: 2007-07-30 11:36:03 $
 * @author juancarlos@icecube.wisc.edu
 *
 * @todo Add a list of recontructions to search on a priority basis
 *
 */


class I3DSTDAQModule13 : public I3Module
{
  /**
   *Constructor
   */
  I3DSTDAQModule13();
  I3DSTDAQModule13(const I3DSTDAQModule13&);

  /**
   *Assignment operator
   */
  I3DSTDAQModule13& operator=(const I3DSTDAQModule13&);


  bool init_; // have initial values been initialized

  // name under which DST objects will be stored in frame
  std::string dstName_;
  std::string dstHeaderName_;

  int eventIndex_;
  int dstHeaderPrescale_;
  unsigned mjd_;

  // variables to keep track of time
  double startTime_;

  // names of objects to read from frame
  std::string trigger_name_;
  vector<unsigned> utriggerIDs_;
  vector<uint16_t> triggerIDs_;
  uint16_t hpix_nside_;
  vector<std::string> i3recoList_;
  std::string eventheader_name_;
  bool dstHeaderWritten_;
  std::string pickKey_;
  bool use_pick_;
  I3IcePickPtr pick_;


  public:

  /**
   *Constructor
   */
  I3DSTDAQModule13(const I3Context& ctx);

  /**
   *Destructor
   */
  virtual ~I3DSTDAQModule13() { }

  /**
   * Initializes I3DSTHeader13 and coordinate sky map
   */
  void Configure();

  /**
   */
  void Finish();

  /**
   * Extracts information from each event and writes an I3DST object to the
   * frame.
   */
  void DAQ(I3FramePtr frame);


  SET_LOGGER("I3DSTDAQModule13");
};

#endif
