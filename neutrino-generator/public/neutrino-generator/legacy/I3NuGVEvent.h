#ifndef I3NuGVEVENT_H
#define I3NuGVEVENT_H

/**
 *   Copyright  (C) 2005
 *   The IceCube Collaboration
 *   $Id: $
 *
 *   @file I3NuGVEvent.h
 *   @version $Revision: $
 *   @date    $Date:     $
 *   @author Kotoyo Hoshina 
 *
 *   @brief interface pure virtual class for event classes
 *   all event classes must inherit it.
 */

#include "neutrino-generator/Steering.h"
#include "neutrino-generator/Particle.h"
#include "neutrino-generator/legacy/I3NuGInjector.h"
#include "neutrino-generator/utils/Utils.h"
#include "neutrino-generator/utils/Defaults.h"
#include "neutrino-generator/utils/Calculator.h"
#include "dataclasses/I3Map.h"
#include "icetray/I3Frame.h"

class I3NuGVEvent {

 public:
  
  I3NuGVEvent(nugen::PropagationMode propmode, 
              int intpos_sample_opt = nugen::Defaults::intpos_sample_opt,
              int interact_weight_opt = nugen::Defaults::interact_weight_opt, 
              int crosssection_cdep_opt = nugen::Defaults::crosssectionxcolumndepth_opt, 
              int impactparam_opt = nugen::Defaults::impactparam_opt) : 
              propagationMode_(propmode),
              intpos_sample_opt_(intpos_sample_opt),
              interact_weight_opt_(interact_weight_opt),
              crosssectionxcolumndepth_opt_(crosssection_cdep_opt),
              impactparam_opt_(impactparam_opt),
              eventCounter_(0) {}
  virtual ~I3NuGVEvent() {}

  virtual bool MakeNeutrinoPropagation(nugen::ParticlePtr initial, 
                                       I3FramePtr frame,
                                       bool skipFinalInteraction = false) = 0;
  virtual void WhoAmI() { std::cerr << "I'm I3NuGVEvent" << std::endl; }
  virtual void SetEventCount(int i) { eventCounter_ = i; }
  //virtual void SetInjector(I3NuGInjectorPtr i) { injector_ptr_ = i; }

  //
  //Accessing Functions to the members
  //

  /**
   * Get ParticleVect of particle created during propagation
   */
  virtual const nugen::ParticlePtrList&  GetInEarthPropagatingParticleList() const { return propagating_list_;};
  virtual nugen::ParticlePtrList&        GetInEarthPropagatingParticleList() { return propagating_list_;};

  const nugen::ParticlePtrList& GetFinalInteractionNuCandidates() const
         { return finalInteractionNuCandidates_; }
  nugen::ParticlePtrList& GetFinalInteractionNuCandidates()
         { return finalInteractionNuCandidates_; }

  /**
   *Get final charged leptons which will propagate in final volume
   */
  virtual const nugen::ParticlePtr GetInIceParticle() const { return inice_ptr_;}
  virtual nugen::ParticlePtr       GetInIceParticle()       { return inice_ptr_;}

  /*
   * Getter for buffer_list_
   */
   const nugen::ParticlePtrList& GetPossibleParentFinalParticleList() const { return buffer_list_;};
   nugen::ParticlePtrList& GetPossibleParentFinalParticleList() { return buffer_list_;};

  virtual const I3MapStringDoublePtr GetMCWeightDictPtr() const { return weightdict_ptr_; }
  virtual I3MapStringDoublePtr GetMCWeightDictPtr() { return weightdict_ptr_; }

  // setters
  void SetInIceParticle(nugen::ParticlePtr ptr){ptr->SetLocationType(I3Particle::InIce); inice_ptr_=ptr; }
  void SetMCWeightDictPtr(I3MapStringDoublePtr ptr){weightdict_ptr_=ptr;}

  virtual void FillMCWeights(I3FramePtr frame);

  //need to implement
  virtual void PrintEvent() = 0;
  virtual int  GetTotalNumberOfInEarthDaughters() = 0;
  virtual void CheckFilledMCWeights() = 0;

  protected:

  /**
   * Propagation Mode
   *
   * nugen::LEGACY       : use legacy Event.
   * nugen::NOPROPWEIGHT : no propagation weight with WeightedEvent. 
   *               Some neutrino may be absorbed by the Earth. 
   * nugen::NCGRWWEIGHTED : CC interaction is forbidden and 
   *               always NC or GR interacion is chosen if
   *               interactions occurred during propagation.
   *               Propagation weight is applied.
   *               No neutrino is absorbed by the Earth.
   * !! CAUTION !! for Tau gen, use NoWeihgt option to activate 
   * tau regeneration.
   */
  nugen::PropagationMode propagationMode_;

  /**
   parameter used to be stored in StaticParams
   option for sampling method for interaction position
   0 : sample flat in length [m]
   1 : sample flat in columnd depth [g/m2]
   see Particle::SetFinalInteractionPosition
   */
  int intpos_sample_opt_;

  /**
   parameter used to be stored in StaticParams
   option for interaction weight calculation
   0 : use (total_crosssection * 1.0e-31) * 
        (total_column_depth / PMASS) * Psurv;  (legacy)
   1 : use (1 - Psurv)*norm*Psurv
   where
   Psurv = surviving probability from entrance of detection volume
        to interaction position
   1-Psurv = interaction probability within detection volume
   norm = normalization factor of Psurv function
   norm * Psurv compensates linear sampling of interaction vertex
   in meter.
   see I3NuGWeightedEvent::SetFinalInteractionPosition
  */
  int interact_weight_opt_;

  /**
   parameter used to be stored in StaticParams
   option for calculationg crosssection * columndepth
   0 : get total columndepth and multiply crosseection at 
       the specified point (legacy)
   1 : calculate columndepth * crosssection at each step and 
       accumulate them over total steps
   see WeightedEvent::SetFinalInteractionPosition and
       WeightedEvent::CalculateMaximumTotalInteractionProbability
   */
  int crosssectionxcolumndepth_opt_;

  /**
   parameter used to be stored in StaticParams
   option to choose whether re-calculate impact param for each
   daughters or not
   0 : always use primary's impact parameters (legacy)
   1 : re-calculate for each daughters
   see Calculator::SetFinalInteractionPosition and
   */
  int impactparam_opt_;

  int eventCounter_;
  I3MapStringDoublePtr  weightdict_ptr_;

  nugen::ParticlePtrList finalInteractionNuCandidates_;

  /**
   * In NuGen "InIceParticle" represents a final neutrino,
   * but not a charged particle
   */
  boost::shared_ptr<nugen::Particle>   inice_ptr_;             
  boost::shared_ptr<nugen::Steering>   steer_ptr_;
  nugen::ParticlePtrList      propagating_list_;          
  nugen::ParticlePtrList      buffer_list_;            

  private:
  // default constructor is forbidden to use
  I3NuGVEvent() :
              propagationMode_(nugen::Defaults::propagationMode),
              intpos_sample_opt_(-1),
              interact_weight_opt_(-1),
              crosssectionxcolumndepth_opt_(-1),
              impactparam_opt_(-1),
              eventCounter_(0) {}


};

typedef boost::shared_ptr<I3NuGVEvent> I3NuGVEventPtr;

#endif //I3NuGVEVENT_H
