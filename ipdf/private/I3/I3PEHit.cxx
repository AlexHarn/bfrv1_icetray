/**
    copyright  (C) 2004
    the icecube collaboration
    $Id$

    @version $Revision: 1.1 $
    @date $Date$
    @author C Wiebusch <wiebusch@physik.uni-wuppertal.de>
        and S Robbins  <robbins@physik.uni-wuppertal.de>

    @brief Implementation of example PEHit representation
*/

#include "ipdf/I3/I3PEHit.h"
#include <iostream>

std::ostream& IPDF::operator<<(std::ostream& os, 
    const IPDF::I3PEHit& pehit) {
  return (os<<"I3PEHit: ("
      <<"leading edge: "<<pehit.le_<<", "
      <<"pe amplitude: "<<pehit.pe_
      <<")");
}
