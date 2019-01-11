/**
 *   copyright  (C) 2005
 *   the IceCube Collaboration
 *   $Id:  $
 *
 *   @version $Revision: $
 *   @date $Date: $
 *   @author Aya Ishihara <aya.ishihara@icecube.wisc.edu>
 *
 *   @brief Crosssection implimentation file for I3NeutrinoGeneratorNC
 *   here you find all the final state classes for processes in the
 *   Standard Model. The default data is supplied through tables.
 * 
 */
//////////////////////////////////////////////////////////////////////// 
#include "neutrino-generator/interactions/InteractionBase.h"
#include "neutrino-generator/interactions/InteractionNC.h"
#include "neutrino-generator/Particle.h"
#include "neutrino-generator/Steering.h"
#include "neutrino-generator/table-interface/CrosssectionTableReader.h"
#include "neutrino-generator/table-interface/FinalStateTableReader.h"
#include "dataclasses/I3Constants.h"

namespace nugen {

////////////////////////////////////////////////////////////////////////
InteractionNC::InteractionNC(I3RandomServicePtr random,
                             SteeringPtr steer):
  InteractionBase(random, steer)
{
   log_debug("Constructing NC interaction");
}
////////////////////////////////////////////////////////////////////////
InteractionNC::~InteractionNC()
{
}

//____________________________________________________________________
void InteractionNC::FillDaughterParticles(ParticlePtr nuin_ptr, 
                                          double)
{

  log_trace("NC::neutral current interaction has been chosen to create next particles");

  if (!nuin_ptr->GetLength()) {
    log_error("Position of interaction need to be determined before selecting daughters!");
  }

  // select a secondary flavor
  I3Particle::ParticleType leptonFlavor = nuin_ptr->GetType();

  // Do we ignore outgoing costheta angle of secondary neutrino?
  // By default it's true because even a small deviation result in missing target (=icecube).
  // If we do it correctly, we have to inject neutrinos from large area.
  //
  if (steer_->IgnoreOutgoingAngleForNC() && nuin_ptr->GetLocationType() != I3Particle::InIce) {
     SetSecondaryParticles(nuin_ptr, leptonFlavor, true);
  } else {
     SetSecondaryParticles(nuin_ptr, leptonFlavor, false);
  }

  log_trace(" # We have %d daughter particles", nuin_ptr->GetTotalNDaughters());

}

}
