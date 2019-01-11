/*
 *  $Id$
 *  @version $Revision$
 *  @date $Date$
 *  @author emiddell
*/


// standard library and tools
#include <boost/shared_ptr.hpp>
#include <cmath>
#include <cfloat>
#include <cassert>
#include <utility>
#include <algorithm>
#include "gsl/gsl_sf_gamma.h"

#include "credo/I3PoissonGaussLogLikelihood.h"
#include "credo/I3CredoFunctions.h"
#include "gulliver/I3EventHypothesis.h"
#include "photonics-service/I3PhotonicsServiceFactory.h"

// icetray/dataclasses stuff
#include "icetray/I3Frame.h"
#include "dataclasses/I3Position.h"
#include "dataclasses/I3Direction.h"
#include "icetray/OMKey.h"
#include "dataclasses/physics/I3Particle.h"
#include "dataclasses/geometry/I3Geometry.h"
#include "dataclasses/status/I3DetectorStatus.h"
#include "dataclasses/calibration/I3Calibration.h"
#include "phys-services/I3Calculator.h"
#include "phys-services/geo-selector/I3GeoTrimmers.h"

I3PoissonGaussLogLikelihood::I3PoissonGaussLogLikelihood( const std::string& serviceName,
                                                          I3PhotonicsServicePtr pdf,
                                                          const std::string& pulsesrc,
                                                          double noiserate,
                                                          double eventlength,
                                                          double activevolume,
                                                          double gaussianErrorConstant,
                                                          bool onlyATWD,
                                                          bool useBaseContrib,
                                                          bool useEmptyPulses,
                                                          double light_scale) :
     I3EventLogLikelihoodBase(),
     pdf_(pdf),
     serviceName_(serviceName),
     inputPulses_(pulsesrc),
     noiseRate_(noiserate),
     constEventLength_(eventlength>0),
     activeVolume_(activevolume),
     onlyATWD_(onlyATWD),
     useBaseContribution_(useBaseContrib),
     useEmptyPulses_(useEmptyPulses),
     gaussianErrorConstant_(gaussianErrorConstant),
     eventLength_(eventlength),
     light_scale_(light_scale),
     minChargeFraction_(0.),
     saturationLimit_(0),
     photonicsSaturation_(false),
     eventCounter_(0),
     multiplicity_(0) {

    // noiserate must be positive
    if (noiseRate_ <= 0){
        log_fatal("(%s) noise rate %f<=0; must be positive", serviceName_.c_str(), noiseRate_ );
    }
    if (activeVolume_ <= 0){
        log_fatal("(%s) active volume %f<=0; must be positive", serviceName_.c_str(), activeVolume_ );
    }
    if (gaussianErrorConstant_ <= 0){
        log_fatal("(%s) gaussian error constant %f<=0; must be positive", serviceName_.c_str(), gaussianErrorConstant_ );
    }
    if ((minChargeFraction_ < 0) || (1 <minChargeFraction_)){
        log_fatal("(%s)minimum charge fraction  %f must be in [0,1]", serviceName_.c_str(), minChargeFraction_ );
    }
    
    log_warn("(%s) Will scale amplitudes predicted by Photonics with a constant factor of %f.", serviceName_.c_str(), light_scale_);
       
    if (onlyATWD_) {
        log_warn("(%s) Assuming that the pulsemap contains ATWD pulses only - Hence will down scale the total charge predicted by photonics according to distance between vertex and DOM (to account for the missing fraction of the arrival time pdf)", serviceName_.c_str());
    } 
	 
    noiseChargeEvent_ = eventLength_ * noiseRate_; 
    maxPerpDistSqr_ = activeVolume_ * activeVolume_;
    npeThreshold_ = gaussianErrorConstant_ * gaussianErrorConstant_;
    gaussianBaseTerm_ = log(sqrt(2*I3Constants::pi)/gaussianErrorConstant_);
}

