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

#include <CoincSuite/lib/HitSorting.h>

//==================namespace HitSorting===============
HitSorting::Hit::Hit(const unsigned int di,
                     const unsigned int pi,
                     const double t,
                     const double c):
  domIndex(di),
  pulseIndex(pi),
  time(t),
  charge(c)
{};

HitSorting::Hit::Hit(const OMKey omkey,
                     const unsigned int pi,
                     const double t,
                     const double c):
  domIndex(OMKeyHash::OMKey2SimpleIndex(omkey)),
  pulseIndex(pi),
  time(t),
  charge(c)
{};

bool HitSorting::Hit::timeOrdered::operator() (const Hit& h1, const Hit& h2) const{
  if (h1.time!=h2.time)
    return(h1.time<h2.time);
  if (h1.domIndex!=h2.domIndex)
    return(h1.domIndex<h2.domIndex);
  return(h1.pulseIndex<h2.pulseIndex);
};

bool HitSorting::Hit::retrievalOrdered::operator() (const Hit& h1, const Hit& h2) const{
  if (h1.domIndex!=h2.domIndex)
    return(h1.domIndex<h2.domIndex);
  return(h1.pulseIndex<h2.pulseIndex);
};

bool HitSorting::operator== (const Hit& h1, const Hit& h2)
  {return(h1.domIndex==h2.domIndex && h1.pulseIndex==h2.pulseIndex);};

bool HitSorting::operator!= (const Hit& h1, const Hit& h2)
  {return(h1.domIndex!=h2.domIndex || h1.pulseIndex!=h2.pulseIndex);};

bool HitSorting::operator< (const Hit& h1, const Hit& h2) {
  if (h1.time!=h2.time)
    return(h1.time<h2.time);
  if (h1.domIndex!=h2.domIndex)
    return(h1.domIndex<h2.domIndex);
  return(h1.pulseIndex<h2.pulseIndex);
};

std::ostream& HitSorting::operator<< (std::ostream& oss, const HitSorting::Hit& h){
  oss << "Hit(" <<
  " domIndex : " << h.domIndex << "("<< OMKeyHash::SimpleIndex2OMKey(h.domIndex)<<")" <<
  ", pulseIndex : "<< h.pulseIndex <<
  ", time : " << h.time <<
  ", charge : " << h.charge << " )";
  return oss;
}

bool HitSorting::RetrievalOrdered (const Hit& h1, const Hit& h2) {
  if (h1.domIndex!=h2.domIndex)
    return(h1.domIndex<h2.domIndex);
  return(h1.pulseIndex<h2.pulseIndex);
};


//==============================================================