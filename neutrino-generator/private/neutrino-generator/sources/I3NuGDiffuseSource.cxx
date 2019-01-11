/** 
 *  $Id: $ *  @file I3NuGDiffuseSource.cxx *  @version $Revision:  $
 *  @date    $Date:      $
 *
 *  Copyright (C) 2005
 *  the IceCube Collaboration <http://www.icecube.wisc.edu>
 *  Aya Ishihara  <aya.ishihara@icecube.wisc.edu>
 *  modified by Kotoyo Hoshina <hoshina@icecube.wisc.edu>
 */


#include "neutrino-generator/Particle.h"
#include "neutrino-generator/Steering.h"
#include "neutrino-generator/sources/I3NuGDiffuseSource.h"
#include "neutrino-generator/sources/SourceUtils.h"
#include "neutrino-generator/utils/Defaults.h"
#include "neutrino-generator/utils/Utils.h"
#include "neutrino-generator/utils/Calculator.h"
#include "neutrino-generator/utils/ZenithSampler.h"

using namespace std;
using namespace nugen;
using namespace earthmodel;

I3_MODULE(I3NuGDiffuseSource);

//______________________________________________________________
I3NuGDiffuseSource::I3NuGDiffuseSource(const I3Context& ctx) : 
  I3ConditionalModule(ctx), 
  number_of_events_(0),
  eventCounter_(0),
  steer_name_(Defaults::steerName), 
  outNuName_(Defaults::primaryNuName),
  gammaIndex_(2.0),
  energyMinLog_(2.0),
  energyMaxLog_(8.0),
  zenithMin_(0.*I3Units::deg),
  zenithMax_(180*I3Units::deg),
  coszenMin_(cos(zenithMax_)),
  coszenMax_(cos(zenithMin_)),
  azimuthMin_(0.*I3Units::deg),
  azimuthMax_(360*I3Units::deg),
  zenith_weight_param_(1.0),
  zenith_sampling_str_(Defaults::angleSamplingModeString),
  zenith_sampling_(Defaults::angleSamplingMode),
  flavor_("NuMu"),
  nutypes_(),
  ratio_vec_(2,1)
{
  AddParameter("RandomService", "pointer to random service", random_);
  AddParameter("SteeringName", "name of steering service", steer_name_);

  AddParameter("NuFlavor", "DEPRECATED: Set injection flavor. If you want to generate different flavors in one dataset(for example, NuE+NuMu etc.), use NuTypes", flavor_);
  AddParameter("NuTypes", "vector of name of injection neutrinos, anti-particles or even other flavors are counted separately. e.g. ['NuMu','NuMuBar','NuTau','NuTauBar'].If you set this parameter, NuFlavor parameter is ignored.", nutypes_);	
  AddParameter("PrimaryTypeRatio", "ratio of injected nu types, must be same size of NuTypes and order of the numbers must match with the NuTypes vector.",ratio_vec_);	

  AddParameter("OutNuName", "output neutrino name", outNuName_);
  AddParameter("GammaIndex","power law index", gammaIndex_);	
  AddParameter("EnergyMinLog", "min energy log10", energyMinLog_);
  AddParameter("EnergyMaxLog", "max energy log10", energyMaxLog_);
  AddParameter("ZenithMin", "zenith min", zenithMin_);
  AddParameter("ZenithMax", "zenith max", zenithMax_);
  AddParameter("AzimuthMin", "azimuth min", azimuthMin_);
  AddParameter("AzimuthMax", "azimuth max", azimuthMax_);  
  AddParameter("ZenithWeightParam", 
               "option for zenith-weighted generation. "
               "default 1.0 gives a flat uniform weight. "
               "set 0.1 <= alpha <= 1.9. With a larger alpha "
               "you get more vertically upgoing events.",
               zenith_weight_param_);
  AddParameter("AngleSamplingMode", 
               "do you want to sample zenith in cos(zen) or zenith? ",
               zenith_sampling_str_);

  AddOutBox("OutBox");
}

