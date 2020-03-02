/**
 * A Monopole Generator Module
 * (c) 2004-2014 IceCube Collaboration
 * Version $Id$
 *
 * @file I3MonopoleGenerator.cxx
 * @date $Date$
 * @author jacobi
 * @author bchristy
 * @author olivas
 * @author flauber
 * @brief Implementation of a module to generate a magnetic monopole particle
 */

#include "monopole-generator/I3MonopoleGenerator.h"

I3_MODULE(I3MonopoleGenerator);

I3MonopoleGenerator::I3MonopoleGenerator(const I3Context &ctx) :
        I3Module(ctx),
        treeName_("I3MCTree"),
        infoName_("MPInfoDict"),
        Nevents_(0),
        mass_(NAN),
        beta_(NAN),
        gamma_(NAN),
        speed_(NAN),
        energy_(NAN),
        useBetaRange_(NAN),
        diskDist_(1000. * I3Units::m),
        diskRad_(800. * I3Units::m),
        radOnDisk_(NAN),
        aziOnDisk_(NAN),
        startTime_(0.0),
        weight_(1.),
        powerLawIndex_(0),
        length_(-1.), //NAN is a valid value, using -1 to signal default value (2* diskDist_)
        totalweight_(0),
        mpinfo_dict_config_(new I3MapStringDouble),
        betaRange_{NAN, NAN},
        zenithRange_{0.0 * I3Units::degree, 180.0 * I3Units::degree}, //min, max zenith
        azimuthRange_{0.0 * I3Units::degree, 360.0 * I3Units::degree}, //min, max azimuth
        shiftCenter_{0.0 * I3Units::meter, 0.0 * I3Units::meter, 0.0 * I3Units::meter},
        precalculated_betas_{},
        need_precalculate_(false)

{
  log_debug("Constructor I3MonopoleGenerator");
  AddParameter("Nevents", "Number of events to be generated for the simulation.", Nevents_);
  AddParameter("TreeName", "Name of the `I3MCTree` to write the generated monopole particle to.", treeName_);
  AddParameter("InfoName", "Name of the monopole info dictionary. The generator writes generation parameters into this dictionary.", infoName_);
  AddParameter("Disk_dist", "Distance of the generation disk from the center of IceCube. (Default: `1000 * I3Units::m`)", diskDist_);
  AddParameter("Disk_rad", "Radius of Generation Disk. (Default: `800 * I3Units::m`)", diskRad_);
  AddParameter("BetaRange", "Velocity range of the monopole particle as array of lower and upper boundary. If a fixed velocify is desired, set lower and upper boundary to the same velocity. Express the velocities as ratio of the speed of light, c. For example: `[0.1, 0.4]`.", betaRange_);
  AddParameter("PowerLawIndex", "If a power-law index is given, the velocities of the simulated monopole particles will follow a power-law distribution with the given index. This can be helpful ti gain statistics for low velocities. If no power-law index is given, the velocities will be uniformly distributed (default).",
               powerLawIndex_);
  AddParameter("Mass", "Mass of the monopole particle. For slow monopoles, this parameter is optional. For fast monopoles, this parameter is needed to calculate energy losses along the trajectory. Example: `1e7 * I3Units::GeV`.", mass_);
  AddParameter("Length", "Length of the monopole track. Can be `NaN` or any length. The default value is calculated to twice the disk distance. Set to `-1` to indicate that the default value, `2 * Disk_dist`, should be used.", length_);
  AddParameter("ZenithRange",
               "List of lower and upper zenith bound. Use this parameter to restrict the direction of the monopole. Example: `[0.0 * I3Units::deg, 180.0 * I3Units::deg]`",
               zenithRange_);
  AddParameter("AzimuthRange",
               "List of lower and upper azimuth bound. Use this parameter to restrict the direction of the monopole. Example: `[0.0 * I3Units::deg, 360.0 * I3Units::deg]`",
               azimuthRange_);
  AddParameter("Rad_on_disk", "Set the radius coordinate of the starting position on the generation disk. Randomized if `NaN`. Example: `5. * I3Units::m`", radOnDisk_);
  AddParameter("Azi_on_disk", "Set the azimuth coordinate of the starting position on the generation disk. Randomized if `NaN`. Example: `45. * I3Units::deg`", aziOnDisk_);
  AddParameter("ShiftCenter", "Shifts the monopole. This is useful to explore different geometries. To shift according to the center of DeepCore (IC86-I SLOP trigger only acts on DC), configure with `ShiftCenter = ([46.0 * icetray.I3Units.m, -34.5 * icetray.I3Units.m, -330.0 * icetray.I3Units.m])`.", shiftCenter_);
  AddParameter("StartTime", "The time, measured from the beginning of the event, the monopole particle should be started. Example: `0. * I3Units::s`", startTime_);
  AddOutBox("OutBox");

}

