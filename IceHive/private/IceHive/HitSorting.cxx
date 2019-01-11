/**
* \file HitSorting.cxx
*
* (c) 2012 the IceCube Collaboration
*
* $Id$
* \version $Revision$
* \date $Date$
* \author Marcel Zoll <marcel.zoll@fysik.su.se>
*/

#include "IceHive/HitSorting.h"

//==================namespace HitSorting===============
using namespace HitSorting;
using namespace std;

//================== class AbsHit =====================
HitSorting::AbsHit::AbsHit(const unsigned int di,
                           const double t):
  domIndex(di),
  time(t)
{};

HitSorting::AbsHit::AbsHit(const OMKey omkey,
                     const double t):
  domIndex(OMKeyHash::OMKey2SimpleIndex(omkey)),
  time(t)
{};

OMKeyHash::SimpleIndex HitSorting::AbsHit::GetDOMIndex() const {return domIndex;};
OMKey HitSorting::AbsHit::GetOMKey() const {return OMKeyHash::SimpleIndex2OMKey(domIndex);};

double HitSorting::AbsHit::GetTime() const {return time;};

bool HitSorting::AbsHit::operator== (const AbsHit& rhs) const
  {return(domIndex==rhs.domIndex && time==rhs.time);};

bool HitSorting::AbsHit::operator< (const AbsHit& rhs) const {
  if (time==rhs.time)
    return(domIndex<rhs.domIndex);
  return(time<rhs.time);
};

std::ostream& HitSorting::operator<< (std::ostream& oss, const HitSorting::AbsHit& h){
  oss << "Hit(" <<
  " omkey : " << h.GetOMKey() <<
  ", time : " << h.GetTime() << " )";
  return oss;
}

//================ CLASS Hit ===================

HitSorting::Hit::Hit():
  AbsHit(std::numeric_limits<unsigned int>::max(),NAN),
  base_obj_ptr_(0)
{};

std::ostream& HitSorting::operator<<(std::ostream& oss, const HitSorting::Hit& h) {
  oss << (AbsHit)h;
  return oss;
};

//================ Helpers ===================

//specialize the GetTime() to the ResponseObject
template <>
double HitSorting::GetInferredTime<I3RecoPulse>(const I3RecoPulse &r)
  {return r.GetTime();};

template <>
double HitSorting::GetInferredTime<I3DOMLaunch>(const I3DOMLaunch &r)
  {return r.GetStartTime();};

template <>
double HitSorting::GetInferredTime<I3MCHit>(const I3MCHit &r)
  {return r.GetTime();};

template <>
double HitSorting::GetInferredTime<I3MCPulse>(const I3MCPulse &r)
  {return r.time;};

//========== CLASS I3RecoPulseSeriesMapHitFacility =============

HitSorting::I3RecoPulseSeriesMap_HitFacility::I3RecoPulseSeriesMap_HitFacility(I3FramePtr frame, const std::string &key):
  OMKeyMap_HitFacility<I3RecoPulse>(frame, key)
{};