//----------------------------------------------------------------------------

I3PoissonGaussLogLikelihood::~I3PoissonGaussLogLikelihood(){}


//----------------------------------------------------------------------------
void I3PoissonGaussLogLikelihood::SetEvent( const I3Frame &f ){

    ++eventCounter_;
    log_info( "(%s) likelihood getting event data for event nr. %d",
              serviceName_.c_str(), eventCounter_ );

    // we need the gcd objects from the frame. 
    I3GeometryConstPtr geometry = f.Get<I3GeometryConstPtr>();
    I3DetectorStatusConstPtr status = f.Get<I3DetectorStatusConstPtr>();
    I3CalibrationConstPtr calibration = f.Get<I3CalibrationConstPtr>();

    bool gcdFound = true;
    if (!geometry) {log_warn("(%s) could not find geometry!", serviceName_.c_str()); gcdFound = false;}
    if (!status) {log_warn("(%s) could not find detector status!", serviceName_.c_str()); gcdFound = false;}
    if (!calibration) {log_warn("(%s) could not find calibration!", serviceName_.c_str()); gcdFound = false;}

    if (!gcdFound) {
        multiplicity_=0;
        return;
    }

    // clear the cache from the previous event
    cacheMap_.clear(); 
    assert(cacheMap_.size()==0);

    // grab the pulses 
    if (useEmptyPulses_)
        pulsesmap_ = I3Credo::GetPulseMapWithGaps( f.Get<I3RecoPulseSeriesMapConstPtr>( inputPulses_ ),
                                                   onlyATWD_);
    else {
        // create a copy of the pulsemap to be able to modify it
        const I3RecoPulseSeriesMap& pmap = f.Get<I3RecoPulseSeriesMap>( inputPulses_);
        pulsesmap_ = I3RecoPulseSeriesMapPtr(new I3RecoPulseSeriesMap(pmap));
    }

    I3RecoPulseSeriesMap::iterator i_pmap;
    std::vector<I3RecoPulse>::iterator i_pls;

    if ( !pulsesmap_ ){
        log_warn( "(%s) Could not retrieve pulses map with name %s", serviceName_.c_str(), inputPulses_.c_str() );
        multiplicity_=0;
        return;
    }

    I3OMGeoMapPtr cleanGeoMapPtr;
    
    // there are two lists of bad DOMs. a static one in badDOMList_ and
    // eventually one in the frame named badDOMListName_. The former
    // was set by SetBadDOMs and therefore contains only unique keys.
    // The list in the frame may contain duplicates within iteself and 
    // with respect to badDOMList_. Hence, create a joined set.
    if (!badDOMListName_.empty() ) {
        std::set<OMKey> uniqueBadDoms(badDOMList_.begin(), badDOMList_.end());
        I3VectorOMKeyConstPtr framebaddomlist = f.Get<I3VectorOMKeyConstPtr>( badDOMListName_);
        if (framebaddomlist) {
            uniqueBadDoms.insert(framebaddomlist->begin(), framebaddomlist->end());
        }
        else {
          log_warn("(%s) this frame does not contain a bad dom list named %s",  
                   serviceName_.c_str(),badDOMListName_.c_str());
        }
        std::vector<OMKey> combinedbadDOMs(uniqueBadDoms.begin(), uniqueBadDoms.end());
        cleanGeoMapPtr = I3GeoTrimmers::GeoWithoutBadOMs(geometry->omgeo, combinedbadDOMs);
    }
    else
        cleanGeoMapPtr = I3GeoTrimmers::GeoWithoutBadOMs(geometry->omgeo, badDOMList_);

    // get a sub-geometry with only those DOMs that are within the "minimum ball" around the hits.
    I3OMGeoMapPtr minballMapPtr = I3GeoTrimmers::GetMinBallGeometryFromData(*cleanGeoMapPtr, *pulsesmap_, 0.1*I3Units::m);
    
    I3OMGeoMap::const_iterator i_geo;
    
    // multiplicity (for reduced llh)
    multiplicity_=0; // number of pulses in all oms + number of nohit DOMs


    // some event describing counters for logging
    stats_.Reset();
    stats_.nCh_hit = pulsesmap_->size();
    
    // 1st loop over pulsemap to compute event length, maximum charge and update some counters
    for (i_pmap = pulsesmap_->begin(); i_pmap != pulsesmap_->end(); ++i_pmap){
        double domcharge(0);
        const OMKey &omkey = i_pmap->first;

        // skip eventual IceTop DOMs
        if (omkey.GetOM() > 60)
            continue;

        // make sure that the DOM is in the cleaned geometry
        if (minballMapPtr->find(omkey) == minballMapPtr->end() )
            continue;

        // investigate event time and pulse charges
        for ( i_pls = i_pmap->second.begin(); i_pls != i_pmap->second.end(); ++i_pls ){
            double width  = i_pls->GetWidth();
            double LEtime = i_pls->GetTime();
            double npe    = i_pls->GetCharge();

            if ( !((std::isfinite(npe) && (npe >= 0)) && (std::isfinite(width) && (width>0)) && (std::isfinite(LEtime))) ) {
                stats_.nPulses_all_bad++; // FIXME: move counter after tweak
                continue;
            }
            domcharge += npe;
            ++stats_.nPulses_all_good;
            stats_.npe_all += npe; 

            if ( stats_.start_time > LEtime )         stats_.start_time = LEtime;
            if ( stats_.end_time   < LEtime + width ) stats_.end_time   = LEtime + width;
        }
        if (domcharge > stats_.npe_max) //maxdomcharge)
            stats_.npe_max = domcharge;
    } // end of first loop of all pulses
    
    // lower charge limit
    double lowerchargelimit = minChargeFraction_ * stats_.npe_max;
    
    // only use the computed eventlength if explicitely asked for
    if ( ! constEventLength_ )
        eventLength_ = stats_.end_time - stats_.start_time;

    noiseChargeEvent_ = eventLength_ * noiseRate_;

    // 2nd loop: now over all DOMs in the minimum ball 
    // calculate and cache the contributions to the likelihood that don't depend on the hypothesis
    for ( i_geo = minballMapPtr->begin(); i_geo != minballMapPtr->end(); ++i_geo ){
        double baseContribution(0);
        double outOfBoundContribution(0);
        double firstPulseTime(+HUGE_VAL);
        unsigned int multiplicitycontribution(0);
        double totalNPE(0);
        double poissonNPE(0);
        unsigned int nPulses(0);
        unsigned int nPoissonPulses(0);
        unsigned int nGaussianPulses(0);
        bool saturated(false);
        double relativedomefficiency(1.0);
        I3CredoDOMCache::OMType omtype = I3CredoDOMCache::UNSPECIFIED;

        const OMKey &omkey = i_geo->first;
        
        // find a reason to skip this DOM
        if (omkey.GetOM() > 60) // IceTop
            continue;

        // check the DetectorStatus for this OM
        // IceCube DOMs
        // check that this OM is in DetectorStatus map and has a high voltage != 0
        std::map<OMKey, I3DOMStatus>::const_iterator i_domstatus = status->domStatus.find(omkey);
        if( i_domstatus == status->domStatus.end() )  { 
          log_trace("(%s) DOM (%d,%d) was in the geometry but is not in the detector status. Skipping it!",
                    serviceName_.c_str(), omkey.GetString(), omkey.GetOM()); 
          stats_.skipped_noStatus++;
          continue;
        }
        else {
          const I3DOMStatus& domstatus = i_domstatus->second;
          if( std::abs(domstatus.pmtHV/I3Units::volt) < DBL_EPSILON ) {
            log_trace("(%s) DOM (%d,%d) is not turned on.  Skipping it!!", 
                      serviceName_.c_str(), omkey.GetString(), omkey.GetOM());
            stats_.skipped_noHV++;
            continue;
          }
        }         
        
        // define type, skip OM if not in the calibration, read out qe
        std::map<OMKey, I3DOMCalibration>::const_iterator i_domcal = calibration->domCal.find(omkey);
        if( i_domcal == calibration->domCal.end() )  { 
          log_trace("(%s) DOM (%d,%d) was in the geometry but is not in the calibration. Skipping it!",
                    serviceName_.c_str(), omkey.GetString(), omkey.GetOM()); 
          stats_.skipped_noCalibration++;
          continue;
        }

        const I3DOMCalibration& domcal = i_domcal->second;
        relativedomefficiency = domcal.GetRelativeDomEff();
        
        if ( std::isnan(relativedomefficiency) ) {
          log_debug("(%s) DOM (%d,%d)'s relative efficiency is nan!", serviceName_.c_str(), omkey.GetString(), omkey.GetOM());
          relativedomefficiency = 1.; // reasonable default...
        }
        
        if (omkey.GetString() > 80) {
          omtype = I3CredoDOMCache::DEEPCOREDOM;
        }
        else { 
          omtype = I3CredoDOMCache::ICECUBEDOM;
        }

        // find DOM in pulsemap and test if the DOM has reconstructed pulses
        i_pmap = pulsesmap_->find(omkey);

        if ( i_pmap != pulsesmap_->end() ) {
            
            // loop over pulses
            for ( i_pls = i_pmap->second.begin(); i_pls != i_pmap->second.end(); ++i_pls ){
                double width  = i_pls->GetWidth(); 
                double LEtime = i_pls->GetTime();
                double npe = i_pls->GetCharge();

                // throw away bad pulses
                if ( !((std::isfinite(npe) && (npe >= 0)) && (std::isfinite(width) && (width>0)) && (std::isfinite(LEtime))) ) {
                    log_warn( "(%s) got bad pulse with PE=%f width=%f LE=%f in OM %02d.%02d", serviceName_.c_str(), 
                              npe, width, LEtime,omkey.GetString(), omkey.GetOM() );
                    continue;
                }
                
                // find the earliest pulse in this DOM
                if (LEtime < firstPulseTime)
                    firstPulseTime = LEtime;

                // update counters
                ++nPulses;
                totalNPE += npe;

                // add contribution of this pulse
                if (npe < npeThreshold_) {                                          // poissonian pulse
                    ++nPoissonPulses;
                    poissonNPE += npe;
                    baseContribution += gsl_sf_lngamma(1.0+npe);
                    // this calculation assumes that the sum over all
                    // expected pulsecharges sum up to the mean expected amplitude
                    // given by photonics
                    outOfBoundContribution -= npe * log(width / eventLength_);  
                }
                else {                                                              // gaussian pulse
                    ++nGaussianPulses;
                    double tmp = (npe - noiseRate_*width) / npe;
                    outOfBoundContribution += gaussianErrorConstant_*gaussianErrorConstant_ * tmp * tmp /2;
                }
            }
        }
        // empty pulseseries are treated like an unhit DOM        
        // next round of reasons to skip this OM
        // if configured: skip DOMs with totalNPE < lowerchargelimit
        if ( (lowerchargelimit > 0) && (totalNPE < lowerchargelimit)) {
            stats_.skipped_mincharge++;
            continue;
        }

        // if configured: test for saturated DOMs
        if ( (omtype == I3CredoDOMCache::ICECUBEDOM) && (saturationLimit_ > 0) && 
             (totalNPE > saturationLimit_) )
            saturated = true;

        if (saturated)
            stats_.skipped_saturated++; // counts saturated but DOMs. These will be skipped later. Hence the name.

        //
        // every OM that reaches this point will enter the likelihood calculation
        //

        // calculate final contributions
        if (totalNPE < DBL_EPSILON) { // unhit DOMs 
            baseContribution += noiseChargeEvent_;
            multiplicitycontribution=1;
        } 
        else { // hit DOMs (poissonian or gaussian)
            baseContribution += noiseChargeEvent_ + nGaussianPulses*gaussianBaseTerm_;  // appr: sum(mu_oi) = mu_o -> use of noiseChargeEvent_
            outOfBoundContribution -= poissonNPE * log(noiseChargeEvent_);
            multiplicitycontribution=nPulses;
        }

        // update mulitplicity and some statistics
        multiplicity_ += multiplicitycontribution;
        stats_.npe_selected_all += totalNPE;
        stats_.npe_selected_poisson += poissonNPE;
        stats_.nPulses_selected_poisson  += nPoissonPulses; 
        stats_.nPulses_selected_gaussian += nGaussianPulses;

         
        // store the calculated values in the cachemap
        const I3Position &pos = i_geo->second.position;
        cacheMap_[omkey] = I3CredoDOMCache(pos, totalNPE, poissonNPE, baseContribution, 
                                           outOfBoundContribution, firstPulseTime, 
                                           i_pmap, omtype, saturated, relativedomefficiency);
    } // end of loop over all DOMs in minimum ball 

    log_info( "(%s) event nr. %d: we have %u DOMs with total %.2f PE", serviceName_.c_str(), eventCounter_, stats_.nCh_hit, stats_.npe_all);
    log_info( "(%s) we use %u/%u pulses (%u poisson, %u gaussian, %u bad)", serviceName_.c_str(), 
              stats_.nPulses_selected_poisson + stats_.nPulses_selected_gaussian,
              stats_.nPulses_all_good + stats_.nPulses_all_bad, 
              stats_.nPulses_selected_poisson,
              stats_.nPulses_selected_gaussian,
              stats_.nPulses_all_bad);
    log_info( "(%s) skipped %u DOMs (reasons: saturation: %u,  minimum charge: %u detector status: %u)", serviceName_.c_str(),
              stats_.skipped_saturated+stats_.skipped_mincharge+stats_.skipped_noStatus+stats_.skipped_noHV,
              stats_.skipped_saturated, stats_.skipped_mincharge, stats_.skipped_noStatus + stats_.skipped_noHV);

    return;
}

