/**
 *
 * @brief declaration of the I3GulliverIPDFPandel class (Gulliver wrapper for ipdf)
 *
 * (c) 2005
 * the IceCube Collaboration
 * $Id$
 *
 * @file I3GulliverIPDFPandel.h
 * @version $Revision$
 * @date $Date$
 * @author boersma
 *
 */
#ifndef I3GULLIVER_IPDF_PANDEL_H_INCLUDED
#define I3GULLIVER_IPDF_PANDEL_H_INCLUDED

// standard library stuff
// #include <vector>
// #include <string>
// #include <algorithm>

// Gulliver stuff
#include "gulliver/I3EventLogLikelihoodBase.h"

// ipdf stuff
#include "ipdf/I3/I3HitOm.h"
#include "ipdf/I3/I3OmReceiver.h"
#include "ipdf/Pandel/PandelConstants.h"
#include "ipdf/Pandel/IceModel.h"
#include "ipdf/I3/I3DetectorConfiguration.h"
#include "ipdf/I3/I3DetectorResponse.h"
#include "ipdf/AllOMsLikelihood.h"

// framework stuff
#include "dataclasses/physics/I3Particle.h"
#include "dataclasses/geometry/I3Geometry.h"
#include "dataclasses/physics/I3RecoPulse.h"
#include "gulliver/I3EventHypothesis.h"
#include "photonics-service/I3PhotonicsService.h"
#include "icetray/I3Logging.h"
#include "icetray/I3Frame.h"
#include "icetray/I3Units.h"

// #include "icetray/I3Int.h"

/**
 * @brief A wrapper for IPDFPandel (only muons, currently) Gulliver-based reconstruction.
 * 
 * @todo Add a constructor which takes a smart pointer to IPDFPandelEventLikelihood
 *       (necessary for layered ice etc.)
 * @todo Maybe make the response type (hit, pulse, amandaanalog) a template argument (urgh)
 */
template<typename IPDFPandelEventLikelihood,typename IpdfEventType>
class I3GulliverIPDFPandel : public I3EventLogLikelihoodBase {
private:
    std::string name_;
    typename boost::shared_ptr< IPDFPandelEventLikelihood > ipdfllh_;
    boost::shared_ptr< const IPDF::I3DetectorConfiguration> dgeomptr_;
    boost::shared_ptr< const IPDF::I3DetectorResponse > dresponseptr_;
    I3GulliverIPDFPandel();
    const std::string inputReadout_;
    unsigned int nReadoutMissing_;
    unsigned int nEmptyReadout_;
    unsigned int nSetEvent_;
    static const unsigned int Nmissmax = 25;
public:

    I3GulliverIPDFPandel( std::string name, typename boost::shared_ptr< IPDFPandelEventLikelihood > ipdfptr,
                          const std::string &inputdata ):
        name_(name), ipdfllh_(ipdfptr), inputReadout_(inputdata),
        nReadoutMissing_(0),nEmptyReadout_(0),nSetEvent_(0){}

    virtual ~I3GulliverIPDFPandel(){
        if ( nReadoutMissing_ > 0 ){
            log_warn("(%s) For %u out of %u SetEvent() calls, there were no "
                     "hits, pulses or AMANDA analog readouts named \"%s\".",
                     name_.c_str(), nReadoutMissing_, nSetEvent_,
                     inputReadout_.c_str() );
        }
        if ( nEmptyReadout_ > 0 ){
            log_warn("(%s) For %u out of %u SetEvent() calls, the input data "
                     "\"%s\" existed in the frame, but it was completely empty.",
                     name_.c_str(), nEmptyReadout_, nSetEvent_,
                     inputReadout_.c_str() );
        }
    }

    void SetGeometry( const I3Geometry &geo ){

        // now make a new IPDF-detectorconfig object
        // we need to keep it, because the domresponse object will
        // indirectly keep references to elements of it (in the I3PEHit class)
        dgeomptr_ = boost::shared_ptr<const IPDF::I3DetectorConfiguration>(
                new IPDF::I3DetectorConfiguration( geo ) );
    }

