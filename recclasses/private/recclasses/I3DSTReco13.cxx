/**
    copyright  (C) 2008
    the icecube collaboration

    @version $Revision: 1.2 $
    @date: mié dic 10 09:25:03 CST 2008
    @author juancarlos@icecube.wisc.edu
*/

#include <icetray/serialization.h>
#include <recclasses/I3DST.h>
#include <recclasses/I3DSTReco13.h>
#include <recclasses/I3DST13.h>
#include <boost/functional/hash/hash.hpp>

#include <cmath>

using std::cout;

I3DSTReco13::I3DSTReco13() : 
	reco1_(),
	reco2_(),
	nchannel_(0),
	nstring_(0),
	cog_(0,0,0),
	ndir_(0),
	ldir_(0),
	rlogl_(0),
	logE_(0),
	sub_event_id_(0),
	triggertag_(0)
{ }

void I3DSTReco13::SetRlogL(double rlogl)
{
    if (std::isnan(rlogl)) {
      rlogl_= 255;
      return;
    } 
    int irlogl = int(round(rlogl*10));

    irlogl = max(irlogl,0);
    irlogl = min(irlogl,254);
    rlogl_= uint8_t(irlogl);
}

void I3DSTReco13::SetLogE(double logE)
{
    if (std::isnan(logE)) {
	  logE_ = 255;
	  return;
    }

	// store logE as a bin index 
	int ilogE = int(round((logE - DST_LOGE_OFFSET)/DST_LOGE_BINSIZE));

	// Make sure index is within 8-bit range of uint8_t
	ilogE = min(ilogE,254);
	ilogE = max(ilogE,0);

	logE_ = uint8_t(ilogE);
}

std::ostream& I3DSTReco13::Print(std::ostream& os) const
{
  os << "[I3DST TriggerTag : " << triggertag_ << '\n'
     << "       SubEventID : " << sub_event_id_ << '\n'
     << "            Reco1 : " << reco1_ << '\n'
     << "            Reco2 : " << reco2_ << '\n'
     << "         NChannel : " << nchannel_ << '\n'
     << "          NString : " << nstring_ << '\n'
     << "              COG : " << cog_ << '\n'
     << "             NDir : " << ndir_ << '\n'
     << "             LDir : " << ldir_ << '\n'
     << "            RLogL : " << rlogl_ << '\n'
     << "             LogE : " << logE_ << '\n'
     << "        RecoLabel : " << reco_label_ << ']';
  return os;
}

std::ostream& operator<<(std::ostream& os, const I3DSTReco13& r)
{
  return(r.Print(os));
}

template <class Archive>
void
I3DSTReco13::serialize(Archive& ar,unsigned version)
{
  if (version != i3dst13_version_)
    log_fatal("Attempting to read version %u from file but running version %u of I3DST class.", 
                    version,i3dst13_version_);
  ar & make_nvp("I3FrameObject", base_object<I3FrameObject>(*this));
  ar & make_nvp("triggertag",triggertag_);
  ar & make_nvp("sub_event_id",sub_event_id_);
  ar & make_nvp("reco1",reco1_);
  ar & make_nvp("reco2",reco2_);
  ar & make_nvp("nchannel",nchannel_);
  ar & make_nvp("nstring",nstring_);
  ar & make_nvp("cog",cog_);
  ar & make_nvp("ndir",ndir_);
  ar & make_nvp("ldir",ldir_);
  ar & make_nvp("rlogl",rlogl_);
  ar & make_nvp("logE",logE_);
  ar & make_nvp("reco_label",reco_label_);
}

I3_SERIALIZABLE(I3DSTReco13);