I3MonopoleGenerator::~I3MonopoleGenerator() {}

void I3MonopoleGenerator::Configure() {
  GetParameter("NEvents", Nevents_);
  GetParameter("TreeName", treeName_);
  GetParameter("Mass", mass_);
  GetParameter("Disk_dist", diskDist_);
  GetParameter("Disk_rad", diskRad_);
  GetParameter("Rad_on_disk", radOnDisk_);
  GetParameter("Azi_on_disk", aziOnDisk_);
  GetParameter("shiftCenter", shiftCenter_);
  GetParameter("InfoName", infoName_);
  GetParameter("Length", length_);
  GetParameter("BetaRange", betaRange_);
  GetParameter("ZenithRange", zenithRange_);
  GetParameter("AzimuthRange", azimuthRange_);
  GetParameter("PowerLawIndex", powerLawIndex_);
  GetParameter("StartTime", startTime_);
  if (Nevents_<= 0){
    log_fatal("Requested less than 1 event to be generated. Abort!");
  }

  /**
   * check that input parameters are sane and set some more variables
   */
  I3MonopoleGeneratorUtils::CheckMass(mass_, betaRange_);
  I3MonopoleGeneratorUtils::CheckAndSetBeta(betaRange_, powerLawIndex_);
  I3MonopoleGeneratorUtils::CheckDisk(diskDist_, diskRad_);
  I3MonopoleGeneratorUtils::CheckAndSetLength(length_, 2 * diskDist_);
  I3MonopoleGeneratorUtils::CheckAndSetZenith(zenithRange_);
  I3MonopoleGeneratorUtils::CheckAndSetAzimuth(azimuthRange_);
  I3MonopoleGeneratorUtils::CheckShiftCenter(shiftCenter_);
  I3MonopoleGeneratorUtils::CheckDiskPos(radOnDisk_, aziOnDisk_, diskRad_);

  if(betaRange_[0] == betaRange_[1]) {
    //cache result so we do not have to add it multiple times
    useBetaRange_ = false;
    beta_ = betaRange_[0];
    gamma_ = I3MonopoleGeneratorUtils::beta2gamma(beta_);
    speed_ = beta_ * I3Constants::c;
    energy_ = mass_ * gamma_;
    weight_ = 1.;
    totalweight_ = Nevents_;
    (*mpinfo_dict_config_)["Gamma"] = gamma_;
    (*mpinfo_dict_config_)["Beta"] = beta_;
    (*mpinfo_dict_config_)["LogEnergy"] = log10(energy_);
    (*mpinfo_dict_config_)["Weight"] = weight_;
    (*mpinfo_dict_config_)["OneWeight"] = weight_ / totalweight_;

  }else{
    useBetaRange_ = true;
    if (std::isnan(powerLawIndex_) || powerLawIndex_!=0 ){
      totalweight_ = Nevents_;
      (*mpinfo_dict_config_)["Weight"] = weight_;
      (*mpinfo_dict_config_)["OneWeight"] = weight_ / totalweight_;
    }else{
      need_precalculate_=true;
    }
  }

  // create monopole-dict and save it
  (*mpinfo_dict_config_)["Mass"] = mass_;
  (*mpinfo_dict_config_)["DiskRadius"] = diskRad_;
  (*mpinfo_dict_config_)["DiskDistance"] = diskDist_;
  (*mpinfo_dict_config_)["ZenithMin"] = zenithRange_[0];
  (*mpinfo_dict_config_)["ZenithMax"] = zenithRange_[1];
  (*mpinfo_dict_config_)["AzimuthMin"] = azimuthRange_[0];
  (*mpinfo_dict_config_)["AzimuthMax"] = azimuthRange_[1];
  //(*mpinfo_dict_config_)["SolidAngle"] = I3MonopoleGeneratorUtils::CalcSolidAngle(zenithRange_[0], zenithRange_[1], azimuthRange_[0], azimuthRange_[1]);
  (*mpinfo_dict_config_)["BetaMin"] = betaRange_[0];
  (*mpinfo_dict_config_)["BetaMax"] = betaRange_[1];
  (*mpinfo_dict_config_)["powerLawIndex"] = powerLawIndex_;
}



