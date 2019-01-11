/**
 *  copyright  (C) 2005
 *  the IceCube collaboration
 *  $Id: $
 *
 *  @version $Revision: $
 *  @date    $Date: $
 *  @author  Kotoyo Hoshina <hoshina@icecube.wisc.edu>
 *
 *  @brief   Constants IMPLEMENTATION FILE
 */

#include "neutrino-generator/utils/Constants.h"
#include "neutrino-generator/utils/Utils.h"

namespace nugen {

double Constants::GetMass(I3Particle::ParticleType p) {

  // ignore neutrino mass.
  if (nugen::Utils::IsNu(p)) return 0;

  switch (p) {
     case I3Particle::EMinus: 
        return M_e;
     case I3Particle::EPlus:
        return M_e;
     case I3Particle::MuMinus:
        return M_MU;
     case I3Particle::MuPlus: 
        return M_MU;
     case I3Particle::TauMinus: 
        return M_TAU;
     case I3Particle::TauPlus:
        return M_TAU;
     case I3Particle::PiMinus:
        return M_pion;
     case I3Particle::PiPlus:
        return M_pion;
     case I3Particle::Pi0:
        return M_pion;
     default: 
        log_fatal("invalid neutrino type of %d", int(p)); 
  }
  return -1;
}

double Constants::EVm2toCM2(double length_in_eVm2)
{
   static const double eVm1_to_m1 = (HBAR*C_LIGHT / EV);
   static const double eVm2_to_cm2 = (eVm1_to_m1*eVm1_to_m1*1e4);
   return length_in_eVm2 * eVm2_to_cm2;
}

}


