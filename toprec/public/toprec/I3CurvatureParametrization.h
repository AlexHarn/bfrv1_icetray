#ifndef I3CURVATUREPARAMETRIZATION_H_INCLUDED
#define I3CURVATUREPARAMETRIZATION_H_INCLUDED

/**
 *
 * @file I3CurvatureParametrization.h
 * @brief declaration of the I3CurvatureParametrization class
 *
 * (c) 2007 the IceCube Collaboration
 * $Id: I3CurvatureParametrization.h 132756 2015-05-21 20:54:27Z hdembinski $
 *
 * @version $Revision: 132756 $
 * @date $Date: 2015-05-21 15:54:27 -0500 (Thu, 21 May 2015) $
 * @author kath
 *
 */

//////////////////////
// The CurvatureFitter is very simple, and only has (for now) one free parameter: "A"
// This code was stolen and adapted from:
//   toprec/public/toprec/I3LaputopParametrization.h
//////////////////////

#include <string>
#include "icetray/I3Units.h"
#include "icetray/I3ServiceBase.h"
#include "gulliver/I3ParametrizationBase.h"


/**
 * @class I3CurvatureParametrization
 * @brief A simple fitter for standalone-curvature
 *
 * For now: simple standard xyzza parametrization, plus one additional parameter
 */
class I3CurvatureParametrization : public I3ServiceBase,
                                    public I3ParametrizationBase {
public:

    /// default constructor for unit tests
    I3CurvatureParametrization( std::string name="unittest" );

    /// constructor I3Tray
    I3CurvatureParametrization(const I3Context &c);

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
    static const double DEFAULT_A_MIN;
    static const double DEFAULT_A_MAX;
    static const double DEFAULT_A_STEPSIZE;
    static const double DEFAULT_D_MIN;
    static const double DEFAULT_D_MAX;
    static const double DEFAULT_D_STEPSIZE;
    static const double DEFAULT_N_MIN;
    static const double DEFAULT_N_MAX;
    static const double DEFAULT_N_STEPSIZE;
    static const double DEFAULT_T_MIN;
    static const double DEFAULT_T_MAX;
    static const double DEFAULT_T_STEPSIZE;

    // Parameter tags & descriptions
    static const std::string MIN_A_TAG;
    static const std::string MIN_D_TAG;
    static const std::string MIN_N_TAG;
    static const std::string MIN_T_TAG;
    static const std::string MAX_A_TAG;
    static const std::string MAX_D_TAG;
    static const std::string MAX_N_TAG;
    static const std::string MAX_T_TAG;
    static const std::string STEPSIZE_A_TAG;    
    static const std::string STEPSIZE_D_TAG;
    static const std::string STEPSIZE_N_TAG;
    static const std::string STEPSIZE_T_TAG;
    static const std::string FREE_A_TAG;
    static const std::string FREE_D_TAG;
    static const std::string FREE_N_TAG;
    static const std::string FREE_T_TAG;
    

    static const std::string MIN_A_DESC;
    static const std::string MIN_D_DESC;
    static const std::string MIN_N_DESC;
    static const std::string MIN_T_DESC;
    static const std::string MAX_A_DESC;
    static const std::string MAX_D_DESC;
    static const std::string MAX_N_DESC;
    static const std::string MAX_T_DESC;
    static const std::string STEPSIZE_A_DESC;
    static const std::string STEPSIZE_D_DESC;
    static const std::string STEPSIZE_N_DESC;
    static const std::string STEPSIZE_T_DESC;
    static const std::string FREE_A_DESC;
    static const std::string FREE_D_DESC;
    static const std::string FREE_N_DESC;
    static const std::string FREE_T_DESC;

private:

    // User parameter booleans
    bool fFreeA_;
    bool fFreeD_;
    bool fFreeN_;
    bool fFreeT_;
    int iparA_;
    int iparD_;
    int iparN_;
    int iparT_;

    // Max, Min, and Stepsizes
    double fAStep_;
    double fDStep_;
    double fNStep_;
    double fTStep_;
    double fAMin_;
    double fDMin_;
    double fNMin_;
    double fTMin_;
    double fAMax_;
    double fDMax_;
    double fNMax_;
    double fTMax_;

    /// log4cplus thingy
    SET_LOGGER( "Curvature" );
};

#endif /* I3CURVATUREPARAMETRIZATION_H_INCLUDED */
