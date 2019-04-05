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
 * @brief Implementation of a module to generate a magnetic monopole particle
 */

#include "monopole-generator/I3MonopoleGenerator.h"
#include "monopole-generator/I3MonopoleGeneratorUtils.h"

#include "icetray/I3Frame.h"
#include "dataclasses/I3Double.h"
#include "dataclasses/physics/I3Particle.h"
#include "dataclasses/physics/I3MCTree.h"
#include "dataclasses/physics/I3MCTreeUtils.h"

#include <cmath>

/**
 *Default Parameter Values
 */
const double DISK_DIST(1000. * I3Units::m);
const double DISK_RAD(800. * I3Units::m);

I3_MODULE(I3MonopoleGenerator);

I3MonopoleGenerator::I3MonopoleGenerator(const I3Context &ctx) :
        I3Module(ctx),
        treeName_("I3MCTree"),
        infoName_("MPInfoDict"),
        mass_(NAN),
        gamma_(NAN),
        betaMin_(NAN),
        betaMax_(NAN),
        diskDist_(DISK_DIST),
        diskRad_(DISK_RAD),
        randPos_(true),
        radOnDisk_(NAN),
        aziOnDisk_(NAN),
        startTime_(0.0),
        powerLawIndex_(NAN), //why not use 0 as a default so we get aflat distribution by default intstead of an error?
        length_(-1.) //NAN is a valid value, using -1 to signal default value (2* diskDist_)
        //using C++99-> vector list initialisations is not yet supported
        //once the experiment moved to C++11,
        //this should work and unify variable initialisation in this module
        /*
        betaRange_={NAN,NAN},
        zenithRange_={0.0*I3Units::degree, 180.0*I3Units::degree}, //min, max zenith
        azimuthRange_={0.0*I3Units::degree, 360.0*I3Units::degree}, //min, max azimuth
        shiftCenter_={0.0*I3Units::meter, 0.0*I3Units::meter, 0.0*I3Units::meter}
        */
{
    log_debug("Constructor I3MonopoleGenerator");

    betaRange_.resize(2);
    betaRange_[0] = NAN;
    betaRange_[1] = NAN;

    zenithRange_.resize(2);
    zenithRange_[0] = 0.0 * I3Units::degree; //min zenith
    zenithRange_[1] = 180.0 * I3Units::degree; //max zenith

    azimuthRange_.resize(2);
    azimuthRange_[0] = 0.0 * I3Units::degree; //min azimuth
    azimuthRange_[1] = 360.0 * I3Units::degree; //max azimuth

    shiftCenter_.resize(3);
    shiftCenter_[0] = 0.0 * I3Units::meter;
    shiftCenter_[1] = 0.0 * I3Units::meter;
    shiftCenter_[2] = 0.0 * I3Units::meter;

    AddParameter("TreeName", "Name of MCTree for generated Monopole.", treeName_);
    AddParameter("Mass", "Monopole mass.", mass_);
    AddParameter("Gamma", "Lorentz Boost Factor.", gamma_);
    AddParameter("BetaRange", "Velocity range relative to c.", betaRange_);
    AddParameter("Disk_dist", "Distance of disk from center.", diskDist_);
    AddParameter("Disk_rad", "Radius of Generation Disk.", diskRad_);
    AddParameter("Rand_pos", "Randomize the start position on generation disk.", randPos_);
    AddParameter("Rad_on_disk", "Radius of start position on the generation disk.", radOnDisk_);
    AddParameter("Azi_on_disk", "Azimuth of start position on the generation disk.", aziOnDisk_);
    AddParameter("ZenithRange",
                 "Restrict the zenith range. Configure with a list containing min zenith and max zenith.",
                 zenithRange_);
    AddParameter("AzimuthRange",
                 "Restrict the azimuth range. Configure with a list containing min azimuth and max azimuth.",
                 azimuthRange_);
    AddParameter("Length", "Length of the Monopole track.", length_);
    AddParameter("ShiftCenter", "Shift the monopole. Useful for other geometries or DeepCore only.", shiftCenter_);
    AddParameter("InfoName", "Name of the monopole info dictionary.", infoName_);
    AddParameter("PowerLawIndex", "Whether to use a powerLawIndex and which one to weight the velocity distribution.",
                 powerLawIndex_);
    AddParameter("StartTime", "Start time of the monopole.", startTime_);
    AddOutBox("OutBox");

}

I3MonopoleGenerator::~I3MonopoleGenerator() { }

void I3MonopoleGenerator::Configure() {
    log_debug("Configuring I3MonopoleGenerator");

    GetParameter("TreeName", treeName_);
    GetParameter("Mass", mass_);
    GetParameter("Gamma", gamma_);
    GetParameter("BetaRange", betaRange_);
    GetParameter("Disk_dist", diskDist_);
    GetParameter("Disk_rad", diskRad_);
    GetParameter("Rand_pos", randPos_);
    GetParameter("Rad_on_disk", radOnDisk_);
    GetParameter("Azi_on_disk", aziOnDisk_);
    GetParameter("ZenithRange", zenithRange_);
    GetParameter("AzimuthRange", azimuthRange_);
    GetParameter("shiftCenter", shiftCenter_);
    GetParameter("InfoName", infoName_);
    GetParameter("Length", length_);
    GetParameter("PowerLawIndex", powerLawIndex_);
    GetParameter("StartTime", startTime_);

    // initialize
    timescale_ = 0.0;

    /**
     * check that input parameters are sane and set some more variables
     */
    I3MonopoleGeneratorUtils::CheckMass(mass_);
    I3MonopoleGeneratorUtils::CheckAndSetBetaGamma(gamma_, betaRange_, powerLawIndex_, beta_, useBetaRange_, betaMin_, betaMax_);
    I3MonopoleGeneratorUtils::CheckDisk(diskDist_, diskRad_);
    I3MonopoleGeneratorUtils::CheckAndSetLength(length_, 2*diskDist_);
    I3MonopoleGeneratorUtils::CheckAndSetZenith(zenithRange_, zenithMin_, zenithMax_);
    I3MonopoleGeneratorUtils::CheckAndSetAzimuth(azimuthRange_, azimuthMin_, azimuthMax_);
    I3MonopoleGeneratorUtils::CheckShiftCenter(shiftCenter_);
    I3MonopoleGeneratorUtils::CheckDiskPos(randPos_, radOnDisk_, aziOnDisk_, diskRad_);
}


