/**
 *
 * @brief implementation of the I3TableZenithWeight class
 *
 * (c) 2007 * the IceCube Collaboration
 * 
 * $Id$
 *
 * @file I3TableZenithWeight.cxx
 * @version $Revision$
 * @date $Date$
 * @author boersma
 *
 */

#include <cmath>
#include <algorithm>
#include "bayesian-priors/I3TableZenithWeight.h"

// options: names and default values
const std::string I3TableZenithWeight::tabvalOptionName_ = "TableValues";
const std::string I3TableZenithWeight::mincoszenOptionName_ = "MinCosZenith";

// numbers of default table are taken from like_ama_zenwght & zenith_weight
// this table comes from the observed AMANDA Cosmic Ray spectrum as reported at ICRC2003
// http://www-rccn.icrr.u-tokyo.ac.jp/icrc2003/PROCEEDINGS/PDF/342.pdf
// siegmund/recoos/reco_amanda.c, line 133-146.
const double deftable[11] = {1.0,200,5e3,5e4,1e5,2e5,4e5,5e5,6e5,8e5,9e5};
const std::vector<double> I3TableZenithWeight::defTable_( deftable, deftable+11 );
const double I3TableZenithWeight::defMinCosZen_ = 0.1;

I3TableZenithWeight::I3TableZenithWeight( const I3Context &context ):
        I3ServiceBase(context),
        table_(defTable_), minCosZen_(defMinCosZen_){

    log_trace( "(%s) STARTING to create table-based zenith weight service",
               GetName().c_str() );

    AddParameter( tabvalOptionName_,
                  "Table values, for equidistant values of cos(zenith). "
                  "The table should contain at least 2 values, the first "
                  "the first gives the weight at the minimum cos(zenith) value, "
                  "the first gives the weight at cos(zenith)==1. "
                  "The default uses values originally optimized for AMANDA data.",
                  table_ );
    defWeight_ = table_[0];

    minCosZen_ = defMinCosZen_;
    AddParameter( mincoszenOptionName_,
                  "Minimum cos(zenith) value; "
                  "for lower values the default weight is used.",
                  minCosZen_ );

    log_trace( "(%s) DONE creating table-based zenith weight service",
               GetName().c_str() );
}

/// get configuration parameters
void I3TableZenithWeight::Configure(){

    // get table specs
    GetParameter( tabvalOptionName_, table_ );
    GetParameter( mincoszenOptionName_, minCosZen_ );

    // silly checks
    if ( minCosZen_ >= 1.0 ){
        log_fatal( "(%s) \"%s\" should get a value lower than 1.0, I got %f",
                   GetName().c_str(), mincoszenOptionName_.c_str(), minCosZen_ );
    }
    if ( minCosZen_ < -1.0 ){
        log_warn( "(%s) \"%s\" should get a value larger than -1.0, I got %f, "
                  "forcing it to -1.0",
                  GetName().c_str(), mincoszenOptionName_.c_str(), minCosZen_ );
        minCosZen_ = -1.0;
    }
    if ( table_.size() < 2 ){
        log_fatal( "(%s) The table should have at least two entries, got %zu.",
                   GetName().c_str(), table_.size() );
    }
    double prevval = 0;
    std::vector<double>::iterator ival;
    for ( ival = table_.begin(); ival != table_.end(); ++ival ){
        if ( *ival < 0 ){
            log_fatal( "(%s) Table values should be positive!", GetName().c_str() );
        }
        if ( *ival < prevval ){
            log_warn( "(%s) Table values seem to decreasing towards lower zenith angles...",
                      GetName().c_str() );
        }
        prevval = *ival;
    }

    // convenience variables
    defWeight_ = table_[0];
    double cosZenRange = 1.0 - minCosZen_;
    nBins_ = table_.size() - 1;
    dCosZen_ = cosZenRange/nBins_;

}

double I3TableZenithWeight::Interpolate( double coszen ){
    assert(table_.size()>1);
    double bins = (coszen-minCosZen_)/dCosZen_;
    int i = int( floor( bins ) );
    double f = bins - i;
    if ( i >= nBins_ ){
        // coszen >= 1.0 within rounding errors
        return table_.back();
    }
    if ( coszen <= minCosZen_ ){
        return table_.front();
    }
    double w = (1-f)*table_[i] + f * table_[i+1];
    return w;
}


double I3TableZenithWeight::LogWeight( double coszen ){
    assert( coszen*coszen <= 1.0 );
    double w = ( coszen > minCosZen_ ) ? Interpolate(coszen)
                                       : defWeight_;
    log_trace("coszen=%f w=%g defw=%g",coszen,w,defWeight_);
    assert(w>0);
    return log(w);
}