    bool CheckPulses(I3RecoPulseSeriesMapConstPtr pulsemap){
        size_t npulses = 0;
        for (I3RecoPulseSeriesMap::const_iterator ipm = pulsemap->begin();
                                                  ipm != pulsemap->end();
                                                  ipm++ ){
            npulses += ipm->second.size();
        }
        if (npulses == 0){
            dresponseptr_.reset();
            ++nEmptyReadout_;
            // Repeat this warning an arbitrary number of times,
            // then give up and yell one more time at the end (see destructor).
            if (++nEmptyReadout_ <= Nmissmax ){
                log_info( "(%s) found pulse map '%s', but it appears to be empty.",
                          name_.c_str(), inputReadout_.c_str() );
            }
            if ( nEmptyReadout_ == Nmissmax ){
                log_info( "(%s) I won't report this anymore till the end.",
                          name_.c_str() );
            }
            return false;
        }
        return true;
    }

    /// provide event data
    void SetEvent( const I3Frame &f ){

        ++nSetEvent_;

        I3RecoPulseSeriesMapConstPtr pulsemap =
                f.template Get< I3RecoPulseSeriesMapConstPtr >(inputReadout_);
        I3AMANDAAnalogReadoutMapConstPtr mudaqmap =
                f.template Get< I3AMANDAAnalogReadoutMapConstPtr >(inputReadout_);

        assert( ! (mudaqmap && pulsemap) );


        // now make a new IPDF-domresponse object
        if ( pulsemap ){
            dresponseptr_ = boost::shared_ptr<const IPDF::I3DetectorResponse>(
                    new IPDF::I3DetectorResponse(pulsemap,*dgeomptr_) );
        } else if ( mudaqmap ){
            dresponseptr_ = boost::shared_ptr<const IPDF::I3DetectorResponse>(
                    new IPDF::I3DetectorResponse(mudaqmap,*dgeomptr_) );
        } else {
            if ( nReadoutMissing_ <= Nmissmax ){
                log_info( "(%s) could not get hits, "
                          "pulses or analogs called \"%s\"",
                          name_.c_str(), inputReadout_.c_str() );
            }
            if ( nReadoutMissing_ == Nmissmax ){
                log_info( "(%s) I won't report this anymore till the end.",
                          name_.c_str() );
            }
            ++nReadoutMissing_;
            dresponseptr_.reset();
        }
        return;

    }

    /// Get +log(likelihood) for a particular emission hypothesis
    double GetLogLikelihood( const I3EventHypothesis &eh ){
        const I3Particle &p = *(eh.particle);
        if ( ! dresponseptr_ ) return NAN;
        if ( dresponseptr_->size() == 0 ) return NAN;
        double energy = std::isfinite( p.GetEnergy() ) ? p.GetEnergy() : 1.0*I3Units::TeV; // random default
        double t = p.GetTime();
        I3Direction dir = p.HasDirection() ? p.GetDir() : I3Direction(0,0,1);
        const I3Position &pos = p.GetPos();
        log_debug( "seed x=%f y=%f z=%f th=%f ph=%f E=%g t=%g",
                  pos.GetX(), pos.GetY(), pos.GetZ(),
                  dir.GetZenith(), dir.GetAzimuth(), energy, t );
        IpdfEventType ipdfeh(pos,dir,energy,t);
        double llh = ipdfllh_->getLogLikelihood(*dresponseptr_, ipdfeh);

        log_debug( "(%s) got llh=%f", name_.c_str(), llh );
        return llh;
    }

    /**
     * Get the multiplicity of the current input event (e.g. number of good hits).
     * The number of degrees of freedom for the fit will be calculated as:
     * multiplicity minus number of fit parameters.
     */
    unsigned int GetMultiplicity(){
        // this is actually the number of hit OMs, not the number of hits
        // but that is OK, I guess
        return dresponseptr_ ? dresponseptr_->size() : 0;
    }

    /* if you'd like, you could implement diagnostical output with this method
    virtual I3FrameObjectPtr GetDiagnostics( const I3EventHypothesis &fitresult ){
        return I3IntPtr(new I3Int(42));
    }
    */

    /// tell your name
    const std::string GetName() const {
        return name_;
    }

    SET_LOGGER( "I3GulliverIPDFPandel" );

};

#endif /* I3GULLIVER_IPDF_PANDEL_H_INCLUDED */
