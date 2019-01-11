#ifndef I3MUE_H
#define I3MUE_H

/**
 * class: I3mue
 * 
 * Version $Id$
 *
 * Date: Tue November 1 2005
 * 
 * (c) 2005 IceCube Collaboration
 */  

#include <icetray/OMKey.h>
#include <icetray/I3ConditionalModule.h>
#include <icetray/I3Frame.h>
#include <icetray/I3Logging.h>
#include <icetray/I3ConditionalModule.h>

/**
 * @brief This module performs reconstruction of IceCube events
 *
 * @author dima
 */

class I3mue : public I3ConditionalModule{
 public:
  SET_LOGGER("I3mue");

  I3mue(const I3Context& ctx);
  virtual ~I3mue();
  void Configure();
  void Physics(I3FramePtr frame);
  void Geometry(I3FramePtr frame);
  void Calibration(I3FramePtr frame);
  void DetectorStatus(I3FramePtr frame);

private:

  bool fInitialized;
  unsigned int fEvent;
  unsigned long long fToffset;
  std::vector<std::string> fRecoPulseSeriesNames;
  std::string fDataReadoutName;
  std::string fRecoResult;
  int fRecoRnum;
  int fRecoIntr;
  std::string fOutputParticle;
  int fVerbose;

  double fDetCenterDepth;
  std::map<OMKey, unsigned long long> fIds;

};

#endif //I3MUE_H
