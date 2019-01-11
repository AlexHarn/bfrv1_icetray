#ifndef I3NEUTRINOGENERATOR_BASE_H
#define I3NEUTRINOGENERATOR_BASE_H
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

#include "phys-services/I3RandomService.h"
#include "dataclasses/physics/I3MCTree.h"
#include "neutrino-generator/legacy/I3NuGInteractionInfo.h"
#include "neutrino-generator/legacy/I3NuGInjector.h"
#include "neutrino-generator/legacy/I3NuGVEvent.h"
#include "neutrino-generator/utils/EnumTypeDefs.h"
#include "earthmodel-service/EarthModelService.h"

namespace nugen {
I3_FORWARD_DECLARATION(Steering);
I3_FORWARD_DECLARATION(Particle);
}

class I3NeutrinoGeneratorBase
{

 public:
  I3NeutrinoGeneratorBase(nugen::PropagationMode propmode, 
              int intpos_sample_opt = nugen::Defaults::intpos_sample_opt,
              int interact_weight_opt = nugen::Defaults::interact_weight_opt, 
              int crosssection_cdep_opt = nugen::Defaults::crosssectionxcolumndepth_opt, 
              int impactparam_opt = nugen::Defaults::impactparam_opt) ;

  virtual ~I3NeutrinoGeneratorBase() {}
  

  /**
   * Propagation functions
   */
  void PropagateAll(boost::shared_ptr<nugen::Particle> p, I3FramePtr frame);
  I3Map<I3ParticleID, double> PropagateInEarth(boost::shared_ptr<nugen::Particle> p, 
                                  I3FramePtr frame, 
                                  bool save_result = true);

  /**
   * This function is used for pybinding test.
   */
  I3Map<I3ParticleID, double> PropagateInEarthWrapper(I3ParticlePtr p, I3FramePtr frame);

  void SetRandomNumberGenerator(I3RandomServicePtr random);
  void SetPropagationMode(nugen::PropagationMode m) { prop_mode_ = m; }
  nugen::PropagationMode GetPropagationMode() const { return prop_mode_; }

  void SetInteractionPositionSamplingOption(int v) { intpos_sample_opt_ = v;}
  int  GetInteractionPositionSmaplingOption() { return intpos_sample_opt_; }
  void SetInteractionWeightOption(int v) {interact_weight_opt_ = v;}
  int  GetInteractionWeightOption() { return interact_weight_opt_; }
  void SetCrosssectionxColumndepthOption(int v) {crosssectionxcolumndepth_opt_ = v;}
  int  GetCrosssectionxColumndepthOption() { return crosssectionxcolumndepth_opt_;}
  void SetImpactParamOption(int v) {impactparam_opt_ = v;}
  int  GetImpactParamOption() { return impactparam_opt_;}

  void SetSkipFinalInteraction(bool b) { skip_final_interaction_ = b; }
  bool SkipFinalInteraction() const { return skip_final_interaction_; }

  virtual void PrintSetting();

 private:
  /**
   *private constructors 
   */
  I3NeutrinoGeneratorBase() {};
  I3NeutrinoGeneratorBase& operator=(const I3NeutrinoGeneratorBase&);

 protected:
  //// Following 4 members are common through out the run ////
  /**
   * number of generated events in a run
   * since icetray does not know how many events are going to be generated
   * and since this number is useful to calculate the Oneweight, we input
   * it as a parameter.
  **/
  long number_of_events_;
  /**
   *Number of events so far injected in a run
   *this is the EventID stored in I3EventHeader
   */
  long eventCounter_;

  /**
   *Decide if any memory info printed
   */
  bool doPrintMemoryInfo_;

  /**
   * Core function to propagate neutrino
   * This function generates I3NuGVEvent, and process
   * MakeNeutrinoPropagation.
   * @return I3NuGVEventPtr
   */
  std::pair<bool, I3NuGVEventPtr>
  PropagateNu(boost::shared_ptr<nugen::Particle> injected_nu,
                             I3FramePtr frame,
                             bool skip_final_interaction=false);

  /**
   * fill MCTree and MCWeightDict
   * @return void
   */
  void  FillMC(I3NuGVEventPtr, I3FramePtr);

 /**
   * Check function
   * @return void
   * @param  I3MCTreePtr
   */
  void  CheckInIceParticle(I3MCTreePtr);

  unsigned int event_year_;
  unsigned int event_daqtime_;
  unsigned int runid_;

  I3RandomServicePtr random_;
  nugen::SteeringPtr steer_;
  earthmodel::EarthModelServicePtr earth_;
  I3NuGInjectorPtr injector_;
  I3NuGInteractionInfoPtr interaction_;
  std::string steering_name_;
  std::string injector_name_;
  std::string interaction_name_;
  std::string pointSourceDirectionName_;
  std::string primary_nu_name_;

  double interaction_cc_factor_; // interaction weighting factor for CC
  double interaction_nc_factor_; // interaction weighting factor for NC
  double interaction_gr_factor_; // interaction weighting factor for GR

  /*
   * Propagation Mode
   *
   * nugen::LEGACY       : use legacy Event.
   * nugen::NOPROPWEIGHT : no propagation weight with WeightedEvent. 
   *              Some neutrino may be absorbed by the Earth. 
   * nugen::NCGRWWEIGHTED : CC interaction is forbidden and 
   *              always NC or GR interacion is chosen if
   *              interactions occurred during propagation.
   *              Propagation weight is applied.
   *              No neutrino is absorbed by the Earth.
   * !! CAUTION !! for Tau gen, use NoWeihgt option to 
   *               activate tau regeneration.
   */
  nugen::PropagationMode prop_mode_; 

  /*
   * option for sampling method for interaction position
   * 0 : sample flat in length [m]
   * 1 : sample flat in columnd depth [g/m2]
   * see Particle::SetFinalInteractionPosition
   */
  int    intpos_sample_opt_; 

  /*
   * option for interaction weight calculation
   * 0 : use (total_crosssection * 1.0e-31) * 
   *         (total_column_depth / PMASS) * Psurv;  (legacy)
   * 1 : use (1 - Psurv)*norm*Psurv
   * where
   * Psurv = surviving probability from entrance of detection volume
   *          to interaction position
   *  1-Psurv = interaction probability within detection volume
   * norm = normalization factor of Psurv function
   * norm * Psurv compensates linear sampling of interaction vertex
   * in meter.
   * see Particle::SetFinalInteractionPosition
   */
  int    interact_weight_opt_;

  /*
   * option for calculationg crosssection * columndepth
   * 0 : get total columndepth and multiply crosseection at 
   *     the specified point (legacy)
   * 1 : calculate columndepth * crosssection at each step and 
   *     accumulate them over total steps
   * see WeightedEvent::SetFinalInteractionPosition and
   *     WeightedEvent::CalculateMaximumTotalInteractionProbability
   */
  int    crosssectionxcolumndepth_opt_; 

  /* 
   * daughters or not
   * 0 : always use primary's impact parameters (legacy)
   * 1 : re-calculate for each daughters
   * see Calculator::SetFinalInteractionPosition and
   * WeightedEvent::CalculateInteractionGeometry
   */ 
  int    impactparam_opt_;  // control impact param calculation

  /*
   * do you want to skip final interaction?
   */ 
  bool   skip_final_interaction_; 

  SET_LOGGER("I3NuG");

};

#endif //I3NEUTRINOGENERATOR_BASE_H
