#ifndef I3TOPPULSEINFO_H_INCLUDED
#define I3TOPPULSEINFO_H_INCLUDED

#include "dataclasses/I3Vector.h"
#include "dataclasses/Utility.h"
#include "icetray/OMKey.h"
#include "dataclasses/I3Map.h"

#include <vector>

#define I3TOPPULSEINFO_H_I3TopPulseInfo_Status	\
  (OK)(Saturated)(BadCharge)(BadTime)

static const unsigned i3toppulseinfo_version_ = 1;

/**
 * A class that has all toppulseinfoParams that can be useful to describe IT waveforms
 */
class I3TopPulseInfo // not necessary: I3Vector will be put into frame: public I3FrameObject
{
  
 public:

  enum Status   // or ITPulseStatus
  {
    OK = 0,
    Saturated = 10,
    BadCharge = 20,
    BadTime = 30
  };


  I3TopPulseInfo();
  I3TopPulseInfo(double ampl, double riset, double trailing, Status itPulseStatus = OK);
  
  virtual ~I3TopPulseInfo();
  
  std::ostream& Print(std::ostream&) const;

  /* public variables */
  double amplitude;            // amplitude of the IceTop waveform 
  double risetime;             // Risetime of the IceTop waveform
  double trailingEdge;         // Trailing Edge of the IceTop waveform
  Status status;               // More status info about the IceTop waveform
  uint8_t channel;             // ATWD channel
  uint8_t sourceID;            // ATWD chip ID

  // More coming... like nPeaks
  
 private:
  
  //serialization routine
  friend class icecube::serialization::access;         //provide boost access to my privates
  template <class Archive> void serialize(Archive& ar, unsigned version);

};

std::ostream& operator<<(std::ostream&, const I3TopPulseInfo&);
bool operator==(const I3TopPulseInfo& lhs, const I3TopPulseInfo& rhs);

I3_POINTER_TYPEDEFS(I3TopPulseInfo);
I3_CLASS_VERSION(I3TopPulseInfo,i3toppulseinfo_version_);


typedef std::vector<I3TopPulseInfo> I3TopPulseInfoSeries;

typedef I3Map<OMKey, I3TopPulseInfoSeries> I3TopPulseInfoSeriesMap;
typedef I3Map<OMKey, I3TopPulseInfo> I3TopPulseInfoMap;

I3_POINTER_TYPEDEFS(I3TopPulseInfoSeries);
I3_POINTER_TYPEDEFS(I3TopPulseInfoSeriesMap);
I3_POINTER_TYPEDEFS(I3TopPulseInfoMap);

#endif
