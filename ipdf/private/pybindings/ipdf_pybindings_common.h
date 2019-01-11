#ifndef IPDF_COMMON_H_INCLUDED
#define IPDF_COMMON_H_INCLUDED

#include "boost/shared_ptr.hpp"
#include "icetray/OMKey.h"
#include "dataclasses/geometry/I3Geometry.h"
#include "dataclasses/physics/I3RecoPulse.h"
#include "ipdf/I3/I3HitOm.h"
#include "ipdf/I3/I3PEHit.h"
#include "ipdf/I3/I3DetectorResponse.h"
#include "ipdf/I3/I3DetectorConfiguration.h"
#include "ipdf/I3/I3OmReceiver.h"

namespace IPDF {
namespace detail {

    // common data cacher for I3PEP and I3DOMLikelihood
    class ipdf_data_cache {
        public:

            ipdf_data_cache(I3GeometryConstPtr g):
                detconf_(boost::make_shared<I3DetectorConfiguration>(*g)),
                omReceiver_(*(detconf_->begin())),
                hitom_(NULL){ }
            ~ipdf_data_cache(){
                delete hitom_;
            }

            void SetPulses(I3RecoPulseSeriesMapConstPtr rpsm){
                detresp_ = boost::make_shared<I3DetectorResponse>(rpsm,*detconf_);
            }
            bool hasData(){
                bool yesno = bool(detresp_);
                log_debug("check data %s", yesno?"OK":"NOT OK");
                return yesno;
            }

            const I3OmReceiver& getOmReceiver(const OMKey &dom) const {
                // TODO: check if this caching business is indeed faster than just always running find(dom).
                if ( dom!=lastDom_ ){
                    omReceiver_ = &(detconf_->find(dom));
                    lastDom_ = dom;
                }
                return *omReceiver_;
            }

            const I3HitOm* getHitOm(const OMKey& dom) const {
                const I3OmReceiver& omr = getOmReceiver(dom);
                I3DetectorResponse::const_iterator idr = detresp_->begin();
                I3DetectorResponse::const_iterator edr = detresp_->end();
                for (; idr != edr; ++idr){
                    const I3OmReceiver& omr2 = (*idr)->getOmReceiver();
                    if ( &omr2 == &omr){
                        return *idr;
                    }
                }
                log_debug("BEWARE");
                return NULL; // BEWARE!
            }

            I3HitOm* makeHitOm(const OMKey &dom, double t, double q) const {
                // TODO: replace this with something more efficient
                const I3OmReceiver& omr = getOmReceiver(dom);
                delete hitom_;
                hitom_ = new I3HitOm(omr, new I3PEHit(omr,t,q));
                return hitom_;
            }

            I3PEHit makePEHit(const OMKey &dom, double t, double q) const {
                const I3OmReceiver& omr = detconf_->find(dom);
                return I3PEHit(omr,t,q);
            }

        private:
            I3DetectorConfigurationPtr detconf_;
            I3DetectorResponsePtr detresp_;
            // Yes, a mutable const pointer! The pointer itself can change, the object it points to is const.
            mutable const I3OmReceiver* omReceiver_;
            mutable I3HitOm *hitom_;
            mutable OMKey lastDom_;
    };
}
}

#endif /* IPDF_COMMON_H_INCLUDED */
