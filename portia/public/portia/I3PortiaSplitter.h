/**
 *   Copyright  (C) 2005
 *   The IceCube collaboration
 *   $ID$
 *   @file I3PortiaSplitter.h
 *   @date $Date: July. 27th, 2012$
 *   @author Shigeru Yoshida
 *   @brief The p-frame splitter for I3Portia. 
 *          This is for the L2 EHE process, mainly
 *          to extract events out of SLOP. All DOM launch time
 *          within the time frame provied by trigger-splitter
 *          is bundled and emitted to a single p-frame.
 *
 */

#ifndef I3PORTIASPLITTER_H
#define I3PORTIASPLITTER_H
#include "icetray/I3ConditionalModule.h"
#include "dataclasses/physics/I3DOMLaunch.h"
#include "dataclasses/I3Map.h"
#include "dataclasses/I3TimeWindow.h"

class I3Position;
class OMKey;
class I3Waveform;
class I3DOMCalibration;
class I3DOMStatus;

class I3PortiaSplitter : public I3ConditionalModule{

 public:

  SET_LOGGER("I3PortiaSplitter");
  
  /** public constructor
   */
  I3PortiaSplitter(const I3Context& ctx);
  /** public deconstructor
   */
  virtual ~I3PortiaSplitter();
  /** IceTray requirement
   */
  void Configure();
  /** IceTray requirement
   */
  void Physics(I3FramePtr frame);
  
  /* time of the margin to select DOM launches within
   * the trigger frame
   */
  static double kTimeMargin_;

 private:
  
  /** default assigment and copy constructors declared priveate
   *  
   */
  I3PortiaSplitter();
  I3PortiaSplitter(const I3PortiaSplitter&);
  I3PortiaSplitter& operator=(const I3PortiaSplitter &);

  
  /** Function to make the "SplittedDOMMap"
   *  I3Map<OMKey,std::vector<launch_time>> = I3Map<OMKey, launchTimeSeries>
   */
  bool MakeSplittedDOMMap(const I3DOMLaunchSeriesMap& launchMap,
			  I3MapKeyVectorDouble&  splittedDOMMap,
			  const I3Geometry& geo);
  bool MakeLaunchTimeSeries(const I3DOMLaunchSeries& launches, std::vector<double>& launchTimeSeries);

  std::string outDOMMapName_;
  /**
   * input names
   */
  std::string inDataReadoutName_; // name of I3DOMLaunchSeriesMap
  std::string inEventHeaderName_; // name of I3EventHeader
  std::string subEventStreamName_; // name of SubEventStream in I3EventHeader. Use for looping over ONLY in-ice p-frames
  std::string frameTimeWindowKeyName_; // key name of I3TimeWindow storing start time and end time of p-frame. Provided by trigger-splitter

  /** option to split series of launch time.
   * 
   */
  bool   splitLaunchTime_; /** if false, null split - every launch time goes to p-frame */

  /** Start and end time of the trigger frame. All launches
   *  in [frameStartTime_, frameEndTime_] with kTimeMargin_ are bundled
   *  and thrown into a single p-frame.  If splitLaunchTime_=false,
   *  every each of launches are bundled (i.e., null split)
   */
  double frameStartTime_;
  double frameEndTime_;

};

#endif //I3PORTIASPLITTER_H
