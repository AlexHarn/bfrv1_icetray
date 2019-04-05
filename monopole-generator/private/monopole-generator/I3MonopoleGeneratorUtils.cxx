/**
 * class: I3MonopoleGeneratorUtils.cxx
 * (c) 2004 - 2014 IceCube Collaboration
 * Version $Id$
 *
 * @date $Date$
 * @author Brian Christy <bchristy@icecube.umd.edu>
 * @author Alex Olivas <olivas@icecube.umd.edu>
 * @brief Utility Namespace for the I3MonopoleGenerator Module
 */

#include "I3MonopoleGeneratorUtils.h"
#include <cmath>

I3Direction
I3MonopoleGeneratorUtils::RandomizeDirection(I3RandomServicePtr random,
                                             double zenithMin, double zenithMax,
                                             double azimuthMin, double azimuthMax) {
    //double zen = acos(random->Uniform(-1,1));
    //double azi = random->Uniform(0,2*I3Constants::pi);
    double zen, azi;
    if (zenithMin != zenithMax) {
        double cosZenith = random->Uniform(cos(zenithMax / I3Units::radian), cos(zenithMin / I3Units::radian));
        zen = acos(cosZenith);
    }
    else {
        zen = zenithMin;
    }
    if (azimuthMin != azimuthMax) {
        azi = random->Uniform(azimuthMin / I3Units::radian, azimuthMax / I3Units::radian);
    }
    else {
        azi = azimuthMin;
    }
    I3Direction dir(zen, azi);
    return dir;
}

I3Position
I3MonopoleGeneratorUtils::RandomizePosition(I3RandomServicePtr random,
                                            I3Direction dir, double diskDist,
                                            double diskRad) {
    double z = diskDist / I3Units::m;
    double r(NAN);
    double theta(NAN);

    r = sqrt(random->Uniform(0, pow(diskRad / I3Units::m, 2)));
    theta = random->Uniform(0, 2 * I3Constants::pi);
    log_debug("Chosen radius is %f (m)", r);
    log_debug("Chosen theta is %f (rad)", theta);

    if (std::isnan(r) || std::isnan(theta)) {
        log_fatal("Radius (%f) or theta (%f) not set in Randomize Position", r, theta);
    }

    double x = r * cos(theta);
    double y = r * sin(theta);
    log_debug("(x,y) position of particle before disk rotation is (%f,%f) in m", x, y);

    /**
     * Rotates Center of disk to:
     * (diskDist,zenith,0) then
     * (diskDist,zenith,azimuth)
     * in spherical coordinates
     */
    I3Position p(x * I3Units::m, y * I3Units::m, z * I3Units::m);
    p.RotateY(dir.GetZenith());
    p.RotateZ(dir.GetAzimuth());
    log_debug("Final (x,y,z) positon is (%f,%f,%f) in m", p.GetX(), p.GetY(), p.GetZ());

    return p;
}

double
I3MonopoleGeneratorUtils::RandomizeVelocity(I3RandomServicePtr random, double betaMin, double betaMax,
                                            double powerLawIndex, double &weight) {
    double beta;
    if (std::isnan(powerLawIndex)) {
        beta = random->Uniform(betaMin, betaMax);
    }
    else {
        double k = 1.;
        double border, uniform;
        if (powerLawIndex == 1) {
            border = k * log(betaMax / betaMin);
            uniform = random->Uniform(0, border);
            beta = betaMin * exp(uniform / k);
            weight = 1. / (k / beta);
        }
        else {
            border = k / (1 - powerLawIndex) * (pow(betaMax, 1 - powerLawIndex) - pow(betaMin, 1 - powerLawIndex));
            uniform = random->Uniform(0, border);
            beta = pow((1 - powerLawIndex) / k * uniform + pow(betaMin, 1 - powerLawIndex), 1 / (1 - powerLawIndex));
            weight = 1. / (k * pow(beta, -powerLawIndex));
        }
        // after simulation, reconstruction etc.
        // you have to reweight with this factor,
        // divide it by the sum of all weights
        // and multiply it with the number of generated events
    }
    return beta;
}

