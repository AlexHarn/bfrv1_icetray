/**
    copyright  (C) 2004
    the icecube collaboration
    $Id$

    @version $Revision: 1.3 $
    @date $Date$
    @author C Wiebusch <wiebusch@physik.uni-wuppertal.de>
        and S Robbins  <robbins@physik.uni-wuppertal.de>

    @brief Implementation of example HitOm representation
*/

#include "ipdf/I3/I3HitOm.h"
#include "ipdf/I3/I3OmReceiver.h"
#include <iostream>

IPDF::I3HitOm::~I3HitOm()
{
  for(IPDF::I3HitOm::iterator hiter = photon_hits_.begin();
      hiter != photon_hits_.end(); ++hiter) {
    delete *hiter;
  }
  if (deleteomr_)
    delete &omr_;
}

std::ostream& IPDF::operator<<(std::ostream& os, 
    const I3HitOm& hitom) {
  os  <<"I3HitOm: (";
  if(hitom.photon_hits_.size()!=0) {
    os  <<" my PEHits: ";
    for(I3HitOm::const_iterator hiter = hitom.photon_hits_.begin();
	hiter != hitom.photon_hits_.end(); ++hiter) {
      os<<(**hiter)<<"; ";
    }
  } else {
    os<<"No PEHits; ";
  }
  return (os<<"my OmReceiver: "<<hitom.omr_<<" )");
}
