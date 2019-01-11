#include "I3EnergyLossConverter.h"


I3TableRowDescriptionPtr
I3EnergyLossConverter::CreateDescription(const I3EnergyLoss &params) {
  I3TableRowDescriptionPtr desc(new I3TableRowDescription());

  //for optimal mem usage : rank from large to small
  desc->AddField<double>("eloss_1000", "GeV/m",
			 "dE/dX at slant depth X=1000m");
  desc->AddField<double>("eloss_1500", "GeV/m",
			 "dE/dX at slant depth X=1500m");
  desc->AddField<double>("eloss_1600", "GeV/m",
			 "dE/dX at slant depth X=1600m");
  desc->AddField<double>("eloss_1700", "GeV/m",
			 "dE/dX at slant depth X=1700m");
  desc->AddField<double>("eloss_1800", "GeV/m",
			 "dE/dX at slant depth X=1800m");
  desc->AddField<double>("eloss_1900", "GeV/m",
			 "dE/dX at slant depth X=1900m");
  desc->AddField<double>("eloss_2000", "GeV/m",
			 "dE/dX at slant depth X=2000m");
  desc->AddField<double>("eloss_2100", "GeV/m",
			 "dE/dX at slant depth X=2100m");
  desc->AddField<double>("eloss_2200", "GeV/m",
			 "dE/dX at slant depth X=2200m");
  desc->AddField<double>("eloss_2300", "GeV/m",
			 "dE/dX at slant depth X=2300m");
  desc->AddField<double>("eloss_2400", "GeV/m",
			 "dE/dX at slant depth X=2400m");
  desc->AddField<double>("eloss_3000", "GeV/m",
			 "dE/dX at slant depth X=3000m");

  desc->AddField<double>("a_estim", "",
			 "primary CR mass (don't take this too serious), fitparameter from the average energyloss");
  desc->AddField<double>("a_estim_err", "",
			 "error on fitparameter a_estim from TMinuit");
  desc->AddField<double>("e0_estim", "GeV",
			 "primary CR energy (don't take this too serious), fitparameter from the average energyloss");
  desc->AddField<double>("e0_estim_err", "GeV",
			 "error on fitparameter e0_estim from TMinuit");

  desc->AddField<double>("stoch_energy", "GeV/m",
			 "average energy loss of high energy stochastic peaks");
  desc->AddField<double>("rel_stoch_energy", "",
			 "average of relative height of stochastic peaks above the fitted average energyloss");
  desc->AddField<double>("total_stoch_energy", "GeV/m",
			 "total energy loss of high energy stochastic peaks");
  desc->AddField<double>("total_rel_stoch_energy", "",
			 "total of relative height of stochastic peaks above the fitted average energyloss");
  desc->AddField<double>("highest_stoch_energy", "GeV/m",
			 "highest energy loss of high energy stochastic peaks");
  desc->AddField<double>("highest_rel_stoch_energy", "",
			 "highest relative height of stochastic peaks above the fitted average energyloss");
  desc->AddField<double>("chi2", "",
			 "chi2/ndof of average energy loss fit");
  desc->AddField<double>("chi2_red", "",
			 "chi2/ndof of average energy loss fit after removal of high energy stochastic peaks");
  desc->AddField<double>("stoch_depth", "m",
			 "average slant depth of high energy stochastic peaks");
  
  desc->AddField<uint8_t>("n_he_stoch", "",
			 "number of high energy stochastic energy loss peaks");

  MAKE_ENUM_VECTOR(status,I3Particle,I3Particle::FitStatus,I3PARTICLE_H_I3Particle_FitStatus);
  //MAKE_ENUM_VECTOR(fit_status,I3Particle,I3Particle::FitStatus,I3PARTICLE_H_I3Particle_FitStatus);
  desc->AddEnumField<I3Particle::FitStatus>("fit_status",status,"",
					    "status of average energy loss fit from TMinuit");

  return desc;
}
    
size_t I3EnergyLossConverter::FillRows(const I3EnergyLoss &params,
					  I3TableRowPtr rows) {

  rows->Set<double>("eloss_1000",params.Eloss_1000);
  rows->Set<double>("eloss_1500",params.Eloss_1500);
  rows->Set<double>("eloss_1600",params.Eloss_1600);
  rows->Set<double>("eloss_1700",params.Eloss_1700);
  rows->Set<double>("eloss_1800",params.Eloss_1800);
  rows->Set<double>("eloss_1900",params.Eloss_1900);
  rows->Set<double>("eloss_2000",params.Eloss_2000);
  rows->Set<double>("eloss_2100",params.Eloss_2100);
  rows->Set<double>("eloss_2200",params.Eloss_2200);
  rows->Set<double>("eloss_2300",params.Eloss_2300);
  rows->Set<double>("eloss_2400",params.Eloss_2400);
  rows->Set<double>("eloss_3000",params.Eloss_3000);

  rows->Set<double>("a_estim", params.primMassEstimate);
  rows->Set<double>("a_estim_err", params.primEnergyEstimate);
  rows->Set<double>("e0_estim", params.primMassEstimate_err);
  rows->Set<double>("e0_estim_err", params.primEnergyEstimate_err);
  rows->Set<double>("stoch_energy", params.avStochEnergy);
  rows->Set<double>("rel_stoch_energy", params.avRelStochEnergy);
  rows->Set<double>("total_stoch_energy", params.totalStochEnergy);
  rows->Set<double>("total_rel_stoch_energy", params.totalRelStochEnergy);
  rows->Set<double>("highest_stoch_energy", params.highestStochEnergy);
  rows->Set<double>("highest_rel_stoch_energy", params.highestRelStochEnergy);
  rows->Set<double>("chi2", params.chi2);
  rows->Set<double>("chi2_red", params.chi2_red);
  rows->Set<double>("stoch_depth", params.avStochDepth);
  rows->Set<uint8_t>("n_he_stoch", params.nHEstoch);

  rows->Set<I3Particle::FitStatus> ("fit_status",params.status);

  return 1;
}


