#ifndef MONOPOLE_PROPAGATOR_MONOPOLEPROPAGATORUTILS_H
#define MONOPOLE_PROPAGATOR_MONOPOLEPROPAGATORUTILS_H
/**
 * class: I3MonopolePropagatorUtils.h
 * (c) 2004 IceCube Collaboration
 * Version $Id: I3MonopoleRelativisticUtils.h 124988 2014-10-23 15:17:00Z jacobi $
 *
 * Date 20 Oct 2006
 * @version $Revision: 124988 $
 * @date $Date: 2014-10-23 10:17:00 -0500 (Do, 23. Okt 2014) $
 * @author Brian Christy <bchristy@icecube.umd.edu>
 * @author Alex Olivas <olivas@icecube.umd.edu>
 * @brief Utility Namespace for the I3MonopolePropagator Module
 */
/** Propagate Monopole assuming only ionization energy loss
 *  Used formula Eq 16 of Ahlen Phys Rev D Vol 17 # 1
 *  "Stopping Power formula for magnetic monopoles"
 *  and used values of K and B for g = 137e/2
 *
 *  Coeff was found using [4Pi Ne(137/2)^2(e)^4]/(m_e c^2)
 *  where Ne = Na rho Z/A
 *
 *  For Density Correction constants
 *  Used values from Table C.2 in Dima's 
 *  Thesis on mmc, which in turn came from
 *  http://pdg.lbl.gov/AtomicNuclearProperties/substances/276.html
*/

#include <dataclasses/I3Map.h>
#include <dataclasses/I3Constants.h>
#include <dataclasses/physics/I3Particle.h>
#include <dataclasses/physics/I3MCTree.h>
#include "icetray/I3Frame.h"


namespace I3MonopoleRelativisticUtils {

    /**
     * Propagate a fast monopole
     */

    void PropagateFastMonopole(I3FramePtr,
                               std::string,
                               I3MCTreeConstPtr,
                               I3MCTreePtr,

                               double,
                               double,
                               double,
                               bool,
                               bool,
                               double,
                               double,
                               bool,
                               bool
    );

    /**
     *Generates monopole at end of given segment
     */
    I3ParticlePtr
            AddMonopole(I3Particle, double, bool, bool, bool, double, double, double);

    /**
     * Responsible for taking a given segment of the Monopole and calculating
     * the energy loss due to Ionization (with Density Correction on/off option)
     */
    double CalculateEnergyLoss(double, double, bool);

    /**
     * Calculates the density correction part of the Ionization energy loss
     * for a given speed
     */
    double CalculateDensityCorrection(double);

    /**
     *Determines length to use for newly generated monopole
     *by estimating how far it could travel before losing 0.1%
     *of its kinetic energy
     */
    double CalculateNextLength(double, double, double, double, double);

    /**
     * Helper function - got tired of writing function over and over
     */
    double EnergyToBeta(double, double);

    /**
     * Responsible for performing extensive sanity checks on particle
     * result to ensure nothing went horribly wrong
     */
    void CheckParticle(I3Particle &, double);

    /**
     * Update the dictionary created by the monopole generator
     */
    void UpdateMPInfoDict(I3MCTreeConstPtr, I3MapStringDoublePtr);


    /**
     * Extract the speed profile of the monopole from the MCTree
     */
    void MPSpeedProfile(I3MCTreeConstPtr, I3VectorDoublePtr);

}
#endif //MONOPOLE_PROPAGATOR_MONOPOLEPROPAGATORUTILS_H
