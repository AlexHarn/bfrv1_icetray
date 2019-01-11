/**
    copyright  (C) 2004
    the icecube collaboration
    $Id: I3OpheliaRawDataEraser.h 4211 2005-11-10 syoshida $

    @version $Revision: 1.2 $
    @date $Date: 2004-10-25 21:23:15 +0900
    @author Shigeru Yoshida
    @author Mio Ono <mio@hepburn.s.chiba-u.jp>

    I3OpheliaRawDataEraser

    This class erases rawATWD/rawFADC in I3DOMLaunch dataclass
    for compressing file size. The class is based on I3RawDataEraser 
    in brutus project which is originally developed for
    high energy MC data production where produced files are HUGE.
*/

#ifndef I3OPHELIARAWDATAERASER_H_INCLUDED
#define I3OPHELIARAWDATAERASER_H_INCLUDED

#include "icetray/I3TrayHeaders.h"
#include "icetray/I3ConditionalModule.h"
#include "dataclasses/physics/I3DOMLaunch.h"
#include "dataclasses/Utility.h"

/**
 * Dummy value for raw data
 */
const int dummyRawData_ = 999;

/**
 * This class erases rawATWD/rawFADC in I3DOMLaunch dataclass
 */
class I3OpheliaRawDataEraser : public I3ConditionalModule{

 public:

  SET_LOGGER("I3OpheliaRawDataEraser"); 

  // constructor and destructor
  I3OpheliaRawDataEraser(const I3Context& ctx);
  virtual ~I3OpheliaRawDataEraser();

  void Configure();
  void Physics(I3FramePtr frame);

private:

// default assigment and copy constructors declared priveate
  
  I3OpheliaRawDataEraser();
  I3OpheliaRawDataEraser(const I3OpheliaRawDataEraser&);
  I3OpheliaRawDataEraser& operator=(const I3OpheliaRawDataEraser &);

  /**
   * Erase timing-wise rae data in a given launch
   */
  void EraseRawDataInDOMLaunch(I3DOMLaunch& dom_launch); 

  /**
   * Find ATWD channel used by DOMcalibrator with calibration mode 1
   * This method is a copy of I3DOMcalibrator::FindNonSaturatedATWDChannel
   */
  unsigned int FindNonSaturatedATWDChannel(I3DOMLaunch& domLaunch, int saturationVal);
  
  /**
   * Key name of I3DOMLaunch Map.
   */
  std::string inDataReadoutName_;
 
  /**
   * Saturation value of ATWD raw data
   */
  int atwdSaturationValue_;

};

#endif //I3OPHELIARAWDATAERASER_H_INCLUDED
