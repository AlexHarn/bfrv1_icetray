#ifndef I3LAPUTOPFITTER_H_INCLUDED
#define I3LAPUTOPFITTER_H_INCLUDED

/**
 * copyright  (C) 2005
 * the icecube collaboration
 * $Id$
 *
 * @file I3LaputopFitter.h
 * @version $Revision$
 * @date $Date$
 * @author kath
 */

// gulliver stuff
#include "gulliver/I3Gulliver.h"
#include "gulliver/I3SeedServiceBase.h"

// needed for getting it from context
#include "toprec/I3LaputopLikelihood.h"

// icetray stuff
#include "icetray/I3ConditionalModule.h"
#include "icetray/IcetrayFwd.h"
#include "dataclasses/physics/I3Particle.h"
#include "dataclasses/I3Map.h"

/**
 * @class I3LaputopFitter
 * @brief Gulliver-based module to mimic "I3TopLateralFit" in the toprec project.
 *
 * I3TopLateralFit (the old fitter) performed four separate minimizatins:
 * 1) core only
 * 2) core + snow, with pulses < 11 meters removed
 * 3) core (+/- 3 sigma) + direction, but no snow
 * 4) core again (unbounded) + snow, but no direction
 * 
 * This fitter performs a similar sequence of "steps", any number between 1 and 4.
 * For details of differences between this fitter and I3TopLateralFit, see the .rst documentation.
 *
 * Code herein is modified from I3SimpleFitter.h
 *
 * @todo There is probably more to be said here.
 */
class I3LaputopFitter : public I3ConditionalModule {

public:

    /// constructor (define configurables)
    I3LaputopFitter(const I3Context& ctx);

    /// destructor
    ~I3LaputopFitter(){}

    /// configure (get & check configurables)
    void Configure();
    void Geometry(I3FramePtr frame);
    void Calibration(I3FramePtr frame);
    void DetectorStatus(I3FramePtr frame);

    /// do a reconstruction
    void Physics(I3FramePtr frame);

    /// say bye
    void Finish();


private:

    // inhibit default constructors and assignment by making them private
    I3LaputopFitter(); /// inhibited
    I3LaputopFitter(const I3LaputopFitter& source); /// inhibited
    I3LaputopFitter& operator=(const I3LaputopFitter& source); /// inhibited

    /// the core Gulliver object for basic tracks (a pointer, so we can postpone instantiation to Configure())
    boost::shared_ptr< I3Gulliver > fitterCore1_;
    boost::shared_ptr< I3Gulliver > fitterCore2_;
    boost::shared_ptr< I3Gulliver > fitterCore3_;
    boost::shared_ptr< I3Gulliver > fitterCore4_;

    /// seed service
    I3SeedServiceBasePtr seedService_;

    /// type to remember whether to store all or some of the fits
    enum StoragePolicyType {
        ONLY_BEST_FIT,
        ALL_FITS_AND_FITPARAMS_IN_VECTORS,
        ALL_FITS_AND_FITPARAMS_NOT_IN_VECTORS,
        ALL_RESULTS_IN_VECTORS,
        ALL_RESULTS_NOT_IN_VECTORS,
	INTERMEDIATE
    };

    // configurables (see parameter docs in icetray-inspect)
    std::string seedServiceName_; /// Seed preparation service.
    std::string llhName_;         /// Event loglikelihood calcuation service.
    std::vector<std::string> LDFFunctions_;
    std::vector<std::string> CurvFunctions_;
    I3LaputopLikelihoodPtr lapuLlhService_;

    // Configuring the "static permanent cuts"
    // As with the curvature, the user is going to have to enter three (or howevermany) of them
    std::vector<double> staticRCuts_;
    std::vector<double> staticTCuts_;
    

    std::string parName1_;         /// Track/shower/anything parametrization service.
    std::string parName2_;         /// Track/shower/anything parametrization service.
    std::string parName3_;         /// Track/shower/anything parametrization service.
    std::string parName4_;         /// Track/shower/anything parametrization service.
    std::string miniName_;        /// Minimizer service.
    std::string storagePolicyString_; /// option to store single or multiple results
    StoragePolicyType storagePolicy_; /// option to store single or multiple results
    std::string nonStdName_;      /// name to use when storing nonstandard part of hypothesis

    std::string fitName_; /// same as module name
    unsigned int eventNr_;
    unsigned int nSeeds_;
    unsigned int nSuccessFits_;
    unsigned int nSuccessEvents_;

    unsigned int nSteps_;   /// How many steps (minimum 1, maximum 4)

    SET_LOGGER( "Laputop" );
  
};  // end of the class definition.


#endif /* I3LAPUTOPFITTER_H_INCLUDED */
