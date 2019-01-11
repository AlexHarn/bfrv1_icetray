/**
 *   copyright  (C) 2005
 *   the IceCube Collaboration
 *   $Id:  $
 *
 *   @version $Revision: $
 *   @date $Date: $
 *   @author Aya Ishihara <aya.ishihara@icecube.wisc.edu>
 *
 *   @brief Crosssection implimentation file for I3NeutrinoGeneratorCC
 *   here you find all the final state classes for processes in the
 *   Standard Model. The default data is supplied through tables.
 * 
 */

#include "neutrino-generator/Steering.h"
#include "neutrino-generator/Particle.h"
#include "neutrino-generator/interactions/InteractionBase.h"
#include "neutrino-generator/interactions/InteractionCC.h"
#include "neutrino-generator/utils/Constants.h"
#include "neutrino-generator/table-interface/CrosssectionTableReader.h"
#include "neutrino-generator/table-interface/FinalStateTableReader.h"
#include "dataclasses/I3Constants.h"

namespace nugen {

using namespace Constants;

//___________________________________________________________________
InteractionCC::InteractionCC(I3RandomServicePtr random,
                             SteeringPtr steer):
  InteractionBase(random, steer)
{
  log_debug("Constructing CC interaction");
}

//___________________________________________________________________
InteractionCC::~InteractionCC()
{
  log_trace("deconstruct SigmaCC");
}

//___________________________________________________________________
void InteractionCC::FillDaughterParticles(ParticlePtr nuin_ptr, 
                                          double)
{
  log_trace("CC::charged current interaction has been chosen to create next particles");

  if (!nuin_ptr->GetLength()) {
    log_error("Position of interaction need to be determined before selecting daughters!");
  }

  // select a secondary flavor
  I3Particle::ParticleType leptonFlavor = I3Particle::unknown;
  bool istau = false;

  switch (nuin_ptr->GetType()) {
     case I3Particle::NuE : 
        leptonFlavor = I3Particle::EMinus;   
        break;
     case I3Particle::NuEBar :
        leptonFlavor = I3Particle::EPlus; 
        break;
     case I3Particle::NuMu :
        leptonFlavor = I3Particle::MuMinus;
        break;
     case I3Particle::NuMuBar : 
        leptonFlavor = I3Particle::MuPlus;
        break;
     case I3Particle::NuTau : 
        leptonFlavor = I3Particle::TauMinus;
        istau = true;
        break;
     case I3Particle::NuTauBar :
        leptonFlavor = I3Particle::TauPlus; 
        istau = true;
        break;
     default: 
        log_error("invalid neutrino type"); 
        break;
  }

  if (istau && nuin_ptr->GetLocationType() != I3Particle::InIce && steer_->IgnoreOutgoingAngleForNC() ) {
     // this is NuTau CC interaction inside the Earth.
     // When IgnoreOutgoingAngleForNC = True, ignore outgoing angle of regenerated NuTau too.
     SetSecondaryParticles(nuin_ptr, leptonFlavor, true);
  } else {
     SetSecondaryParticles(nuin_ptr, leptonFlavor, false);
  }

  log_trace(" # We have %d daughter particles", nuin_ptr->GetTotalNDaughters());

}

}

