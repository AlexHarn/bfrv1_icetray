#ifndef IPDF_PhotoSplinePEP_H_INCLUDED
#define IPDF_PhotoSplinePEP_H_INCLUDED

/**
 *  copyright  (C) 2011
 *  the icecube collaboration
 *  $Id$
 *
 *  @version $Revision$
 *  @date $Date$
 *  @author David Boersma <boersma@icecube.wisc.edu>
 */

#include <iosfwd>
#include "photonics-service/I3PhotonicsService.h"
#include "ipdf/Hypotheses/InfiniteMuon.h"
#include "ipdf/Hypotheses/DirectionalCascade.h"
#include "ipdf/Hypotheses/PointCascade.h"
#include <string>

namespace IPDF {

    inline int GetPhotonicsType(const InfiniteMuon &m){return 0;}       // MUON is #defined in "photoamasim.h" as 0, but in rdmc.h there is a conflicting #define
    inline int GetPhotonicsType(const DirectionalCascade &d){return 1;} // EMS is defined as 1 in "photoamasim.h"
    inline int GetPhotonicsType(const PointCascade &p){return 1;}       // EMS is defined as 1 in "photoamasim.h"

    /**
     * Dummy icemodel type
     * Needed for the event geometry, as defined by each event hypothesis.
     * For PhotoSplinePEP this is then actually only used in a log message, 
     * not in actual calculations.
     */
    class DummyIceModelType {
        public:
            DummyIceModelType(){}
            ~DummyIceModelType(){}
            static const double TD_DIST_P1;
            static const double TD_DIST_P0_CS0;
            static const double TD_DIST_P0_CS1;
            static const double TD_DIST_P0_CS2;
    };

    /**
     * @brief PhotoSpline implementation of PEP concept
     */
    class PhotoSplinePEP {
        public:
            typedef PhotoSplinePEP Self;
            typedef DummyIceModelType IceModelType;
            typedef double pdf_result;

            explicit PhotoSplinePEP(I3PhotonicsServicePtr pss,std::string name) :
                    pss_(pss), name_(name) {
                log_warn("PhotoSpline for ipdf is very new and highly "
                         "experimental, please do not expect ANY reasonable "
                         "reasonable results yet. It compiles, that's all.");
            }
            ~PhotoSplinePEP();

            /// Methods to access the PDF:
            template<typename PEHit, typename EmissionHypothesis>
            pdf_result
            getPdf(const PEHit& pehit, const EmissionHypothesis& eh) const;

            template<typename PEHit, typename EmissionHypothesis>
            pdf_result
            getLogPdf(const PEHit& pehit, const EmissionHypothesis& eh) const;

            template<typename PEHit, typename EmissionHypothesis>
            pdf_result
            getIntPdf(const PEHit& pehit, const EmissionHypothesis& eh) const;

            /// Probability of at least one photo-electron hit on an OM
            template<typename OmReceiver, typename EmissionHypothesis>
            pdf_result
            getHitProb(const OmReceiver& omr, const EmissionHypothesis& eh) const;

            /// Mean number of expected photo-electron hits on an OM
            template<typename OmReceiver, typename EmissionHypothesis>
            pdf_result
            expectedNPE(const OmReceiver& omr, const EmissionHypothesis& eh) const;

            friend std::ostream& operator<<(std::ostream&, const Self&);

        private:
            I3PhotonicsServicePtr pss_;
            std::string name_;
            SET_LOGGER("PhotoSplinePEP");

    };

    inline std::ostream& operator<<(std::ostream& os,const PhotoSplinePEP& psi){
        return (os<<"PhotoSplinePEP: "  // print state
                  <<"(using photonics service \"" << psi.name_ << "\")");
    }

    inline PhotoSplinePEP::~PhotoSplinePEP(){}

    /********************************************************************/
    template<typename OmReceiver, typename EmissionHypothesis>
    inline double PhotoSplinePEP::expectedNPE(
            const OmReceiver& omr, const EmissionHypothesis& eh) const {

        pss_->SelectModuleCoordinates(omr.getX(),omr.getY(),omr.getZ());

        double meanPEs, emissionPointDistance,geoTime;

        // for infinite muons, photonics expects a length of 0
        PhotonicsSource source(
                eh.getPointX(),eh.getPointY(),eh.getPointZ(),
                eh.getZenith(),eh.getAzimuth(),1., 0 /* length */,
                eh.getEnergy(),IPDF::GetPhotonicsType(eh) );

        pss_->SelectSource( meanPEs, emissionPointDistance, geoTime, source);
        // slightly wrong: needs correction factor
        // (quantile of probdensity within event time window)
        return meanPEs;
    }

    template<typename OmReceiver, typename EmissionHypothesis>
    inline double PhotoSplinePEP::getHitProb(
            const OmReceiver& omr, const EmissionHypothesis& eh) const {

        double meanPEs = this->expectedNPE(omr,eh);
        double hitprob=0.;
        if (meanPEs>0.){
            hitprob = 1. - exp(-meanPEs);
        }
        return hitprob;
    }