double
I3MonopoleGeneratorUtils::CalcSolidAngle(double zmin, double zmax,
                                         double amin, double amax) {
    return (amax / I3Units::radian - amin / I3Units::radian) * \
    (cos(zmin / I3Units::radian) - cos(zmax / I3Units::radian));
}

double I3MonopoleGeneratorUtils::beta2gamma(double beta){ //helper function to translate gamma <-> beta
    return 1.0 / (sqrt(1.0 - pow(beta, 2)));
}

double I3MonopoleGeneratorUtils::gamma2beta(double gamma){
    return sqrt(1.0 - pow(gamma, -2));
}

void
I3MonopoleGeneratorUtils::CheckMass(const double &mass) {
    if (mass < 1e5 * I3Units::GeV || mass > 1e17 * I3Units::GeV) {
        log_fatal("Mass (%f GeV) out of range.", mass / I3Units::GeV);
    }
}

void
I3MonopoleGeneratorUtils::CheckAndSetBetaGamma(double &gamma, const std::vector<double> &betaRange,
                                               const double &powerLawIndex, double &beta, bool &useBetaRange,
                                               double &betaMin, double &betaMax) {
    //3 valid cases
    //
    //1. gamma set
    // -> beta mustn't be set, powerlaw mustn't be set
    //2. beta set
    //-> gamma mustn't be set, range or single beta?
    //2.1 single beta
    //-> powerlaw mustn't be set, calculate gamma, set useBetaRange False
    //2.2 beta range
    //-> powerlaw must be set, set useBetaRange True
    
    
    if (betaRange.size() != 1 && betaRange.size() != 2) {
        log_fatal("Beta needs to be one or two elements!");
    }

    if ((!std::isnan(gamma) && !std::isnan(betaRange[0])) ||
        (std::isnan(gamma) && std::isnan(betaRange[0]))) {
        log_fatal("Please set either beta XOR gamma to express the velocity of the monopole.");
    }


    //usecases:
    if (!std::isnan(gamma)) {
        log_trace("Gamma usecase");
        if (gamma <= 1. || gamma > 1000.) {
            log_fatal("Gamma (%f) out of range.", gamma);
        }
        useBetaRange = false;
        beta = gamma2beta(gamma);

    } else if (betaRange.size() == 1 || std::isnan(betaRange[1]) ||
               betaRange[0] == betaRange[1]) { //betaRange_[0] cannot be NAN due to previous check
        log_trace("Single beta usecase");
        beta = betaRange[0];
        if (beta < 1e-6 || beta >= 1.) {
            log_fatal("Beta (%f) out of range.", beta);
        }

        useBetaRange = false;
        gamma = beta2gamma(beta);

    } else {
        betaMin = betaRange[0];
        betaMax = betaRange[1];
        useBetaRange = true;

        if (betaMin < 1e-6 || betaMin >= 1.) {
            log_fatal("BetaMin (%f) out of range.", beta);
        }
        if (betaMax < 1e-6 || betaMax >= 1.) {
            log_fatal("BetaMax (%f) out of range.", beta);
        }
        if (!std::isnan(powerLawIndex) && powerLawIndex < 0.) {
            log_fatal("This generator can only produce negative power laws but you chose index %f.", -powerLawIndex);
        }
    }

    if (!useBetaRange && !std::isnan(powerLawIndex)) {
        log_fatal("No range for beta is defined, cannot apply a power law.");
    }
}

void
I3MonopoleGeneratorUtils::CheckDisk(const double &diskDist, const double &diskRad) {
    // stuff regarding the trajectory of the monopole
    if (diskDist < 0 || diskDist > 5. * I3Units::km) {
        log_fatal("Disk_dist (%f m) out of range.", diskDist / I3Units::m);
    }
    if (diskRad < 0) {
        log_fatal("Disk_Rad (%f m) out of range.", diskRad / I3Units::m);
    }
}

