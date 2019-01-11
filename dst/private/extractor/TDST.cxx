/**
    copyright  (C) 2004
    the icecube collaboration
    $Id: dst.cxx 16031 2006-02-20 12:38:45Z troy $

    @version $Revision: 1.2 $
    @date $Date: 2006-02-20 06:38:45 -0600 (lun, 20 feb 2006) $
    @author juancarlos@icecube.wisc.edu
*/

#include "recclasses/I3DST.h"
#include "dst/extractor/TDST.h"
#include <icetray/serialization.h>

#include <iostream>
#include <cmath>

using std::cout;


TDST::~TDST() { }


TDST::TDST():
	weight(1.0),
	TimeScale(1.0),
    mjd(0), usec(0), mjdTime(0),
    localMST(0),
    localAntiS(0),
    llhAzimuth(0),
    llhZenith(0),
    lfAzimuth(0),
    lfZenith(0),
    linllhOpeningAngle(0),
    RA(0),
    Dec(0),
    RAAntiS(0),
    DecAntiS(0),
    RASolar(0),
    DecSolar(0),
    RASun(0),
    DecSun(0),
    RAMoon(0),
    DecMoon(0),
    logMuE(0),
    rlogl(0),
    sdcog(0),
    cogx(0),
    cogy(0),
    cogz(0),
    ldir(0),
    runId(0),
    ndir(0),
    nchan(0),
    nstring(0),
    subrunId(0),
    isGoodLineFit(false)
{
}


bool TDST::cut_nan()
{
  if (std::isnan(llhAzimuth) || std::isnan(llhZenith) )
        return false; 
  if (std::isnan(lfAzimuth) || std::isnan(lfZenith) )
        return false; 
  if (std::isnan(mjdTime) || std::isnan(ndir) || std::isnan(nstring) || std::isnan(nchan))
        return false; 
  return true;
}

template <class Archive>
void
TDST::serialize(Archive& ar,unsigned version) 
{
  ar & make_nvp("I3FrameObject", base_object<I3FrameObject>(*this));
  ar & make_nvp("LocalMST", localMST);
  if (version > 0) {
    ar & make_nvp("eqAnti", localAntiS);
  }
  ar & make_nvp("ModJulDay", mjdTime);
  ar & make_nvp("LLHAzimuthDeg", llhAzimuth);
  ar & make_nvp("LLHZenithDeg", llhZenith);
  ar & make_nvp("LFAzimuthDeg", lfAzimuth);
  ar & make_nvp("LFZenithDeg", lfZenith);
  ar & make_nvp("LinLLHOpeningAngleDeg", linllhOpeningAngle);
  ar & make_nvp("DecDeg", Dec);
  ar & make_nvp("RADeg", RA);
  ar & make_nvp("RAAntiS", RAAntiS);
  ar & make_nvp("DecAntiS", DecAntiS);
  ar & make_nvp("RASolar", RASolar);
  ar & make_nvp("DecSolar", DecSolar);
  ar & make_nvp("RASun", RASun);
  ar & make_nvp("DecSun", DecSun);
  ar & make_nvp("RAMoon", RAMoon);
  ar & make_nvp("DecMoon", DecMoon);
  ar & make_nvp("LogMuE", logMuE);
  ar & make_nvp("RLogL", rlogl);
  ar & make_nvp("SDCoG", sdcog);
  ar & make_nvp("CoG_X", cogx);
  ar & make_nvp("CoG_Y", cogy);
  ar & make_nvp("CoG_Z", cogz);
  ar & make_nvp("LDir", ldir);
  ar & make_nvp("RunId", runId);
  ar & make_nvp("NDirHits", ndir);
  ar & make_nvp("NChannels", nchan);
  ar & make_nvp("NStrings", nstring);
  ar & make_nvp("SubRunId", subrunId);
  ar & make_nvp("IsGoodLineFit", isGoodLineFit);

}


I3_SERIALIZABLE(TDST);

