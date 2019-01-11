/*
 * A container class to store IceTop waveform info, like amplitude, nPeaks, ...
 * and to output it to the Frame
 */

// Amplitude, RiseTime, Trailing Edge


// Delay? More internal stuff? into this class?


#include <icetray/serialization.h>
#include "tpx/I3TopPulseInfo.h"

/* ********************************************* */
/*                 Constructor                   */
/* ********************************************* */

I3TopPulseInfo::I3TopPulseInfo() :
  amplitude(NAN),
  risetime(NAN),
  trailingEdge(NAN),
  status(OK),
  channel(0),
  sourceID(0)
{}


I3TopPulseInfo::I3TopPulseInfo(double ampl, double riset, double trailing,Status itPulseStatus) :
  amplitude(ampl),
  risetime(riset),
  trailingEdge(trailing),
  status(itPulseStatus),
  channel(0),
  sourceID(0)
{} 



//Important to make a vector out of it
bool operator==(const I3TopPulseInfo& lhs, const I3TopPulseInfo& rhs){
  return ( lhs.amplitude == rhs.amplitude &&
	   lhs.risetime == rhs.risetime &&
           lhs.trailingEdge == rhs.trailingEdge &&
	   lhs.status == rhs.status &&
	   lhs.channel == rhs.channel &&
	   lhs.sourceID == rhs.sourceID
           ); 

}


/* ********************************************* */
/*                 Destructor                    */
/* ********************************************* */
I3TopPulseInfo::~I3TopPulseInfo() { }


/* ********************************************* */
/*                 Serialization                 */
/* ********************************************* */

template <class Archive>
void I3TopPulseInfo::serialize(Archive& ar, unsigned version)
{
  if (version > i3toppulseinfo_version_)
    log_fatal("Attempting to read version %u from file but running version %u "
	      "of I3TopPulseInfo class.",version,i3toppulseinfo_version_);
  
  //Serialize the output parameters
  ar & make_nvp("amplitude", amplitude);
  ar & make_nvp("risetime", risetime);
  ar & make_nvp("trailingEdge", trailingEdge);
  ar & make_nvp("status", status);

  if (version >= 1) {
    ar & make_nvp("channel", channel);
    ar & make_nvp("sourceID", sourceID);
  }
}

//this macro instantiates all the needed serialize methods
I3_SERIALIZABLE(I3TopPulseInfo);

I3_SERIALIZABLE(I3TopPulseInfoSeriesMap);
I3_SERIALIZABLE(I3TopPulseInfoMap);

std::ostream& I3TopPulseInfo::Print(std::ostream& os) const{
  os << "[I3TopPulseInfo:\n"
     << "      Amplitude: " << amplitude << '\n'
     << "      Rise Time: " << risetime << '\n'
     << "  Trailing Edge: " << trailingEdge << '\n'
     << "         Status: " << status << '\n'
     << "        Channel: " << channel << '\n'
     << "      Source ID: " << sourceID << "\n]";
  return os;
}

std::ostream& operator<<(std::ostream& os, const I3TopPulseInfo& in){
  return(in.Print(os));
}