void I3MonopoleGenerator::DAQ(I3FramePtr frame) {
  I3RandomServicePtr random = GetService<I3RandomServicePtr>();
  //in case of powerlaw set, we need to calculate all betas in advance to properly calculate OneWeight
  if(need_precalculate_) {
    need_precalculate_ = false;
    for (unsigned int i = 0; i < Nevents_; ++i) {
      beta_ = I3MonopoleGeneratorUtils::RandomPowerLawSampled(random, betaRange_[0], betaRange_[1], powerLawIndex_);
      totalweight_ += 1. / (pow(beta_, -powerLawIndex_));
      precalculated_betas_.push_back(beta_);
    }
  }
  --Nevents_;
  I3MapStringDoublePtr mpinfo(new I3MapStringDouble(*mpinfo_dict_config_));
  if (!random) {
    log_fatal("Failed to Get Random Service.");
  }

  const double zenith = zenithRange_[0] == zenithRange_[1] ? zenithRange_[0] : I3MonopoleGeneratorUtils::RandomCosinusSampled(random, zenithRange_[0] / I3Units::radian, zenithRange_[1] / I3Units::radian);
  const double azimuth = azimuthRange_[0] == azimuthRange_[1] ? azimuthRange_[0] : I3MonopoleGeneratorUtils::RandomUniformSampled(random, azimuthRange_[0] / I3Units::radian, azimuthRange_[1] / I3Units::radian);
  I3Direction mpDir(zenith, azimuth);

  (*mpinfo)["Zenith"] = zenith;
  (*mpinfo)["Azimuth"] = azimuth;

  //r and theta on creation disk
  const double r = !std::isnan(radOnDisk_) ? radOnDisk_ : I3MonopoleGeneratorUtils::RandomCircularSampled(random, 0 / I3Units::m, diskRad_ / I3Units::m);
  const double theta = !std::isnan(aziOnDisk_) ? aziOnDisk_ : I3MonopoleGeneratorUtils::RandomUniformSampled(random, 0, 2 * I3Constants::pi);
  I3Position mpPos(r * cos(theta) * I3Units::m, r * sin(theta) * I3Units::m, diskDist_  * I3Units::m);

  (*mpinfo)["OnDiskRadius"] = r;
  (*mpinfo)["OnDiskAzimuth"] = theta;


  //rotate to get the proper arrival direction
  mpPos.RotateY(mpDir.GetZenith());
  mpPos.RotateZ(mpDir.GetAzimuth());

  //apply the shift
  mpPos.SetX(mpPos.GetX() + shiftCenter_[0]);
  mpPos.SetY(mpPos.GetY() + shiftCenter_[1]);
  mpPos.SetZ(mpPos.GetZ() + shiftCenter_[2]);

  // Randomize speed
  if (useBetaRange_) {
    if((std::isnan(powerLawIndex_) || powerLawIndex_!=0 )){
      beta_ = I3MonopoleGeneratorUtils::RandomUniformSampled(random, betaRange_[0], betaRange_[1]);
    }else{
      beta_ = precalculated_betas_[Nevents_];
      weight_ = 1. / (pow(beta_, -powerLawIndex_)); //probability for this beta to show up
      (*mpinfo)["Weight"] = weight_;
      (*mpinfo)["OneWeight"] = weight_ / totalweight_;
    }
    gamma_ = I3MonopoleGeneratorUtils::beta2gamma(beta_);
    speed_ = beta_ *I3Constants::c;
    energy_ = mass_ * gamma_;
    (*mpinfo)["Gamma"] = gamma_;
    (*mpinfo)["Beta"] = beta_;
    (*mpinfo)["LogEnergy"] = log10(energy_);
  }

  I3Particle monopole;
  monopole.SetType(I3Particle::Monopole);
  monopole.SetLocationType(I3Particle::InIce);
  monopole.SetPos(mpPos);
  monopole.SetDir(mpDir);
  monopole.SetEnergy(energy_);
  monopole.SetSpeed(speed_);
  monopole.SetTime(startTime_);
  monopole.SetLength(length_);

  I3MCTreePtr mptree(new I3MCTree);
  I3MCTreeUtils::AddPrimary(*mptree, monopole);

  //add tree and info dict to frame
  frame->Put(treeName_, mptree);
  frame->Put(infoName_, mpinfo);
  PushFrame(frame);
  if(Nevents_ <= 0){
    RequestSuspension();
  }
}

void I3MonopoleGenerator::Finish() {
}