//______________________________________________________________
void I3NuGDiffuseSource::Configure()
{ 

  GetParameter("RandomService",    random_);
  if (!random_) {
     random_ = context_.Get<I3RandomServicePtr>();
     if (!random_) {
       log_fatal("No random service installed.");
     }
  }

  GetParameter("SteeringName", steer_name_);
  steer_ptr_ = GetService<SteeringPtr>(steer_name_);
  if (!steer_ptr_) {
     log_fatal("You have to add I3NuGSteerService in advance!");
  }

  earth_ptr_ = steer_ptr_->GetEarthModelService();
  if (!earth_ptr_) {
     log_fatal("EarthModelService doesn't exist! "
               "Check name in nugen::Steer and make sure "
               "if you add EarthModelService in advance.");
  }

  GetParameter("NuTypes", nutypes_);	
  if (nutypes_.size() == 0) {
     GetParameter("NuFlavor", flavor_);	
     nutypes_.push_back(flavor_);
     nutypes_.push_back(flavor_+"Bar");
  }

  GetParameter("PrimaryTypeRatio", ratio_vec_);	
  if (nutypes_.size() != ratio_vec_.size()) {
     log_error("Vector size of NuTypes and PrimaryTypeRatio must be same");
  }

  type_map_.clear();
  for (unsigned int i = 0;  i<nutypes_.size(); ++i) {
      type_map_[Utils::GetParticleType(nutypes_[i])] = ratio_vec_[i];
  }

  map<I3Particle::ParticleType, double>::iterator ii;
  for (ii = type_map_.begin(); ii != type_map_.end(); ++ii) {
     log_debug("InjectionNuType %d, weight %f", ii->first , ii->second);
  }

  GetParameter("OutNuName",    outNuName_);
  GetParameter("GammaIndex",   gammaIndex_);
  GetParameter("EnergyMinLog", energyMinLog_);
  GetParameter("EnergyMaxLog", energyMaxLog_);
  GetParameter("ZenithMin",    zenithMin_);
  GetParameter("ZenithMax",    zenithMax_);
  GetParameter("AzimuthMin",   azimuthMin_);
  GetParameter("AzimuthMax",   azimuthMax_);  

  if (zenithMin_ == zenithMax_) {
      log_warn("ZenithMin and ZenithMax are the same value. SolidAngle factor will be set to 1.0.");
  } 
  if (azimuthMin_ == azimuthMax_) {
      log_warn("AzimuthMin and AzimuthMax are the same value. SolidAngle factor will be set to 1.0.");
  }

  GetParameter("ZenithWeightParam",  zenith_weight_param_);
  GetParameter("AngleSamplingMode", zenith_sampling_str_);

  zenith_sampling_ = ToAngleSamplingMode(zenith_sampling_str_);

  // prepare zenith sampler, used with ANGEMU mode 
  coszenMin_ = cos(zenithMax_);
  coszenMax_ = cos(zenithMin_);
  flat_zen_emulator_.Initialize(coszenMin_, coszenMax_);

  // initialize event counter etc.
  number_of_events_ = steer_ptr_->GetNGen();
  eventCounter_ = 0;
}

