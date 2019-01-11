#include <icetray/serialization.h>
#include "stochastics/I3EnergyLoss.h"

//Serialization
template <class Archive>
void I3EnergyLoss::serialize(Archive& ar, unsigned version)
{
  if ( version > i3energyloss_version_)
    log_fatal("Attempting to read version %u from file but running version %u of I3EnergyLoss class.",version,i3energyloss_version_);

  //Must serialize Base Object
  ar & make_nvp("I3FrameObject", base_object<I3FrameObject>(*this));

  //Serialize the output parameters
  ar & make_nvp("Eloss_1000",Eloss_1000);
  ar & make_nvp("Eloss_1500",Eloss_1500);
  ar & make_nvp("Eloss_1600",Eloss_1600);
  ar & make_nvp("Eloss_1700",Eloss_1700);
  ar & make_nvp("Eloss_1800",Eloss_1800);
  ar & make_nvp("Eloss_1900",Eloss_1900);
  ar & make_nvp("Eloss_2000",Eloss_2000);
  ar & make_nvp("Eloss_2100",Eloss_2100);
  ar & make_nvp("Eloss_2200",Eloss_2200);
  ar & make_nvp("Eloss_2300",Eloss_2300);
  ar & make_nvp("Eloss_2400",Eloss_2400);
  ar & make_nvp("Eloss_3000",Eloss_3000);

  ar & make_nvp("primMassEstimate",primMassEstimate); 
  ar & make_nvp("primEnergyEstimate",primEnergyEstimate);
  ar & make_nvp("primMassEstimate_err",primMassEstimate_err); 
  ar & make_nvp("primEnergyEstimate_err",primEnergyEstimate_err);
  
  ar & make_nvp("nHEstoch",nHEstoch);
  ar & make_nvp("avStochEnergy",avStochEnergy);
  if (version >= 1){
    ar & make_nvp("totalStochEnergy",totalStochEnergy);
    ar & make_nvp("highestStochEnergy",highestStochEnergy);
  }
  ar & make_nvp("avRelStochEnergy",avRelStochEnergy);
  if (version >= 1){
    ar & make_nvp("totalRelStochEnergy",totalRelStochEnergy);
    ar & make_nvp("highestRelStochEnergy",highestRelStochEnergy);
  }
  ar & make_nvp("chi2",chi2);
  ar & make_nvp("chi2_red",chi2_red);
  ar & make_nvp("avStochDepth",avStochDepth);
  ar & make_nvp("status",status);
}

//this macro instantiates all the needed serialize methods
I3_SERIALIZABLE(I3EnergyLoss);

std::ostream& operator<<(std::ostream& os, const I3EnergyLoss& p) {
  os << "[ I3EnergyLoss::\n"
     << std::setw(24) << "Eloss_1000: " << p.Eloss_1000 << "\n"
     << std::setw(24) << "Eloss_1500: " << p.Eloss_1500 << "\n"
     << std::setw(24) << "Eloss_1600: " << p.Eloss_1600 << "\n"
     << std::setw(24) << "Eloss_1700: " << p.Eloss_1700 << "\n"
     << std::setw(24) << "Eloss_1800: " << p.Eloss_1800 << "\n"
     << std::setw(24) << "Eloss_1900: " << p.Eloss_1900 << "\n"
     << std::setw(24) << "Eloss_2000: " << p.Eloss_2000 << "\n"
     << std::setw(24) << "Eloss_2100: " << p.Eloss_2100 << "\n"
     << std::setw(24) << "Eloss_2200: " << p.Eloss_2200 << "\n"
     << std::setw(24) << "Eloss_2300: " << p.Eloss_2300 << "\n"
     << std::setw(24) << "Eloss_2400: " << p.Eloss_2400 << "\n"
     << std::setw(24) << "Eloss_3000: " << p.Eloss_3000 << "\n"
     << std::setw(24) << "primMassEstimate: " << p.primMassEstimate << "\n"
     << std::setw(24) << "primEnergyEstimate: " << p.primEnergyEstimate << "\n"
     << std::setw(24) << "primMassEstimate_err: " << p.primMassEstimate_err << "\n"
     << std::setw(24) << "primEnergyEstimate_err: " << p.primEnergyEstimate_err << "\n"
     << std::setw(24) << "nHEstoch: " << p.nHEstoch << "\n"
     << std::setw(24) << "avStochEnergy: " << p.avStochEnergy << "\n"
     << std::setw(24) << "totalStochEnergy: " << p.totalStochEnergy << "\n"
     << std::setw(24) << "highestStochEnergy: " << p.highestStochEnergy << "\n"
     << std::setw(24) << "avRelStochEnergy: " << p.avRelStochEnergy << "\n"
     << std::setw(24) << "totalRelStochEnergy: " << p.totalRelStochEnergy << "\n"
     << std::setw(24) << "highestRelStochEnergy: " << p.highestRelStochEnergy << "\n"
     << std::setw(24) << "chi2: " << p.chi2 << "\n"
     << std::setw(24) << "chi2_red: " << p.chi2_red << "\n"
     << std::setw(24) << "avStochDepth: " << p.avStochDepth << "\n"
     << std::setw(24) << "status: " << p.status << "\n"
     << "]";
  return os;
}