void I3MonopoleGenerator::DAQ(I3FramePtr frame) {
    I3RandomServicePtr random = GetService<I3RandomServicePtr>();
    if (!random) {
        log_fatal("Failed to Get Random Service.");
    }

    I3Direction mpDir;
    mpDir = I3MonopoleGeneratorUtils::RandomizeDirection(random, zenithMin_, zenithMax_, azimuthMin_, azimuthMax_);

    if (std::isnan(mpDir.GetZenith()) || std::isnan(mpDir.GetAzimuth())) {
        log_fatal("Something went wrong with setting direction.");
    }

    I3Position mpPos;
    if (randPos_) {    // randomize the start position of the particle on the generation disk
        mpPos = I3MonopoleGeneratorUtils::RandomizePosition(random, mpDir, diskDist_, diskRad_);
    }
    else {            // use a fixed start position of the particle on the generation disk
        mpPos.SetX(radOnDisk_ * cos(aziOnDisk_));
        mpPos.SetY(radOnDisk_ * sin(aziOnDisk_));
        mpPos.SetZ(diskDist_);

        // to keep the distance diskDist_ from Center of IceCube when disk is rotated around the detector
        mpPos.RotateY(mpDir.GetZenith());
        mpPos.RotateZ(mpDir.GetAzimuth());
    }


    // To shift the monopoles to the center of DeepCore (IC86-I) configure with:
    // shiftCenter = [46.0*icetray.I3Units.meter, -34.5*icetray.I3Units.meter, -330.0*icetray.I3Units.meter]
    //log_warn("Initial Position: %f %f %f", mpPos.GetX(), mpPos.GetY(), mpPos.GetZ());
    mpPos.SetX(mpPos.GetX() + shiftCenter_[0]);
    mpPos.SetY(mpPos.GetY() + shiftCenter_[1]);
    mpPos.SetZ(mpPos.GetZ() + shiftCenter_[2]);
    //log_warn("Shifted Position: %f %f %f", mpPos.GetX(), mpPos.GetY(), mpPos.GetZ());


    // Randomize speed
    double weight = 1.;
    if (useBetaRange_) {
        beta_ = I3MonopoleGeneratorUtils::RandomizeVelocity(random, betaMin_, betaMax_, powerLawIndex_, weight);
        gamma_ = I3MonopoleGeneratorUtils::beta2gamma(beta_);
    }


    // create mc-tree and save it
    double energy(mass_*gamma_);
    double speed(I3Constants::c*beta_);

    I3Particle monopole;
    monopole.SetType(I3Particle::Monopole);
    monopole.SetLocationType(I3Particle::InIce);
    monopole.SetPos(mpPos);
    monopole.SetDir(mpDir);
    monopole.SetEnergy(energy);
    monopole.SetSpeed(speed);
    monopole.SetTime(startTime_);
    if (!std::isnan(length_))
        monopole.SetLength(length_);

    I3MCTreePtr mptree(new I3MCTree);
    I3MCTreeUtils::AddPrimary(*mptree, monopole);

    frame->Put(treeName_, mptree);

    // create monopole-dict and save it
    I3MapStringDoublePtr mpinfo(new I3MapStringDouble);
    (*mpinfo)["Mass"] = mass_;
    (*mpinfo)["Gamma"] = gamma_;
    (*mpinfo)["Beta"] = beta_;
    (*mpinfo)["DiskRadius"] = diskRad_;
    (*mpinfo)["DiskDistance"] = diskDist_;
    (*mpinfo)["LogEnergy"] = log10(energy);
    (*mpinfo)["ZenithMin"] = zenithMin_;
    (*mpinfo)["ZenithMax"] = zenithMax_;
    (*mpinfo)["AzimuthMin"] = azimuthMin_;
    (*mpinfo)["AzimuthMax"] = azimuthMax_;
    (*mpinfo)["SolidAngle"] = I3MonopoleGeneratorUtils::CalcSolidAngle(zenithMin_, zenithMax_, azimuthMin_,
                                                                       azimuthMax_);
    (*mpinfo)["Weight"] = weight;
    if (useBetaRange_) {
        (*mpinfo)["BetaMin"] = betaMin_;
        (*mpinfo)["BetaMax"] = betaMax_;
    }else{
        (*mpinfo)["BetaMin"] = beta_;
        (*mpinfo)["BetaMax"] = beta_;
    }
    (*mpinfo)["powerLawIndex"] = powerLawIndex_;

    // sum weights
    timescale_ += weight;
    //log_warn("\nTimescale %g \nWeight %g", weight, timescale_);


    frame->Put(infoName_, mpinfo);
    PushFrame(frame, "OutBox");
}


void I3MonopoleGenerator::Finish() {
    if (!std::isnan(powerLawIndex_)) {
        log_warn("\nTimescale %g", timescale_);
        log_warn(
                "Make sure to run the script resources/scripts/NormalizeWeighting.py after finishing the simulation, to get the correct weights!");
    }
}
