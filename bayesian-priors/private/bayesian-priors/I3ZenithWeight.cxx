/**
 *
 * @brief implementation of the I3ZenithWeight class plus instantiations
 *
 * (c) 2007 * the IceCube Collaboration
 * $Id$
 *
 * @file I3ZenithWeight.cxx
 * @version $Revision$
 * @date $Date$
 * @author boersma
 *
 */

#include "icetray/I3SingleServiceFactory.h"
#include "bayesian-priors/I3ZenithWeight.h"
#include "bayesian-priors/I3PolynomialZenithWeight.h"
#include "bayesian-priors/I3TableZenithWeight.h"
#include "bayesian-priors/I3PowExpZenithWeight.h"
#include "bayesian-priors/I3ConstantZenithWeight.h"

// upon request from code reviewer:
// option names & default values are static data members

template<class WeightCalc>
const std::string I3ZenithWeight<WeightCalc>::flipOptionName_ = "FlipTrack";
template<class WeightCalc>
const std::string I3ZenithWeight<WeightCalc>::penaltyOptionName_ = "PenaltyValue";
template<class WeightCalc>
const std::string I3ZenithWeight<WeightCalc>::slopeOptionName_ = "PenaltySlope";
template<class WeightCalc>
const std::string I3ZenithWeight<WeightCalc>::coszenrangeOptionName_ = "CosZenithRange";

template<class WeightCalc>
const double I3ZenithWeight<WeightCalc>::DEFAULT_PENALTY_ = -200;
template<class WeightCalc>
const bool I3ZenithWeight<WeightCalc>::DEFAULT_FLIP_ = false;
template<class WeightCalc>
const double I3ZenithWeight<WeightCalc>::DEFAULT_SLOPE_ = 0;
template<class WeightCalc>
const double I3ZenithWeight<WeightCalc>::DEFAULT_MINCOSZENITH_ = -1.;
template<class WeightCalc>
const double I3ZenithWeight<WeightCalc>::DEFAULT_MAXCOSZENITH_ = +1.;

/// construct self & declare configuration parameters
template<class WeightCalc>
I3ZenithWeight<WeightCalc>::I3ZenithWeight( const I3Context &context ):
    I3EventLogLikelihoodBase(), WeightCalc(context),
        flip_(DEFAULT_FLIP_), 
        penalty_(DEFAULT_PENALTY_),
        penaltySlope_(DEFAULT_SLOPE_),
        minCosZenith_(DEFAULT_MINCOSZENITH_),
        maxCosZenith_(DEFAULT_MAXCOSZENITH_),
        zenithCache_(NAN),weightCache_(NAN){

    log_debug( "(%s) hey, this is a zenith weight service",
               GetName().c_str());

    std::vector<double> cosZenRange(2);
    cosZenRange[0] = minCosZenith_;
    cosZenRange[1] = maxCosZenith_;
    WeightCalc::AddParameter(
        coszenrangeOptionName_,
        "Minimum and maximum value of cos(zenith) "
        "(before any flipping). ",
        cosZenRange );

    WeightCalc::AddParameter(
        penaltyOptionName_,
        "LogLikelihood value outside the cos(zenith) range. "
        "Should be a large negative number.",
        penalty_ );

    WeightCalc::AddParameter(
        slopeOptionName_,
        "If your goal is to force a reconstruction to end up within "
        "within a specific zenith range, but you don't like setting bounds "
        "on the parameters, then you can stimulate the reconstruction to "
        "'roll' back to the allowed range via a sloped penalty value, i.e. "
        "a penalty which gets worse farther away from the allowed zenith "
        "range, then you should set a negitive value here, e.g. -1000.",
        penaltySlope_ );

    WeightCalc::AddParameter(
        flipOptionName_,
        "Boolean option: if you set this to true, then the weight will "
        "be computed as if the track pointed in the opposite direction.",
        flip_ );

}

