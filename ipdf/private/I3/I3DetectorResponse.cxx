/**
    copyright  (C) 2004
    the icecube collaboration
    $Id$

    @version $Revision: 1.3 $
    @date $Date$
    @author C Wiebusch <wiebusch@physik.uni-wuppertal.de>
        and S Robbins  <robbins@physik.uni-wuppertal.de>

    @brief Implementation of example DetectorResponse representation
*/

#include "ipdf/I3/I3DetectorResponse.h"
#include "ipdf/I3/I3HitOm.h"
#include <iostream>

#include "ipdf/I3/I3DetectorConfiguration.h"
#include "ipdf/I3/I3OmReceiver.h"
#include "ipdf/Utilities/IPdfException.h"

using namespace IPDF;

IPDF::I3DetectorResponse::~I3DetectorResponse()
{
  for(IPDF::I3DetectorResponse::iterator hiter = hitoms_.begin();
      hiter != hitoms_.end(); ++hiter) {
    delete *hiter;
  }
}

std::ostream& IPDF::operator<<(std::ostream& os,
    const I3DetectorResponse& detres) {
  os<<"I3DetectorResponse: ( "
      <<detres.size()<<" OMs hit: ";

  for(IPDF::I3DetectorResponse::const_iterator hiter = detres.hitoms_.begin();
      hiter != detres.hitoms_.end(); ++hiter) {
    os<<(**hiter)<<"; ";
  }
      
  os<<" )";
  return os;
}
