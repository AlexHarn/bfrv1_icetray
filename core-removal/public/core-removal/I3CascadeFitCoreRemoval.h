/**
 *
 * Declaration of I3CascadeFitCoreRemoval
 *
 * (c) 2009
 * the IceCube Collaboration
 * $Id$
 *
 * @file I3CascadeFitCoreRemoval.h
 * @date $Date$
 * @author rutledge
 *
 */

#ifndef __I3_CASCADE_FIT_CORE_REMOVAL_H_
#define __I3_CASCADE_FIT_CORE_REMOVAL_H_

#include "icetray/I3Module.h"
#include "icetray/I3TrayHeaders.h"

#include <string>

#include "dataclasses/physics/I3RecoPulse.h"
#include "dataclasses/I3MapOMKeyMask.h"
#include "dataclasses/physics/I3Particle.h"
#include "dataclasses/geometry/I3Geometry.h"

class I3CascadeFitCoreRemovalTester;

/**
 * @brief A module that is designed to separate a single set of pusles into two sets of
 * pulses, based on a prior vertex seed, and a prior energy fit (although this can be done 
 * in-house using an energy to nch calibration).
 */
class I3CascadeFitCoreRemoval : public I3Module
{
  public:
    /**
     * IceTray constructor.
     *
     * @param context the context (set ot services) provided by Icetray.
     * 
     * @return an instance of this class.
     */
    I3CascadeFitCoreRemoval(const I3Context& context);

    /**
     * Destructor
     */
    ~I3CascadeFitCoreRemoval();

    /**
     * Configure the module.
     */
    void Configure();
  
    /**
     * Perform any cleanup at the end of a script.
     */
    void Finish();
 
    /**
     * Process a physics frame.
     */
    void Physics(I3FramePtr frame);

  private:
    /**
     * Calculate the SPE Radius based on the vertex energy. Actually calls the method 
     * with the same name, but with the different signature.
     *
     * @param vertex the prior vertex reconstruction (an input from the user).
     *
     * @return the SPE radius, as determined by the energy.
     */
    double CalculateSPERadius(I3ParticleConstPtr vertex);

    /**
     * Calculate the SPE Radius based on the vertex energy. 
     *
     * @param energy the reco'd energy of the event.
     *
     * @return the SPE radius, as determined by the energy.
     */
    double CalculateSPERadius(double energy);

    /**
     * Provides a way of estimating the energy from the number of hit channels.
     *
     * @param nCh the number of hit channels in the event.
     *
     * @return the energy estimated from the NCh.
     */
    double EstimateEnergyFromNCh(int nCh);

     /**
      *  Splits the input recopulses into 2 sets, determined by the input vertex and the energy 
      * of the event, and places each in the frame
      *
      * @param frame               pointer to an I3Frame object
      * @param inputPulseSeriesMap the input pulses.
      * @param hitRemovalRadius    the radius of separation, outside of which
      *                            hits will be considered "Corona" pulses,
      *                            and inside of which the pulses will be
      *                            considered to be "Core".
      * @param vertex              the seed vertex, as provided by the user.
      * @param geometry            the geometry map from the Geometry frame.
      *
      */
     void SplitPulsesByEnergyRadius(I3FramePtr frame, I3RecoPulseSeriesMapConstPtr inputPulseSeriesMap,
       const double hitRemovalRadius, I3ParticleConstPtr vertex, const I3Geometry& geometry);
    /**
     * The SPE Radius versus energy plot eventually diverges from linear, so this patches the
     * trend so that it doesn't go negative, but rather approaches some constant value.
     */
    void CalculateSpline();

    /**
     * This does exaclty what it says it does.
     */
    void PushEmptyResultIntoFrameToHandleBadCase(I3FramePtr frame);

  private:

    std::string vertexName_;
    std::string recoPulseInputName_;
    std::string recoPulseOutputName_;
    std::string corePulsesOutputName_;
    std::string radiusName_;

    bool nChCalib_;
   
    double speFraction_;
    double lambdaAttn_;
    double cLambda_;
  
    //use these to define the spline patch between zero and the critical energy
    double minRadius_;
    double splineConst_;
    double criticalEnergy_;

    friend class I3CascadeFitCoreRemovalTester;

    SET_LOGGER("I3CascadeFitCoreRemoval");
};

#endif
