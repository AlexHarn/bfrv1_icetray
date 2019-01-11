/**
 *   Copyright  (C) 2005
 *   The IceCube collaboration
 *   $ID$
 *   @file I3Portia.h
 *   @version $Revision: 1.0.0$
 *   @date $Date: Feb. 21st, 2008$
 *   @author Aya Ishihara <aya.ishihara@icecube.wisc.edu>
 *   @brief I3Portia stands for IceCube PORTable Impulse Analyzer, 
 *          adapted to the name of the beautiful lady in 
 *          "The Merchant of Venice"
 *
 *   I3Portia header file
 *
 */

#ifndef I3PORTIA_H
#define I3PORTIA_H
#include "icetray/I3ConditionalModule.h"
#include "dataclasses/physics/I3DOMLaunch.h"
#include "recclasses/I3PortiaPulse.h"
#include "recclasses/I3PortiaEvent.h"

class I3Position;
class OMKey;
class I3Geometry;
class I3Waveform;
class I3DOMCalibration;
class I3DOMStatus;

class I3Portia : public I3ConditionalModule{

 public:

  //enum BaselineOption{eheoptimized = 0, first = 1, last = 2, lower = 3, first_or_last = 4, iteration = 5};

  SET_LOGGER("I3Portia");
  
  /** public constructor
   */
  I3Portia(const I3Context& ctx);
  /** public deconstructor
   */
  virtual ~I3Portia();
  /** IceTray requirement
   */
  void Configure();
  /** IceTray requirement
   */
  void Physics(I3FramePtr frame);
  
  /* boundary of the basetime window around the DOM largest NPE hit timing
   *
   */
  static double kStartTimeBtw_;
  static double kEndTimeBtw_;

 /** Function to make the "launchTimeMap"
   *  I3Map<OMKey,std::vector<launch_time>> = I3Map<OMKey, launchTimeSeries>
   */
  static bool MakeSplittedDOMMap(I3DOMLaunchSeriesMap& launchMap,
				 I3MapKeyVectorDouble&  launchTimeMap,
				 const I3Geometry& geo,
				 bool makeIceTopPulse = false);
  static bool MakeLaunchTimeSeries(const I3DOMLaunchSeries& launches, std::vector<double>& launchTimeSeries);

 private:
  
  /** default assigment and copy constructors declared priveate
   *  
   */
  I3Portia();
  I3Portia(const I3Portia&);
  I3Portia& operator=(const I3Portia &);

  
  /** Function to make PortiaPulse and RecoPulse on each DOM found from ATWD
   *  
   */
  bool MakeATWDPulses(double launchTimeOfThisDOM,
		      const I3Waveform&       wave, 
		      const I3DOMCalibration& calib,
		      const I3DOMStatus&      status,
		      const I3Position&       pos,
		      I3PortiaPulse&          portia);

  /** Function to make PortiaPulse and RecoPulse on each DOM found from FADC
   *  
   */
  bool MakeFADCPulses(double launchTimeOfThisDOM,
		      const I3Waveform&       wave, 
		      const I3DOMCalibration& calib,
		      const I3DOMStatus&      status,
		      const I3Position&       pos,
		      I3PortiaPulse&          portia);

  /** Function to make PortiaPulse and RecoPulse from both ATWD and FADC
   *  e.g. NPE from the ATWD time window 
   */
  bool MakeBestPulses(const I3PortiaPulse&    atwd_p,
		      const I3Waveform&       fadc_w, 
		      const I3PortiaPulse&    fadc_p,
		      const I3DOMCalibration& calib,
		      I3PortiaPulse&          best_p);

  /** Function to make Portia Event from PortiaPulses
   */
  void MakePortiaEvent(I3MapKeyVectorDouble& launchTimeMap,
		       I3PortiaPulseMapPtr     atwd_portia,
		       I3PortiaPulseMapPtr     fadc_portia,
		       I3PortiaEventPtr        portia_event,
		       bool baseTimeWindowEvent = false);
  
  /* output names
   *
   */
  std::string outATWDPulseSeriesName_;
  std::string outFADCPulseSeriesName_;
  std::string outATWDPortiaPulseName_;
  std::string outFADCPortiaPulseName_;
  std::string outPortiaEventName_;
  std::string outBestPortiaPulseName_;
  /**
   * input names
   */
  std::string inDataReadoutName_;
  std::string inTopDataReadoutName_;
  std::string inTopATWDWaveformName_;
  std::string inATWDWaveformName_;
  std::string inFADCWaveformName_;
  std::string inDOMMapName_;
  /** baseline option
   * 
   */
  std::string optionATWDBaseLine_;
  std::string optionFADCBaseLine_;

  /** option concerning the (splitted) DOM map is read out
   *  instead of directly reading the DOM launch series map.
   *  This option is introduced in the p-frame split era
   *  to deal with SLOP frame.
   */
  bool  readExternalDOMMap_;

  /** option which information FADC/ATWD and InIce/IceTop is used
   * 
   */
  bool   makeBestPulse_;
  bool   makeIceTopPulse_;
  bool   useFADC_;
  /** default pmt gain value (in case pmt gain from detector status is unavailable) 
   * 
   */
  double pmtGain_;

  /** A hit is considered to be a hit if its integrated charge is larger than this value
   * 
   */
  double atwdThresholdCharge_;
  double fadcThresholdCharge_;
  /** A LE time is considered to be the first bin which crosses this threshold
   * 
   */
  double atwdThresholdLEAmp_;
  double fadcThresholdLEAmp_;

  /** time of waveform with the largest NPE 
   *
   */
  double largestTime_;
  bool foundLargestNPEDOM_;


};

#endif //I3PORTIA_H
