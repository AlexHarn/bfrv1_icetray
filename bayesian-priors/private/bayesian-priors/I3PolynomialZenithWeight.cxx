/**
 *
 * @brief implementation of the I3PolynomialZenithWeight class
 *
 * (c) 2007 * the IceCube Collaboration
 * 
 * $Id$
 *
 * @file I3PolynomialZenithWeight.cxx
 * @version $Revision$
 * @date $Date$
 * @author boersma
 *
 */

#include <cmath>
#include "bayesian-priors/I3PolynomialZenithWeight.h"

const std::string I3PolynomialZenithWeight::coefsOptionName_ = "Coefficients";
const std::string I3PolynomialZenithWeight::mincoszenOptionName_ = "MinCosZenith";
const std::string I3PolynomialZenithWeight::defw8OptionName_ = "DefaultWeight";

const double I3PolynomialZenithWeight::DEFAULT_DEFWEIGHT_ = 1.0/9e5;
const double I3PolynomialZenithWeight::DEFAULT_MINCOSZEN_ = 0.1;

static const double TyceNumbers[8] = {
    // numbers taken from like_ama_zenwght2
    // siegmund/recoos/reco_amanda.c, line 205-235
    // See also section 5.2 (pages 44-47):
    // http://icecube.berkeley.edu/manuscripts/20010501xx-ty_thesis.pdf
          -124.41        * 53.21/9e5,
          3212.9         * 53.21/9e5,
        -25377.          * 53.21/9e5,
         27699.          * 53.21/9e5,
             0.39179E+06 * 53.21/9e5,
            -0.10423E+07 * 53.21/9e5,
             0.10268E+07 * 53.21/9e5,
            -0.36495E+06 * 53.21/9e5
};
const std::vector<double>
I3PolynomialZenithWeight::DEFAULT_COEFFICIENTS_(TyceNumbers,TyceNumbers+8);

I3PolynomialZenithWeight::I3PolynomialZenithWeight( const I3Context &context ):
        I3ServiceBase(context), coefficients_(DEFAULT_COEFFICIENTS_){

    defWeight_ = DEFAULT_DEFWEIGHT_;
    AddParameter( defw8OptionName_,
                  "Default weight value for cos(zen) < MinCosZenith",
                  defWeight_ );

    minCosZen_ = DEFAULT_MINCOSZEN_;
    AddParameter( mincoszenOptionName_,
                  "Minimum cos(zenith) value; "
                  "for lower values the default weight is used.",
                  minCosZen_ );

    AddParameter( coefsOptionName_,
                  "Coefficients for a polynomial of cos(zenith). "
                  "The default uses the same coefficients as Ty's P7 zenith "
                  "weight function for AMANDA data.",
                  coefficients_ );

}

double I3PolynomialZenithWeight::EvalPolynomial( double coszen ){
    double cospow = 1.0;
    double w=0;
    std::vector<double>::const_iterator icoef;
    for ( icoef = coefficients_.begin(); icoef != coefficients_.end(); ++icoef ){
        w += (*icoef)*cospow;
        cospow *= coszen;
    }
    return w;
}

/// get configuration parameters
void I3PolynomialZenithWeight::Configure(){
    GetParameter( coefsOptionName_, coefficients_ );
    GetParameter( mincoszenOptionName_, minCosZen_ );
    GetParameter( defw8OptionName_, defWeight_ );

    if ( minCosZen_ >= 1.0 ){
        log_fatal( "%s should get a value lower than 1.0, I got %f",
                   mincoszenOptionName_.c_str(), minCosZen_ );
    }
    if ( minCosZen_ >= -1.0 ){
        if ( defWeight_ <= 0 ){
            log_fatal( "default weight should be positive, I got %g",
                       defWeight_ );
        }
        double wmin = EvalPolynomial(minCosZen_);
        double diff = fabs(wmin-defWeight_);
        if ( (diff > 0.0001) && (diff / defWeight_ > 1 ) ){
            log_warn( "default weight=%g weight(mincoszen=%g)=%g",
                      defWeight_, minCosZen_, wmin );
        }
    }
}


double I3PolynomialZenithWeight::LogWeight(double coszen ){
    assert( coszen*coszen <= 1.0 );
    double w = ( coszen > minCosZen_ ) ? EvalPolynomial(coszen)
                                       : defWeight_;
    log_trace("coszen=%f w=%g",coszen,w);
    assert(w>0);
    return log(w);
}