//----------------------------------------------------------------------------

double I3PoissonGaussLogLikelihood::GetLogLikelihood( const I3EventHypothesis &q ){

    log_trace( "About to calculate LLH");

    // retrieve the parameters of the cascade hypothesis
    assert(q.particle);
    const I3Particle &p = *(q.particle);
    const I3Direction &dir = p.GetDir();
    const I3Position &vertex = p.GetPos();

    double x = vertex.GetX()/I3Units::m;
    double y = vertex.GetY()/I3Units::m;
    double z = vertex.GetZ()/I3Units::m;
    double time = p.GetTime()/I3Units::ns;
    double zenith = dir.GetZenith()/I3Units::degree;
    double azimuth = dir.GetAzimuth()/I3Units::degree;
    double energy = p.GetEnergy();
    double length = 0; // with photonics-service the length has to be 0 for cascades
                       // otherwise some wrong scaling of the amplitude is done
            
    log_debug( "(%s) hypothesis x=%f y=%f z=%f t=%f "
               "theta=%.1fdeg phi=%.1fdeg energy=%.3g GeV length=%fm",
               serviceName_.c_str(), x, y, z, time, zenith, azimuth, energy, length );

    // silly checks
    assert( ! std::isnan( x ) );
    assert( ! std::isnan( y ) );
    assert( ! std::isnan( z ) );
    assert( ! std::isnan( zenith ) );
    assert( ! std::isnan( azimuth ) );
    assert( ! std::isnan( time ) );

    // if energy is not fitted: use some arbitrary value of 1 TeV
    // otherwise photorec may return NAN
    if ( std::isnan(energy) || (energy <= 0) ) 
        energy = 1.0 * I3Units::TeV;

    // the result variable
    double logLikelihood(0);

    const int photonicsType = 1; // e.m. cascade 

    PhotonicsSource photonics_source(x,y,z,zenith,azimuth,0.,length,energy,photonicsType);
    
    // loop over DOMs in the cachemap
    std::map<OMKey,I3CredoDOMCache>::iterator i_dom;
    for ( i_dom = cacheMap_.begin(); i_dom != cacheMap_.end(); ++i_dom) {
        #ifndef NDEBUG // only needed for log_trace messages
        const OMKey &omkey = i_dom->first;
        int stringnr = omkey.GetString();
        unsigned int omnr = omkey.GetOM();
        #endif

        double domContribution = 0.;
      
        // domcache holds both the dom geometry and its event data information
        I3CredoDOMCache& cachedValues = i_dom->second;
        const I3Position &ompos = cachedValues.position; 

        // skip saturated DOMs
        if (cachedValues.saturated)
            continue;
 
	    pdf_->SelectModuleCoordinates(ompos.GetX(),ompos.GetY(),ompos.GetZ());
        
        double pdsqr = I3Credo::DistSquared(vertex,ompos);

        if (useBaseContribution_) {
            domContribution += cachedValues.baseContribution;
        } 

        // for a DOM outside the active volume of the photonics tables
        // the calculation of it's contribution to the llh ends here
        if ( pdsqr > maxPerpDistSqr_ ){
            log_trace( "(%s) DOM(%d,%u) outside of active volume.", serviceName_.c_str(), stringnr, omnr );
            domContribution += cachedValues.outOfBoundContribution;

            cachedValues.llhcontrib = domContribution;
            logLikelihood += domContribution;
            continue;
        }

        log_trace( "(%s) DOM(%d,%u) within active volume", serviceName_.c_str(), stringnr, omnr );
            
        // get expected total charge in this DOM from photonics
        double dummyfill=0.; 
        double mean_total_npe=0.; 
        double dummytime = 100.0;
            
        if (pdf_->GetPhotorecInfo(mean_total_npe,dummyfill,dummytime,photonics_source)==false) {
          log_trace("The Photorec call failed!");
          stats_.failed_photorec_calls++;
          continue; 
        }
        log_trace("The Photorec call yielded a mean amplitude of %e", mean_total_npe);

        // check result 
        if ( !std::isfinite( mean_total_npe ) || ( mean_total_npe < 0) )
            mean_total_npe = 0;
        
        // calculate the distance between the DOM and and the hypothesis
        GetDirectTimeDistance(p, ompos, pdsqr);

        
        // apply correction factors to charge prediction
        double correctionFactor = light_scale_;
        
        // store the photonics prediction in the cachemap for diagnostics
        cachedValues.expectedAmpliude = mean_total_npe;
        cachedValues.amplitudeCorrection = correctionFactor;
        
        // apply more corrections, quantum efficiency for all DOMs

        mean_total_npe *= cachedValues.relativedomefficiency;
        log_trace("The relative quantum efficiency of this DOM has been read out to %e", cachedValues.relativedomefficiency);
        
        // and apply the photonics correction factor
        mean_total_npe *= correctionFactor;

        // if configured clip the photonics prediction to defined saturation limits
        if (photonicsSaturation_) {
            if ((cachedValues.type == I3CredoDOMCache::ICECUBEDOM) && 
                (saturationLimit_ > 0) && (saturationLimit_ < mean_total_npe) )
                mean_total_npe = saturationLimit_;
        }
            

        // for an unhit DOM the calculation of its contribution to the llh ends here 
        if (cachedValues.npe == 0) {
            domContribution += mean_total_npe; 
            
            cachedValues.llhcontrib = domContribution;
            logLikelihood += domContribution;
            continue;
        }

        // now deal with hit DOMs

        // total expected charge in this DOM from signal and noise
        double totalExpectedChargeWithNoise  = mean_total_npe + noiseChargeEvent_;

        // poisson contribution : mu_o ~ totalExpectedChargeWithNoise - gaussian npe
        domContribution += totalExpectedChargeWithNoise - (cachedValues.npe - cachedValues.nPoissonPE); // appr: mu_oi = m_o !!!
        domContribution -= cachedValues.nPoissonPE * log( totalExpectedChargeWithNoise );

        // now loop over the reconstructed pulses of the hit DOMs 
        I3RecoPulseSeriesMap::const_iterator i_pmap = cachedValues.i_pmap;

        if ( i_pmap != pulsesmap_->end() ){
            std::vector<I3RecoPulse>::const_iterator i_pls;
            for ( i_pls = i_pmap->second.begin(); i_pls != i_pmap->second.end(); ++i_pls ){
                double LEtime = i_pls->GetTime();
                double width  = i_pls->GetWidth();
                //double npe    = skipWeights_ ? 1.0 : i_pls->GetCharge();
                double npe    = i_pls->GetCharge();
                
                if ( !((std::isfinite(npe) && (npe >= 0)) && (std::isfinite(width) && (width>0)) && (std::isfinite(LEtime))) )
                    continue;

                log_trace( "(%s) Got a recopulse with t=%fns, q=%fpe, tot=%fns",
                           serviceName_.c_str(), LEtime/I3Units::ns, npe, width/I3Units::ns );
 
                double prob=-1; 
                double expectedPulseCharge=-1;
                double tres = LEtime - directTime_; // delay time
                
                //if ( midPulse_ )
                //    tres += 0.5 * width;
                
                if (pdf_->GetPhotorecInfo(dummyfill,prob,tres,photonics_source)==false) {
                  log_trace("The Photorec call failed!");
                  continue;
                }
                // calculate photonics prediction
                // for IceCube calculate the charge basically as mean_npe * dp_dt * width + noise
                if ( std::isfinite(prob) && (prob > 0) && (dummyfill>0) )
                  expectedPulseCharge = (mean_total_npe*prob + noiseRate_)*width;
                else
                  expectedPulseCharge = noiseRate_*width;


                if (npe < npeThreshold_) { 
                    log_trace("poisson pulse expected %.4f seen %.4f", expectedPulseCharge, npe);
                    domContribution -= npe * log (expectedPulseCharge/totalExpectedChargeWithNoise); 
                } 
                else {
                    log_trace("gausian pulse expected %.4f seen %.4f", expectedPulseCharge, npe);
                    double tmp = (npe - expectedPulseCharge) / expectedPulseCharge;

                    domContribution += log(expectedPulseCharge);
                    domContribution += tmp*tmp*gaussianErrorConstant_*gaussianErrorConstant_/2.0;
                }
            } // end loop over pulses
        } // end : if ( i_pmap != pulsesmap_->end() )
        cachedValues.llhcontrib = domContribution;
        logLikelihood += domContribution;
    } // OM loop finish
    
    // with llh calculated, return it.
    log_debug("llh is %f", logLikelihood);
    if( !std::isfinite(logLikelihood) ){
        log_warn("got bad llh=%f", logLikelihood);
        logLikelihood=0;
    }
    
    return -logLikelihood;
}


//----------------------------------------------------------------------------

void I3PoissonGaussLogLikelihood::GetDirectTimeDistance( const I3Particle& p, const I3Position& pos, double pdsqr){
    double chtime( NAN );
    const I3Position &vertex = p.GetPos();
    distance_ = (pos - vertex).Magnitude();
    chtime = distance_ * I3Constants::n_ice_group / I3Constants::c;
    assert( ! std::isnan(chtime) );
    
    directTime_=p.GetTime() + chtime;
}

