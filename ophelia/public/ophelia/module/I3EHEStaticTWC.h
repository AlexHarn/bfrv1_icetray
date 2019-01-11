#ifndef I3EHEStaticTWC_H
#define I3EHEStaticTWC_H

#include "icetray/I3ConditionalModule.h"
#include "dataclasses/physics/I3DOMLaunch.h"
//input
#include "recclasses/I3PortiaPulse.h"
#include "dataclasses/geometry/I3Geometry.h"

struct DOMhit_pos_npe
{

  DOMhit_pos_npe(const double &t, const I3Position &pos, const double &n)
  {
    time = t;
    position = pos;
    npe = n;
  }

  DOMhit_pos_npe(DOMhit_pos_npe const & hit)
  {
    time = hit.time;
    position = hit.position;
    npe = hit.npe;
  }

  double time;
  I3Position position;
  double npe;
};
bool operator<(const DOMhit_pos_npe& left, const DOMhit_pos_npe& right)
{
  return left.time < right.time ;
}

bool operator>(const DOMhit_pos_npe& left, const DOMhit_pos_npe& right)
{
  return left.time > right.time ;
}

class I3OpheliaStaticTWCTrack;


class I3EHEStaticTWC : public I3ConditionalModule {

public:

  // constructor and destructor
  I3EHEStaticTWC(const I3Context& ctx);
  ~I3EHEStaticTWC();
  
  void Configure();
  //void DAQ(I3FramePtr frame);
  void Physics(I3FramePtr frame);
  SET_LOGGER("I3EHEStaticTWC");

private:
  
  /* 
   * Default constructor, copy constructor, and assignment operator
   * declared private to prevent use
   */ 
  I3EHEStaticTWC();
  I3EHEStaticTWC(const I3EHEStaticTWC& source);
  I3EHEStaticTWC& operator=(const I3EHEStaticTWC& source);

  std::string inputPulseName_;
  std::string inputPortiaEventName_;
  std::string outputPulseName_;
  double timeInterval_;
  double timeWindowNegative_;
  double timeWindowPositive_;

};
#endif //I3EHEStaticTWC_H