    /********************************************************************/
    template<typename PEHit, typename EmissionHypothesis>
    inline double PhotoSplinePEP::getLogPdf(
            const PEHit& pehit,
            const EmissionHypothesis& emitter) const {
        double pdf = this->getPdf(pehit,emitter);
        double logpdf = NAN;
        if (pdf>0.){
            logpdf = log(pdf);
        }
        return logpdf;
    }

    /********************************************************************/
    template<typename PEHit, typename EmissionHypothesis>
    inline double PhotoSplinePEP::getPdf(
            const PEHit& pehit,
            const EmissionHypothesis& emitter) const {
        pss_->SelectModuleCoordinates(
                pehit.getOmReceiver().getX(),
                pehit.getOmReceiver().getY(),
                pehit.getOmReceiver().getZ()
        );
        double meanPEs, emissionPointDistance, geoTime;
        // for infinite muons, photonics expects a length of 0
        PhotonicsSource source(
                emitter.getPointX()/I3Units::m,emitter.getPointY()/I3Units::m,emitter.getPointZ()/I3Units::m,
                emitter.getZenith()/I3Units::degree,emitter.getAzimuth()/I3Units::degree,1., 0 /* length */,
                emitter.getEnergy()/I3Units::GeV,IPDF::GetPhotonicsType(emitter) );
       
        // PhotonicsSource(double x, double y, double z, double zenith,
        // double azimuth, double speed, double length, double Energy,
        // int type);

        // setting the getAmp flag to "false" to avoid extraneous spline lookups
        // SelectSource() will return a meanPEs of 0
        pss_->SelectSource( meanPEs, NULL, emissionPointDistance,
                geoTime, source, true);
        //log_warn("meanPE: %e",meanPEs);
        if(meanPEs>=0){

          double pdf = 0.;
          double tdelay = pehit.getLeTime() -emitter.getTZero()- geoTime;

          pss_->GetProbabilityDensity( pdf, tdelay );
           if (pdf>0.){
            // log_trace("SUCCESS got pdf=%g", pdf);
            return pdf;
          }

        }
        /*
        log_warn("FAILED to get pdf with omxyz=(%f,%f,%f) "
                          " and particle xyz=(%f,%f,%f)",
            pehit.getOmReceiver().getX(),
            pehit.getOmReceiver().getY(),
            pehit.getOmReceiver().getZ(),
            emitter.getPointX(),emitter.getPointY(),emitter.getPointZ()
            );
        */

        return 0.;
    }

    /********************************************************************/
    template<typename PEHit, typename EmissionHypothesis>
    inline double PhotoSplinePEP::getIntPdf(
            const PEHit& pehit,
            const EmissionHypothesis& emitter) const {

        pss_->SelectModuleCoordinates(
            pehit.getOmReceiver().getX(),
            pehit.getOmReceiver().getY(),
            pehit.getOmReceiver().getZ()
        );

        double meanPEs, emissionPointDistance,geoTime;
        // for infinite muons, photonics expects a length of 0
        PhotonicsSource source(
                emitter.getPointX()/I3Units::m,emitter.getPointY()/I3Units::m,emitter.getPointZ()/I3Units::m,
                emitter.getZenith()/I3Units::degree,emitter.getAzimuth()/I3Units::degree,1., 0 /* length */,
                emitter.getEnergy()/I3Units::GeV,IPDF::GetPhotonicsType(emitter) );
        // setting the getAmp flag to "false" to avoid extraneous spline lookups
        // SelectSource() will return a meanPEs of 0
        pss_->SelectSource( meanPEs, NULL, emissionPointDistance,
                geoTime, source, true);
        double intpdf = 0.;
        double cdf_array[1] = {0.};
        double tdelay  = pehit.getLeTime() -emitter.getTZero()- geoTime;

        // setting bounds for the "quantile" to look up
        // the lower bound should be a big negative number to
        // make sure we get a lower probability bound of 0
        double time_edges[2] = {-10000.,tdelay};

        if(meanPEs>=0){
                // GetProbabilityQuantiles() should now work
                bool success = pss_->GetProbabilityQuantiles(time_edges, 0, cdf_array,
                        NULL, 1);
            if (success) {
                intpdf = cdf_array[0];
                // log_trace("SUCCESS got cdf=%g", intpdf);
  	  } else {
                /*
                    log_warn( "FAILED to get cdf with omxyz=(%f,%f,%f) "
                              " and particle xyz=(%f,%f,%f)",
                              pehit.getOmReceiver().getX(),
                              pehit.getOmReceiver().getY(),
                              pehit.getOmReceiver().getZ(),
                              emitter.getPointX(),emitter.getPointY(),emitter.getPointZ()
                            );
                */
            }
  
            if ( (intpdf>=0.) && (intpdf<=1.) ){
                return intpdf;
            } else if (intpdf<0.){
                return 0.;
            } else if (intpdf>=1.){
                return 1.;
            }
        }
        // intpdf is NAN
        return 0.;
    }

    /********************************************************************/

} // namespace IPDF

#endif // IPDF_PhotoSplinePEP_H_INCLUDED
