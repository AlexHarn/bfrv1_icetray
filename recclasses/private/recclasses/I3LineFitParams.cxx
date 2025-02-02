#include "recclasses/I3LineFitParams.h"
#include "recclasses/Utility.h"
#include <icetray/serialization.h>
#include <icetray/I3Logging.h>

I3LineFitParams::~I3LineFitParams() {}

template <class Archive>
void I3LineFitParams::serialize(Archive& ar, unsigned version)
{
  if (version>i3linefitparams_version_)
    log_fatal("Attempting to read version %u from file but running version %u of I3LineFitParams class.",
              version, i3linefitparams_version_);

  ar & make_nvp("I3FrameObject", base_object<I3FrameObject>(*this));
  ar & make_nvp("LFVel",  LFVel );
  ar & make_nvp("LFVelX", LFVelX );
  ar & make_nvp("LFVelY", LFVelY );
  ar & make_nvp("LFVelZ", LFVelZ );
  if (version >= 1)
    ar & make_nvp("nHits", nHits );
}

std::ostream& operator<<(std::ostream& os, const I3LineFitParams& p)
{
  return(p.Print(os));
}

std::ostream& I3LineFitParams::Print(std::ostream& os) const
{
  os << "[ I3LineFitParams::"
     << "\n   LFVel: " << LFVel
     << "\n  LFVelX: " << LFVelX
     << "\n  LFVelY: " << LFVelY
     << "\n  LFVelZ: " << LFVelZ
     << "\n   nHits: " << nHits
     << "]";
  return os;
}

bool I3LineFitParams::operator==(const I3LineFitParams& other) const
{
  return
    nan_aware_equality(LFVel, other.LFVel) &&
    nan_aware_equality(LFVelX, other.LFVelX) &&
    nan_aware_equality(LFVelY, other.LFVelY) &&
    nan_aware_equality(LFVelZ, other.LFVelZ) &&
    nHits == other.nHits;
}

I3_SERIALIZABLE(I3LineFitParams);
