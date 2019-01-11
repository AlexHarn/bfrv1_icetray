/**
 *  copyright  (C) 2005
 *  the IceCube collaboration
 *  $Id: $
 *
 *  @version $Revision: $
 *  @date    $Date: $
 *  @author  Kotoyo Hoshina <hoshina@icecube.wisc.edu>
 *
 *  @brief   Calculator IMPLEMENTATION FILE
 */

#include "neutrino-generator/Steering.h"
#include "neutrino-generator/Particle.h"
#include "neutrino-generator/utils/Pickup.h"

namespace nugen {

class Candidate {
   public :
   Candidate(): nu(ParticlePtr()), bias_weight(1.0), survival_weight(1.0){}
   ParticlePtr nu;
   double bias_weight;
   double survival_weight;
};

//_________________________________________________________
std::pair<bool, std::pair<ParticlePtr, double> >
Pickup::PickupANeutrino(I3RandomServicePtr rnd,
                  std::vector<ParticlePtr> &particles, 
                  I3FramePtr frame,
                  bool useHE)
{
  if (particles.size() < 1) {
     log_error("particles size must be > 0");
  }

  I3MapStringDoublePtr wmap = 
    Utils::PutWeightDict(frame, particles[0]->GetSteer()->GetWeightDictName());
  double nNu = (*wmap)["NInIceNus"];
  double totalnNu = double(particles.size());
  if (nNu != 0) { 
     // NInIceNus is already stored, may be by
     // I3NuGSourceSelector.
     // In this case, multiple Nu primaries are injected.
     // Among them, only one neutrino is selected 
     //(by I3NuGSourceSelector) and
     // made interactions inside the Earth,
     // and regenerate more neutrinos.
     // It may happen when
     // 1) Corsika atmospheric neutrinos are injected
     //    and NuEBar is selected to propagete
     // 2) Tau or NuTau is generated via Grashow CC
     //    interaction
     // 3) Tau or NuTau generates more neutrinos with
     //    regeneration
     // 
     // Then, total number of arrival neutrinos is 
     //
     // nNu + particles.size() -1
     //
     // where 
     // nNu : number of primary neutrinos
     // particles.size() : for selected neutrino + 
     //                    regenerated neutrinos
     //
     // -1 is for removing double counting of the
     //  selected neutrino.
     //
     totalnNu += (nNu - 1); 
  }
  log_debug("Pickup: stored NInIceNus %f, NCandidates %f, total %f", 
             nNu, double(particles.size()), totalnNu);

  // Just in case Pickup function failed, save
  // the number to weight map directly.
  // This number will be updated by the selected 
  // inice particle, with initial value of NInIceNus=0.
  // To propagate correct NInIceNus, copy the NInIceNus 
  // to all inice neutrino candidates.
  (*wmap)["NInIceNus"] = totalnNu;
  for (unsigned int i=0; i<particles.size(); ++i) {
     particles[i]->GetInteractionInfoRef().SetNInIceNus(totalnNu);
  }

  if (useHE) {
     return PickupANeutrinoHE(rnd, particles);
  }
  return PickupANeutrinoLE(rnd, particles);
}

//_________________________________________________________
std::pair<bool, std::pair<ParticlePtr, double> >
Pickup::PickupANeutrinoHE(I3RandomServicePtr rnd,
                  std::vector<ParticlePtr> &particles)
{
  std::vector<Candidate> neutrinos;
  double noninteraction_weight = 1.;
  for (unsigned int i=0; i<particles.size(); ++i) {

     ParticlePtr nu = particles[i];

     // this line calculates total surviving prob. etc.
     const DoubleStepMap &steps = nu->GetStepMapRef();

     // get total surviving probability from last entry
     DoubleStepMap::const_iterator ii = steps.end();
     --ii;

     Step laststep = ii->second;
     double psurv = laststep.psurv_out_;

     Candidate c;
     c.nu = nu;
     c.survival_weight = psurv;
     noninteraction_weight *= c.survival_weight;

     neutrinos.push_back(c);
  }

  // select a neutrino

  double total_weight = 0;
  double total_bias_weight = 0;
  double total_survival_weight = 0;
  std::map <double, Candidate> choices;
  for (unsigned int i=0; i<neutrinos.size(); ++i) {

     Candidate &c = neutrinos[i];

     // The weight for selecting a particular neutrino should be proportional
     // to the probability that all the other neutrinos would *not* interact.
     // The probability of the chosen neutrino interacting will be part of
     // its forced-interaction weight.

     // Psurv_notinteracted
     c.survival_weight = noninteraction_weight / c.survival_weight;

     // Bias selection, use class T's function  
     c.bias_weight = SelectionWeighter(c.nu);
     if (c.bias_weight > 0) {
        total_weight += c.survival_weight*c.bias_weight;
        total_survival_weight += c.survival_weight;
        total_bias_weight += c.bias_weight;
        choices[total_weight] = c;
     }
  }
  
  if (total_weight <= 0) {
     // no candidate survived. return dummy now.
     std::pair<ParticlePtr, double> dummy(ParticlePtr(new Particle(I3Particle::Null, I3Particle::unknown, particles[0]->GetSteer())), 0.);
     return std::pair<bool, std::pair<ParticlePtr, double> >(false, dummy);
  }

  double r = rnd->Uniform(0., total_weight);
  std::map<double, Candidate>::iterator choice = choices.lower_bound(r);
  if (choice == choices.end()) {
     log_fatal("Couldn't find prob %f in (%f %f)]", r, 
                0., total_weight);
  }

  log_trace_stream("Picked " << choice->second.nu->GetEnergy() 
        << " GeV " << choice->second.nu->GetTypeString() 
        << " to interact");

  // The weight associated with picking exactly one neutrino to interact is
  // the ratio between the natural frequency (survival_weight/total_survival_weight)
  // and the weighted frequency (survival_weight*bias_weight/total_weight)
  // = total_weight / (total_suvival_weight * bias_weight)

  double selection_weight = total_weight / 
     (choice->second.bias_weight*total_survival_weight);
  log_trace_stream("psurv/total " 
        << (choice->second.survival_weight / total_survival_weight)
        << " bias/total " << (choice->second.bias_weight / total_bias_weight)
        << " selection weight " <<  selection_weight);

  std::pair<ParticlePtr, double> apair(choice->second.nu, selection_weight);
  return std::pair<bool, std::pair<ParticlePtr, double> >(true, apair);

}

//_________________________________________________________
std::pair<bool, std::pair<ParticlePtr, double> >
Pickup::PickupANeutrinoLE(I3RandomServicePtr rnd,
                  std::vector<ParticlePtr> &particles)
{
  std::vector<Candidate> neutrinos;
  std::map <double, Candidate> choices;
  double total_bias_weight = 0;
  for (unsigned int i=0; i<particles.size(); ++i) {
     Candidate c;
     c.nu = particles[i]; 
     c.bias_weight = SelectionWeighter(c.nu);
     if (c.bias_weight > 0) {
        total_bias_weight += c.bias_weight;
        choices[total_bias_weight] = c;
     }
  }

  if (total_bias_weight <= 0) {
     // no candidate survived. return dummy now.
     std::pair<ParticlePtr, double> dummy(ParticlePtr(new Particle(I3Particle::Null, I3Particle::unknown, particles[0]->GetSteer())), 0.);
     return std::pair<bool, std::pair<ParticlePtr, double> >(false, dummy);
  }

  double r = rnd->Uniform(0., total_bias_weight);
  std::map<double, Candidate>::iterator choice = choices.lower_bound(r);
  if (choice == choices.end()) {
     log_fatal("Couldn't find prob %f in (%f %f)]", r, 
                0., total_bias_weight);
  }

  log_trace_stream("Picked " << choice->second.nu->GetEnergy() 
        << " GeV " << choice->second.nu->GetTypeString() 
        << " to interact");

  // The weight associated with picking one neutrino to represent all N
  // neutrinos in the bundle is approximately
  // N_nu * p_natural / p_biased = N_nu * (1/N_nu) / p_biased = 1 / p_biased
  // = total_bias_weight / bias_weight
  // See: https://docushare.icecube.wisc.edu/dsweb/Get/Document-72536/thesis.pdf
  // Eq. 5.24
  // This is correct as long as we regards product of surviving
  // probability of all NOT SELECTED neutrinos (Psurv_notselected)
  // is nearly equal 1.0.

  double selection_weight = total_bias_weight / 
                            (choice->second.bias_weight);

  std::pair<ParticlePtr, double> apair(choice->second.nu, selection_weight);
  return std::pair<bool, std::pair<ParticlePtr, double> >(true, apair);

}

}

