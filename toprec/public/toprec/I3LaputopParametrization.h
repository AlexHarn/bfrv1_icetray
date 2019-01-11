#ifndef I3LAPUTOPPARAMETRIZATION_H_INCLUDED
#define I3LAPUTOPPARAMETRIZATION_H_INCLUDED

/**
 *
 * @file I3LaputopParametrization.h
 * @brief declaration of the I3LaputopParametrization class
 *
 * (c) 2007 the IceCube Collaboration
 * $Id$
 *
 * @version $Revision$
 * @date $Date$
 * @author kath
 *
 */

//////////////////////
// This code was stolen and adapted from double-muon!
//   double-muon/public/double-muon/I3DoubleMuonParametrization.h
//////////////////////


#include <string>
#include "icetray/I3Units.h"
#include "icetray/I3ServiceBase.h"
#include "gulliver/I3ParametrizationBase.h"


/**
 * @class I3LaputopParametrization
 * @brief define fittable variables and their parametrization for toprec lateral fit
 *
 * For now: simple standard xyzza parametrization, plus additional parameters (S125, beta, etc.)
 */
class I3LaputopParametrization : public I3ServiceBase,
                                    public I3ParametrizationBase {
public:

    /// default constructor for unit tests
    I3LaputopParametrization( std::string name="unittest" );

    /// constructor I3Tray
    I3LaputopParametrization(const I3Context &c);

    /// set parameters (in I3Tray)
    void Configure();

    /// compute event hypothesis from minimizer parameters
    void UpdatePhysicsVariables();

    /// compute minimizer parameters from event hypothesis
    void UpdateParameters();

    /// passes the covariance matrix after the minimizer finished
    void PassCovariance(const boost::numeric::ublas::symmetric_matrix<double>&);

    /// tell your name
    const std::string GetName() const {
        return this->I3ServiceBase::GetName();
    }

    // Defaults
    static const bool DEFAULT_FIX_CORE;
    static const bool DEFAULT_FIX_SIZE;
    static const bool DEFAULT_FIX_TRACKDIR;
    static const bool DEFAULT_FIT_SNOWCORRECTIONFACTOR;
    static const bool DEFAULT_ISITBETA;
    static const double DEFAULT_LIMITCOREBOXSIZE;
    static const double DEFAULT_MINBETA;
    static const double DEFAULT_MAXBETA;
    static const double DEFAULT_COREXY;
    static const double DEFAULT_MAXLOGS125;

    static const double DEFAULT_CORE_STEPSIZE;
    static const double DEFAULT_NXY_STEPSIZE;
    static const double DEFAULT_LOGS125_STEPSIZE;
    static const double DEFAULT_BETAAGE_STEPSIZE;
    static const double DEFAULT_T0_STEPSIZE;
    static const double DEFAULT_SNOWD0_STEPSIZE;

    // Parameter tags & descriptions
    static const std::string FIX_CORE_TAG;
    static const std::string FIX_SIZE_TAG;
    static const std::string FIX_TRACKDIR_TAG;
    static const std::string FIT_SNOWCORRECTIONFACTOR_TAG;
    static const std::string BETA_TAG;
    static const std::string MINBETA_TAG;
    static const std::string MAXBETA_TAG;
    static const std::string LIMITCORE_TAG;
    static const std::string COREXY_TAG;
    static const std::string MAXLOGS125_TAG;

    static const std::string FIX_CORE_DESCRIPTION;
    static const std::string FIX_SIZE_DESCRIPTION;
    static const std::string FIX_TRACKDIR_DESCRIPTION;
    static const std::string FIT_SNOWCORRECTIONFACTOR_DESCRIPTION;
    static const std::string BETA_DESCRIPTION;
    static const std::string MINBETA_DESCRIPTION;
    static const std::string MAXBETA_DESCRIPTION;
    static const std::string LIMITCORE_DESCRIPTION;
    static const std::string COREXY_DESCRIPTION;
    static const std::string MAXLOGS125_DESCRIPTION;

    // More tags
    static const std::string vstep_optionname;
    //static const std::string astep_optionname;
    static const std::string sstep_optionname;
    static const std::string betastep_optionname;
    //static const std::string snowstep_optionname;

    // Curvature A, D, N stuff (as is done in I3CurvatureParametrization)
    // Defaults
    static const double DEFAULT_A_MIN;
    static const double DEFAULT_A_MAX;
    static const double DEFAULT_A_STEPSIZE;
    static const double DEFAULT_D_MIN;
    static const double DEFAULT_D_MAX;
    static const double DEFAULT_D_STEPSIZE;
    static const double DEFAULT_N_MIN;
    static const double DEFAULT_N_MAX;
    static const double DEFAULT_N_STEPSIZE;
    // Tags
    static const std::string MIN_A_TAG;
    static const std::string MIN_D_TAG;
    static const std::string MIN_N_TAG;
    static const std::string MAX_A_TAG;
    static const std::string MAX_D_TAG;
    static const std::string MAX_N_TAG;
    static const std::string STEPSIZE_A_TAG;    
    static const std::string STEPSIZE_D_TAG;
    static const std::string STEPSIZE_N_TAG;
    static const std::string FREE_A_TAG;
    static const std::string FREE_D_TAG;
    static const std::string FREE_N_TAG;
    // Descriptions
    static const std::string MIN_A_DESC;
    static const std::string MIN_D_DESC;
    static const std::string MIN_N_DESC;
    static const std::string MAX_A_DESC;
    static const std::string MAX_D_DESC;
    static const std::string MAX_N_DESC;
    static const std::string STEPSIZE_A_DESC;
    static const std::string STEPSIZE_D_DESC;
    static const std::string STEPSIZE_N_DESC;
    static const std::string FREE_A_DESC;
    static const std::string FREE_D_DESC;
    static const std::string FREE_N_DESC;



private:

    // User parameter booleans
    bool fFixCore_;
    bool fFixSize_;
    bool fFixDir_;
    bool fFitSnowCorrectionFactor_;
    bool fIsItBeta_;

    // Limit the core position to a "box" around the previous core?
    // If so, set this to something > 0
    // Then it will represent the dimensions of the box (plus/minus this number in both x and y)
    // Otherwise, fCoreXYLimit will be used instead.
    double fLimitCoreBoxSize_;  

    // Stepsizes
    double fCoreStep_;
    double fNxyStep_;
    double fLogS125Step_;
    double fBetaAgeStep_;
    double fT0Step_;
    double fSnowD0Step_;
    double fBetaMin_;
    double fBetaMax_;
    double fCoreXYLimit_;
    double fLogS125Max_;

    /* add later
    /// stepsize for zenith (for azimuth it's twice this value)
    double angleStepsize_;
    */

    // Only needed if LimitCoreRadius is true, and we need to
    // remember where the original seed was.
    // Also, if the core moves around, we're going to want to remember
    // what the original T0 was.
    double seedX_;
    double seedY_;
    double seedT_;
    
    // Curvature A, D, N stuff:
    // Booleans
    bool fFreeA_;
    bool fFreeD_;
    bool fFreeN_;
    //int iparA_;
    //int iparD_;
    //int iparN_;
    // Max, Min, and Stepsizes
    double fAStep_;
    double fDStep_;
    double fNStep_;
    double fAMin_;
    double fDMin_;
    double fNMin_;
    double fAMax_;
    double fDMax_;
    double fNMax_;

    /// log4cplus thingy
    SET_LOGGER( "Laputop" );
};

#endif /* I3LAPUTOPPARAMETRIZATION_H_INCLUDED */
