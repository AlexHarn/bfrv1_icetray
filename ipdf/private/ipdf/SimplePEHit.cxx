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

#include <iostream>
#include "ipdf/Simple/SimplePEHit.h"

std::ostream& IPDF::operator<<(std::ostream& os, 
    const SimplePEHit& pehit) {
  return (os<<"SimplePEHit: ("
      <<"leading edge: "<<pehit.le_<<", "
      <<"pe amplitude: "<<pehit.pe_
      <<")");
}
