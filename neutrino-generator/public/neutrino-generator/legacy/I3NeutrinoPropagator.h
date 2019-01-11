#ifndef I3NEUTRINOPROPAGATOR_H
#define I3NEUTRINOPROPAGATOR_H
/**
 *  copyright  (C) 2005
 *  the IceCube collaboration
 *  $Id: $
 *
 *  @version $Revision: $
 *  @date    $Date: $
 *  @author  Aya Ishihara <aya.ishihara@icecube.wisc.edu>
 *
 * @brief I3NeutrinoGenerator header file
 * This class fills dataclasses with the event information 
 * obtained from the neutrino-generator simulation based on ANIS
 * 
 */

#include "sim-services/I3PropagatorService.h"
#include "neutrino-generator/legacy/I3NeutrinoGeneratorBase.h"

namespace nugen {
I3_FORWARD_DECLARATION(Steering);
I3_FORWARD_DECLARATION(Particle);
}

class I3NeutrinoPropagator : public I3PropagatorService,
                             public I3NeutrinoGeneratorBase
{
 public:
   I3NeutrinoPropagator(
              I3RandomServicePtr random, 
              nugen::SteeringPtr steer,
              I3NuGInteractionInfoPtr interaction);

   I3NeutrinoPropagator(
              I3RandomServicePtr random, 
              nugen::SteeringPtr steer,
              I3NuGInteractionInfoPtr interaction,
              nugen::PropagationMode propmode,
              int intpos_sample_opt = nugen::Defaults::intpos_sample_opt,
              int interact_weight_opt = nugen::Defaults::interact_weight_opt, 
              int crosssectionxcdep_opt = nugen::Defaults::crosssectionxcolumndepth_opt, 
              int impactparam_opt = nugen::Defaults::impactparam_opt) ;


   void SetRandomNumberGenerator(I3RandomServicePtr r);
   std::vector<I3Particle> Propagate(I3Particle& incident_nu, DiagnosticMapPtr d, I3FramePtr frame);

 SET_LOGGER("I3NuG");

};

#endif //I3NEUTRINOPROPAGATOR_H
