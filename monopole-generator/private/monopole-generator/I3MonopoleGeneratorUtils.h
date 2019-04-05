#ifndef MONOPOLE_GENERATOR_I3MONOPOLEGENERATORUTILS_H_INCLUDED
#define MONOPOLE_GENERATOR_I3MONOPOLEGENERATORUTILS_H_INCLUDED

/**
 * A Monopole Generator Module
 * (c) 2004 - 2014 IceCube Collaboration
 * Version $Id$
 *
 * @file I3MonopoleGeneratorUtils.h
 * @date $Date$
 * @author bchristy
 * @author olivas
 * @brief Utility namespace for the I3MonopoleGenerator module
 */

#include "dataclasses/I3Constants.h"
#include "icetray/I3Units.h"
#include "dataclasses/I3Position.h"
#include "dataclasses/I3Direction.h"
#include "phys-services/I3RandomService.h"

/**
 *Namespace used to generate the position of the particle
 */
namespace I3MonopoleGeneratorUtils {
    /**
     *@brief randomizes monopole direction if desired
     *@param random is the random service generator
     *@param zenithMin is the lower zenith bound
     *@param zenithMax is the upper zenith bound
     *@param azimuthMin is the lower azimuth bound
     *@param azimuthMax is the upper azimuth bound
     *@return direction (as I3Direction) of monopole
     */
    I3Direction RandomizeDirection(I3RandomServicePtr, double, double, double, double);

    /**
     *@brief generates the disk and position on it
     *@param random is the random service generator
     *@param dir is the direction from which the particle is coming from
     *@param diskDist is how far away from the detector center the
     *generation disk is placed
     *@param diskRad is the radius of the generation disk
     *@return position (as I3Position) of the generated monopole
     */
    I3Position RandomizePosition(I3RandomServicePtr, I3Direction, double, double);

    /**
    *@brief generate a random velocity for a monopole
    *@param random is the random service generator
    *@param betaMin_ lower bound of the velocity distribution
    *@param betaMax_ upper bound of the velocity distribution
    *@param powerLawIndex_ power law index for the velocity distribution
    *@param weight * pointer to the weight
    *@return beta
     */
    double RandomizeVelocity(I3RandomServicePtr, double, double, double, double &);

    /**
    *@brief Calculate the solid angle for the given configuration
    *@param zmin minimum zenith angle
    *@param zmax maximum zenith angle
    *@param amin minimum azimuth angle
    *@param amax maximum azimuth angle
    *@return solid angle
     */
    double CalcSolidAngle(double, double, double, double);

    /**
    *@brief Calculate corresponding gamma for given beta
    *@param beta beta
    *@return gamma
     */
    double beta2gamma(double);

    /**
    *@brief Calculate corresponding beta for given gamma
    *@param gamma gamma
    *@return beta
     */
    double gamma2beta(double);

    /**
    *@brief Sanity checks for the mass parameter (1e5 GeV < mass < 1e17 GeV)
    *@param mass
    */
    void CheckMass(const double &);

    /**
    *@brief Check which usecase (gamma only, beta range, only one beta), Set other variables accordingly and apply sanity checks (0 < beta < 1, 1 < gamma < 1000, etc.)
    *@param gamma if NAN, will be set depending on beta_range
    *@param betaRange vector containing the minimum and maximum beta value, if maximum is NAN or equal to minimum, only the minimum is used as a beta value
    *@param powerLawIndex power law factor used if a beta range is defined. If NAN, an uniform distribution will be applied. Mustn't be set or be set to -1 if single gamma or beta usecase
    *@param beta always set by the function depending on gamma and betaRange
    *@param useBetaRange always set by the function depending on betaRange
    *@param betaMin set by the function depending on betaRange
    *@param betaMax set by the function depending on betaRange
    */
    void CheckAndSetBetaGamma(double &, const std::vector<double> &, const double &, double &, bool &, double &,
                              double &);

    /**
    *@brief Sanity checks for the Disk Distance parameters
    *@param diskDist
    *@param diskRad
    */
    void CheckDisk(const double &, const double &);

    /**
    *@brief Sanity checks for the length parameter and apply default value if length=-1
    *@param length
    *@param defaultValue value of length if length was set to -1
    */
    void CheckAndSetLength(double &, const double &);

    /**
    *@brief Only internally used to abstract CheckAndSetZenith and CheckAndSetAzimuth to a common function
    *@param name name used in logs, azimuth or zenith
    *@param range vector of size 2 with minimum and maximum angle
    *@param min writen by function, will be min angle
    *@param max writen by function, will be max angle
    *@param validMin minimum of range for sanity check
    *@param validMax maximum of range for sanity check
    */
    void CheckAndSetAngle(const std::string &, const std::vector<double> &, double &, double &, const double &,
                          const double &);

    /**
    *@brief Applies sanity checks to the zenith and extracts the values from the vector the min/max variables
    *@param zenithRange vector of size 2 with minimum and maximum angle
    *@param zenithMin writen by function, will be min angle
    *@param zenithMax writen by function, will be max angle
    */
    void CheckAndSetZenith(const std::vector<double> &, double &, double &);

    /**
    *@brief Applies sanity checks to the azimuth and extracts the values from the vector the min/max variables
    *@param azimuthRange vector of size 2 with minimum and maximum angle
    *@param azimuthMin writen by function, will be min angle
    *@param azimuthMax writen by function, will be max angle
    */
    void CheckAndSetAzimuth(const std::vector<double> &, double &, double &);

    /**
    *@brief Sanity checks for the shiftCenter parameter
    *@param shiftCenter
    */
    void CheckShiftCenter(const std::vector<double> &);

    /**
    *@brief Applies sanity checks to position on generation disk parameters depending on a set position or random position
    *@param randPos enables generation of random position on generation disk
    *@param radOnDisk radius in disk
    *@param aziOnDik azimuth on disk
    *@param diskRad radius of disk
    */
    void CheckDiskPos(const bool &, const double &, const double &, const double &);
}
#endif //MONOPOLE_GENERATOR_I3MONOPOLEGENERATORUTILS_H_INCLUDED