/// get configuration parameters
template<class WeightCalc>
void I3ZenithWeight<WeightCalc>::Configure(){

    // configure flip
    WeightCalc::GetParameter( flipOptionName_, flip_ );
    WeightCalc::Configure();

    // configure cos(zenith) range
    std::vector<double> cosZenRange;
    WeightCalc::GetParameter( coszenrangeOptionName_, cosZenRange );
    if ( cosZenRange.size() == 2 ){
        minCosZenith_ = cosZenRange[0];
        maxCosZenith_ = cosZenRange[1];
        if ( ( minCosZenith_ < -1 ) ||
             ( maxCosZenith_ > +1 ) ||
             ( minCosZenith_ >= maxCosZenith_ ) ){
            log_error("(%s) got cos(zenith) min:max = %f:%f",
                       GetName().c_str(), minCosZenith_, maxCosZenith_ );
            log_fatal("(%s) that's not good, should be "
                      "-1<=min<max<=+1", GetName().c_str() );
        }
    } else {
        log_fatal( "(%s) Wrong number of args (%zu) for option %s "
                   "(expected 2 args)",
                   GetName().c_str(), cosZenRange.size(),
                   coszenrangeOptionName_.c_str() );
    }

    // llh to use in case cos(zenith) is out of range
    WeightCalc::GetParameter( penaltyOptionName_, penalty_ );
    if ( penalty_ > 0 ){
        log_warn( "(%s) Penalty is positive (%f)??? "
                  "(expecting large negative number).",
                  GetName().c_str(), penalty_ );
    }

    // llh slope to use in case cos(zenith) is out of range
    // (gently coax the minimizer to roll back to the allowed range)
    WeightCalc::GetParameter( slopeOptionName_, penaltySlope_ );
    if ( penaltySlope_ > 0 ){
        log_warn( "(%s) Penalty slope is positive (%f)??? "
                  "(expecting large negative number).",
                  GetName().c_str(), penaltySlope_ );
    }

}

/// provide event data: the zenith weight function does not use event data.
void SetEvent( const I3Frame &f ){}

/// Get +log(likelihood) for a particular emission hypothesis
template<class WeightCalc>
double
I3ZenithWeight<WeightCalc>::GetLogLikelihood( const I3EventHypothesis &t ){
    double zenith = t.particle->GetZenith();
    if ( zenith == zenithCache_ ){
        return weightCache_;
    }
    zenithCache_ = zenith;
    double cos_zenith = cos(zenith);
    if ( cos_zenith < minCosZenith_ ){
        weightCache_ = penalty_ + penaltySlope_*(minCosZenith_ - cos_zenith);
    } else if ( cos_zenith > maxCosZenith_ ){
        weightCache_ = penalty_ + penaltySlope_*(cos_zenith - minCosZenith_);
    } else {
        if (flip_) cos_zenith *= -1;
        weightCache_ = WeightCalc::LogWeight( cos_zenith );
    }
    return weightCache_;
}

/// tell your name
template<class WeightCalc>
const std::string I3ZenithWeight<WeightCalc>::GetName() const {
    return this->I3ServiceBase::GetName();
}


typedef I3ZenithWeight<I3PolynomialZenithWeight>
I3PolynomialZenithWeightService;
typedef I3SingleServiceFactory< I3PolynomialZenithWeightService,I3EventLogLikelihoodBase >
I3PolynomialZenithWeightServiceFactory;
I3_SERVICE_FACTORY( I3PolynomialZenithWeightServiceFactory )

typedef I3ZenithWeight<I3TableZenithWeight>
I3TableZenithWeightService;
typedef I3SingleServiceFactory< I3TableZenithWeightService,I3EventLogLikelihoodBase >
I3TableZenithWeightServiceFactory;
I3_SERVICE_FACTORY( I3TableZenithWeightServiceFactory )


/**
 * @class I3PowExpZenithWeightServiceFactory
 * @brief This service provides a gulliver I3EventLogLikelihoodBase which includes
 * a prior probability to the liklihood as a function of zenith angle to provide a 
 * "Bayesian" reconstruction
 * 
 * The zenith weight function is a three-parameter analytic
 * function of costh=cos(zenith): w=a0*pow(costh,a1)*exp(-a2/costh);
 * The default values for the constants a0, a1, a2 are the same as for
 * AMANDA data (function like_ama_zenwght3(..), as defined in 
 * siegmund/recoos/reco_amanda.c, line 238-257).
 *
 */
typedef I3ZenithWeight<I3PowExpZenithWeight>
I3PowExpZenithWeightService;
typedef I3SingleServiceFactory< I3PowExpZenithWeightService,I3EventLogLikelihoodBase >
I3PowExpZenithWeightServiceFactory;
I3_SERVICE_FACTORY( I3PowExpZenithWeightServiceFactory )

typedef I3ZenithWeight<I3ConstantZenithWeight>
I3ConstantZenithWeightService;
typedef I3SingleServiceFactory< I3ConstantZenithWeightService,I3EventLogLikelihoodBase >
I3ConstantZenithWeightServiceFactory;
I3_SERVICE_FACTORY( I3ConstantZenithWeightServiceFactory )