//___________________________________________________________________
void I3NuGDiffuseSource::DAQ(I3FramePtr frame)
{

  log_trace("+++++++++++++++++++++++++++++++++++++++++++++");
  log_trace("+ I3NuGDiffuseSource DAQ is called  for event %d +", eventCounter_);
  log_trace("+++++++++++++++++++++++++++++++++++++++++++++");

  if( eventCounter_ >= number_of_events_ ) {
     RequestSuspension();
     return;
  }

  if (! random_) {
     random_ = GetService<I3RandomServicePtr>();
  }

  // generate primary neutrino
  ParticlePtr nu = 
         SourceUtils::GeneratePrimaryNeutrino(type_map_, 
                                              random_, steer_ptr_);

  // set energy
  if (!SourceUtils::SetPowerLowEnergy(nu, gammaIndex_, 
                                      energyMinLog_,
                                      energyMaxLog_, random_)) {
     log_error("failed to set energy");
     return;
  }

  // set direction 
  if (!SetDirection(nu)) {
     log_error("failed to set direction");
     return;
  }

  // set position
  if (!SourceUtils::SetNuPositions(nu,earth_ptr_,random_,steer_ptr_)) {
     log_error("failed to set position");
     return;
  }

  // make MCWeightDict and put it to frame
  string weightname = steer_ptr_->GetWeightDictName();
  I3MapStringDoublePtr wmap = 
         Utils::PutWeightDict(frame, weightname);

  // fill primary generation information to wmap
  SourceUtils::FillPrimaryInfo(nu, wmap);
  SourceUtils::FillDiffusePrimaryGenInfo(nu, 
                                  steer_ptr_,
                                  gammaIndex_,
                                  energyMinLog_,
                                  energyMaxLog_,
                                  zenithMin_,
                                  zenithMax_,
                                  azimuthMin_,
                                  azimuthMax_,
                                  wmap);

  // put particle and its weights(par particle info) to frame
  Utils::PutParticle(frame, nu, outNuName_, wmap);

  // put particle and its weights(par particle info) to frame
  string treename = steer_ptr_->GetMCTreeName();
  Utils::PutMCTree(frame, nu, treename);

  PushFrame(frame,"OutBox");  

  log_trace("+++++++++++++++++++++++++++++++++++++++++++++");
  log_trace("+ primary energy log10 %f is created", log10(nu->GetEnergy()));
  log_trace("+++++++++++++++++++++++++++++++++++++++++++++");

  ++eventCounter_;
  return;

}

//_________________________________________________________________
bool I3NuGDiffuseSource::SetDirection(ParticlePtr nu)
{
  log_trace("producing a diffuse source");

  double Zenith=0.0, Azimuth=0.0, cosZenith=0.0;
  double weight = 1.0;

  // get zenith

  if (zenithMin_ == zenithMax_) {
     Zenith = zenithMin_;
  } else {
     double random = random_->Uniform(0, 1.0); 
     if (zenith_sampling_ == COS) {
        std::vector<double> result = 
           ZenithSampler::SimpleSlopeSampler(
                 zenith_weight_param_,
                 coszenMin_, coszenMax_, random); 
        cosZenith = result[0];
        weight = result[1];
        Zenith = acos(cosZenith); 

     } else if (zenith_sampling_ == ANG) {
        std::vector<double> result = 
           ZenithSampler::SimpleSlopeSampler(
                 zenith_weight_param_,
                 zenithMin_, zenithMax_, random); 
        Zenith = result[0]; 
        weight = result[1]; 

     } else if (zenith_sampling_ == ANGEMU) {
        std::vector<double> result = 
           flat_zen_emulator_.Sampling(random);
        cosZenith = result[0]; 
        weight = result[1]; 
        Zenith = acos(cosZenith); 

     } else {
        log_error("invalid zenith sampling type");
     }
  }

  // get azimuth

  if (azimuthMin_ == azimuthMax_) {
     Azimuth = azimuthMin_;
  } else {
     Azimuth = random_->Uniform(azimuthMin_, azimuthMax_);
  }

  // set direction
  nu->SetDir(Zenith, Azimuth);

  I3Direction direction(nu->GetDir());
  log_trace("injected to (zenith, azimuth)=(%f,%f), "
            "(theta,phi) = (%f,%f)[rad]",
        Zenith, Azimuth, direction.CalcTheta(),direction.CalcPhi()); 


  // set direction weight. 

  nu->GetInteractionInfoRef().SetDirectionWeight(weight);

  return true;

}

