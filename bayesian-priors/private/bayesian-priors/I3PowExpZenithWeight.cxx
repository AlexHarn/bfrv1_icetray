/**
 *
 * @brief implementation of the I3PowExpZenithWeight class
 *
 * (c) 2007 * the IceCube Collaboration
 * 
 * $Id$
 *
 * @file I3PowExpZenithWeight.cxx
 * @version $Revision$
 * @date $Date$
 * @author boersma
 *
 */

#include <cmath>
#include "bayesian-priors/I3PowExpZenithWeight.h"

// upon request from code reviewer:
// option names & default values are static data members

const std::string I3PowExpZenithWeight::amplOptionName_ = "Amplitude";
const std::string I3PowExpZenithWeight::powerOptionName_ = "Power";
const std::string I3PowExpZenithWeight::expOptionName_ = "ExponentFactor";
const std::string I3PowExpZenithWeight::defw8OptionName_ = "DefaultWeight";

// these default values come from siegmund code
// they were fit to a table which comes from the observed AMANDA cosmic
// ray spectrum as reported at ICRC2003:
// http://www-rccn.icrr.u-tokyo.ac.jp/icrc2003/PROCEEDINGS/PDF/342.pdf
const double I3PowExpZenithWeight::DEFAULT_AMPLITUDE_ = 2.49655e-7;
const double I3PowExpZenithWeight::DEFAULT_POWER_ = 1.67721;
const double I3PowExpZenithWeight::DEFAULT_EXPFACTOR_ = 0.778393;
const double I3PowExpZenithWeight::DEFAULT_DEFWEIGHT_ = exp(-200);

I3PowExpZenithWeight::I3PowExpZenithWeight( const I3Context &context ):
        I3ServiceBase(context),
        amplitude_(DEFAULT_AMPLITUDE_),
        power_(DEFAULT_POWER_),
        expFactor_(DEFAULT_EXPFACTOR_),
        defWeight_(DEFAULT_DEFWEIGHT_)
{

    AddParameter( amplOptionName_,
                  "Amplitude factor (a0 in w=a0*pow(z,a1)*exp(-a2/z) "
                  "with z=cos(zenith))",
                  amplitude_ );

    AddParameter( powerOptionName_,
                  "Exponent in the power factor (a1 in w=a0*pow(z,a1)*exp(-a2/z) "
                  "with z=cos(zenith))",
                  power_ );

    AddParameter( expOptionName_,
                  "Numerator in the exponent of the exponent factor "
                  "(a2 in w=a0*pow(z,a1)*exp(-a2/z) "
                  "with z=cos(zenith))",
                  expFactor_ );

    AddParameter( defw8OptionName_,
                  "Default weight for to low cos(zenith) values",
                  defWeight_ );

}

/// get configuration parameters
void I3PowExpZenithWeight::Configure(){
    GetParameter( amplOptionName_, amplitude_ );
    GetParameter( powerOptionName_, power_ );
    GetParameter( expOptionName_, expFactor_ );
    GetParameter( defw8OptionName_, defWeight_ );

    if ( amplitude_ < 0.0 ){
        log_fatal( "(%s) %s should get a positive value, I got %f",
                   GetName().c_str(), amplOptionName_.c_str(), amplitude_ );
    }
    if ( power_ < 0.0 ){
        log_fatal( "(%s) %s should get a positive value, I got %f",
                   GetName().c_str(), powerOptionName_.c_str(), power_ );
    }
    if ( expFactor_ < 0.0 ){
        log_fatal( "(%s) %s should get a positive value, I got %f",
                   GetName().c_str(), expOptionName_.c_str(), expFactor_ );
    }
    if ( defWeight_ <= 0.0 ){
        log_fatal( "(%s) %s should get a positive value, I got %f",
                   GetName().c_str(), defw8OptionName_.c_str(), defWeight_ );
    }
    double w1 = amplitude_ * exp(-expFactor_);
    if ( defWeight_ >= w1 ){
        log_fatal( "(%s) %s should get a lower value than for cos(zenith)=1, "
                   "I got defw8=%f while weight(cos(zenith)=0) = %f",
                   GetName().c_str(), defw8OptionName_.c_str(), defWeight_, w1 );
    }

    logDefWeight_ = log(defWeight_);
    logAmplitude_ = log(amplitude_);
    int nestlevel = 10;
    log_debug( "(%s) Going to approximate min cos zenith, "
               "starting at nesting level %d",
               GetName().c_str(), nestlevel );
    minCosZenith_ = FindMinimumCosZenith( 0.0, 1.0, 0.01, nestlevel );
}

double I3PowExpZenithWeight::FindMinimumCosZenith(
        double logz, double dlogz, double tol, int nestlevel ){
    if ( nestlevel == 0 ){
        log_fatal("(%s) Reached final nest level...", GetName().c_str());
        return NAN;
    }
    double z = exp(logz);
    while ( z>0 ){
        double fz = logAmplitude_ + power_ * logz - expFactor_/z;
        log_trace( "(%s) nesting level %d, z=costh=%g, logz=%f, dlogz=%f, w=%g, logdefw=%g tol=%g",
                   GetName().c_str(), nestlevel, z, logz, dlogz, fz, logDefWeight_, tol );
        if ( logDefWeight_ > fz ){
            if ( logDefWeight_ - fz < tol ){
                log_debug( "(%s) Found min cos zenith at nesting level %d",
                           GetName().c_str(), nestlevel);
                return z;
            }
            return FindMinimumCosZenith(logz+dlogz,0.1*dlogz,tol,nestlevel - 1);
        }
        logz -= dlogz;
        z = exp(logz);
    }
    return 0;
}

double I3PowExpZenithWeight::LogWeight(double coszen ){
    assert( coszen*coszen <= 1.0 );
    double logw = logDefWeight_;
    if ( coszen > minCosZenith_ ){
        logw = logAmplitude_ + power_ * log(coszen) - expFactor_/coszen;
        if ( logw < logDefWeight_ ){
            logw = logDefWeight_;
        }
    }
    log_debug("%f %f %f %f %f",minCosZenith_,logAmplitude_,power_,expFactor_,logDefWeight_);
    log_debug( "coszen=%f log(w)=%g (log(defw8)=%g)",
               coszen, logw, logDefWeight_ );
    return logw;
}