void
I3MonopoleGeneratorUtils::CheckAndSetLength(double &length, const double &defaultValue) {
    if (length ==
        -1.) {   // The default length is 2* diskDist_ but it can be configured to any valid value, including NaN
        length = defaultValue;
    }

    if (length < 0) {
        log_fatal("Can't have a negative length.");
    }
}

void
I3MonopoleGeneratorUtils::CheckAndSetAngle(const std::string &name, const std::vector<double> &range, double &min,
                                           double &max, const double &validMin, const double &validMax) {
    if (range.size() != 2) {
        log_fatal("Please configure %sRange with a list containing *only* the minimum and the maximum %s.", name.c_str(), name.c_str());
    }

    min = range[0];
    max = range[1];

    if (min / I3Units::degree < validMin || min / I3Units::degree > validMax) {
        log_fatal("%sMin (%f rad) out of range.", name.c_str(), min / I3Units::radian);
    }
    if (max / I3Units::degree < validMin || max / I3Units::degree > validMax) {
        log_fatal("%sMax (%f rad) out of range.", name.c_str(), max / I3Units::radian);
    }
    if (min > max) {
        log_error("%s: (%f rad, %f rad)", name.c_str(), min / I3Units::radian, max / I3Units::radian);
        log_fatal("Can't have lower boundary > upper boundary.");
    }
    if (min / I3Units::degree > validMin || max / I3Units::degree < validMax ) {
        log_warn("The direction of the monopole is restricted!\n%s range: [%.2f, %.2f] degree.", name.c_str(),
                 min / I3Units::degree, max / I3Units::degree);
    }
}

void
I3MonopoleGeneratorUtils::CheckAndSetZenith(const std::vector<double> &zenithRange, double &zenithMin,
                                            double &zenithMax) {
    CheckAndSetAngle("zenith", zenithRange, zenithMin, zenithMax, 0., 180.);
}

void
I3MonopoleGeneratorUtils::CheckAndSetAzimuth(const std::vector<double> &azimuthRange, double &azimuthMin,
                                             double &azimuthMax) {
    CheckAndSetAngle("azimuth", azimuthRange, azimuthMin, azimuthMax, 0., 360.);
}

void
I3MonopoleGeneratorUtils::CheckShiftCenter(const std::vector<double> &shiftCenter) {
    if (shiftCenter.size() != 3) {
        log_fatal("If you want to shift the center, please provide an array containing x,y and z coordinates.");
    }
    if (shiftCenter[0] != 0. || shiftCenter[1] != 0. || shiftCenter[2] != 0.) {
        log_warn("Shifting positions from center of IceCube by (%.2f,%.2f,%.2f).", shiftCenter[0], shiftCenter[1],
                 shiftCenter[2]);
    }
}

void
I3MonopoleGeneratorUtils::CheckDiskPos(const bool &randPos, const double &radOnDisk, const double &aziOnDisk, const double &diskRad) {
    if (randPos) {
        if (!std::isnan(radOnDisk) || !std::isnan(aziOnDisk)) {
            log_fatal("You cannot set the radius and azimuth on the generation disk if the starting position is randomized.");
        }
    } else {
        if (std::isnan(radOnDisk) || std::isnan(aziOnDisk)) {
            log_fatal(
                    "You have to set the radius and azimuth on the generation disk if the starting position is not randomized.");
        }
        if (aziOnDisk / I3Units::degree < 0. || aziOnDisk / I3Units::degree > 360.) {
            log_fatal("Azimuth on generation disk (%f rad) out of range.", aziOnDisk / I3Units::radian);
        }
        if (radOnDisk < 0 || radOnDisk > diskRad) {
            log_fatal(
                    "Radius on generation disk (%f m) out of range. Make sure it's not larger than the disk radius.",
                    radOnDisk / I3Units::m);
        }
        log_warn("The start position on the generation disk is not randomized.");
    }
}




