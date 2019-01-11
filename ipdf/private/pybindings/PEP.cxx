#include "boost/shared_ptr.hpp"
#include "boost/make_shared.hpp"
#include "dataclasses/physics/I3RecoPulse.h"
#include "dataclasses/physics/I3Particle.h"
#include "ipdf/I3/I3PEHit.h"
#include "ipdf/I3/I3OmReceiver.h"
#include "ipdf/I3/I3DetectorConfiguration.h"
#include "ipdf/I3/I3DetectorResponse.h"
#include "ipdf/Pandel/GaussConvolutedPEP.h"
#include "ipdf/Pandel/IceModel.h"
#include "ipdf/PhotoSpline/PhotoSplinePEP.h"
#include "ipdf/Hypotheses/InfiniteMuon.h"
#include "ipdf/Hypotheses/DirectionalCascade.h"
#include "ipdf_pybindings_common.h"

using boost::make_shared;

namespace IPDF {

    /**
     * @class I3PEP
     *
     * Public interface for PEP objects defining the methods that can be used in python.
     * The public methods only use icetray datatypes. The non-public methods and the other
     * classes serve to estiblish the connection to the ipdf classes, hopefully in a flexible
     * enough way.
     *
     * You *can* supply a pulsemap, and then ask for the PDF/PEP value for the first hit of the hit DOMs, given an I3Particle.
     * You can also get PEP/PDF computed for your own favorite combination of DOM, hit time (or I3RecoPulse object) and I3Particle.
     *
     * TODO: compute PDFs for a numpy array of time values, output also as a numpy array.
     */
    class I3PEP {
        public:
            I3PEP(I3GeometryConstPtr g):cache_(make_shared<detail::ipdf_data_cache>(g)){}
            virtual ~I3PEP(){}
            void SetPulses(I3RecoPulseSeriesMapConstPtr rpsm){
                cache_->SetPulses(rpsm);
            }
            double getPdfFirstHit(I3ParticleConstPtr p, const OMKey& dom) const {
                if (!cache_->hasData()){
                    log_error("you should provide a recopulsemap (with set_event) before trying to compute a likelihood on any DOM.");
                    return NAN;
                }
                const I3HitOm* hitom = cache_->getHitOm(dom);
                if (hitom){
                    return this->_getPdf(p, (*hitom)[0]);
                }
                log_error("could not find the hit OM(%d,%d)",dom.GetString(),dom.GetOM());
                return NAN;
            }
            double getLogPdfFirstHit(I3ParticleConstPtr p, const OMKey& dom) const {
                if (!cache_->hasData()){
                    log_error("you should provide a recopulsemap (with set_event) before trying to compute a likelihood on any DOM.");
                    return NAN;
                }
                const I3HitOm* hitom = cache_->getHitOm(dom);
                if (hitom){
                    return this->_getLogPdf(p, (*hitom)[0]);
                }
                log_error("could not find the hit OM(%d,%d)",dom.GetString(),dom.GetOM());
                return NAN;
            }
            double getPdfAtTQ(I3ParticleConstPtr p, const OMKey& dom, double t, double q=1. ) const {
                return _getPdf(p,cache_->makePEHit(dom,t,q));
            }
            double getLogPdfAtTQ(I3ParticleConstPtr p, const OMKey& dom, double t, double q=1. ) const {
                return _getLogPdf(p,cache_->makePEHit(dom,t,q));
            }
            double getPdf(I3ParticleConstPtr p, const OMKey& dom, const I3RecoPulse &pulse) const {
                return this->_getPdf(p, cache_->makePEHit(dom, pulse.GetTime(), pulse.GetCharge()));
            }
            double getLogPdf(I3ParticleConstPtr p, const OMKey& dom, const I3RecoPulse &pulse) const {
                return this->_getLogPdf(p, cache_->makePEHit(dom, pulse.GetTime(), pulse.GetCharge()));
            }
            /// virtual function should be overridden
            virtual double getHitProb(I3ParticleConstPtr p, const OMKey& dom ) const {
                return NAN;
            }
            /// virtual function should be overridden
            virtual double expectedNPE(I3ParticleConstPtr p, const OMKey& dom ) const {
                return NAN;
            }
        protected:
            /// virtual function: should be overridden
            virtual double _getPdf(I3ParticleConstPtr p, const I3PEHit& pehit ) const {
                return NAN;
            }
            /// virtual function: should be overridden
            virtual double _getLogPdf(I3ParticleConstPtr p, const I3PEHit& pehit ) const {
                return NAN;
            }
            /// access to geometry and event data in an ipdf-friendly way
            mutable boost::shared_ptr<detail::ipdf_data_cache> cache_;
    };

    template<typename EmissionHypothesis, typename ipdfpep>
    class PEPimpl : public I3PEP {
        public:
            PEPimpl( I3GeometryConstPtr g, typename boost::shared_ptr<ipdfpep> ipdf_pep_ptr):
                I3PEP(g), pep_(ipdf_pep_ptr){}
            ~PEPimpl(){}
        protected:
            virtual double _getPdf(I3ParticleConstPtr p, const I3PEHit& pehit) const {
                return pep_->getPdf(pehit,EmissionHypothesis(p));
            }
            virtual double _getLogPdf(I3ParticleConstPtr p, const I3PEHit& pehit ) const {
                return pep_->getLogPdf(pehit,EmissionHypothesis(p));
            }
        public:
            double getHitProb(I3ParticleConstPtr p, const OMKey& dom ) const {
                return pep_->getHitProb(cache_->getOmReceiver(dom),EmissionHypothesis(p));
            }
            double expectedNPE(I3ParticleConstPtr p, const OMKey& dom ) const {
                return pep_->expectedNPE(cache_->getOmReceiver(dom),EmissionHypothesis(p));
            }
        private:
            typename boost::shared_ptr<ipdfpep> pep_;
    };

    typedef Pandel::GaussConvolutedPEP<Pandel::H2> gausspandel;
    class muon_pandel_pep: public PEPimpl<InfiniteMuon,gausspandel> {
        public:
            muon_pandel_pep(I3GeometryConstPtr g, double jitter):
                PEPimpl<InfiniteMuon,gausspandel>( g,make_shared<gausspandel>(make_shared<Pandel::H2>(),jitter) ){}
    };

    class cascade_pandel_pep: public PEPimpl<DirectionalCascade,gausspandel> {
        public:
            cascade_pandel_pep(I3GeometryConstPtr g, double jitter):
                PEPimpl<DirectionalCascade,gausspandel>( g,make_shared<gausspandel>(make_shared<Pandel::H2>(),jitter) ){}
    };

    class muon_photospline_pep: public PEPimpl<InfiniteMuon,PhotoSplinePEP> {
        public:
            muon_photospline_pep(I3GeometryConstPtr g, I3PhotonicsServicePtr pss):
                PEPimpl<InfiniteMuon,PhotoSplinePEP>( g, make_shared<PhotoSplinePEP>(pss,"PhotoSplinePEP") ){}
    };

    class cascade_photospline_pep: public PEPimpl<DirectionalCascade,PhotoSplinePEP> {
        public:
            cascade_photospline_pep(I3GeometryConstPtr g, I3PhotonicsServicePtr pss):
                PEPimpl<DirectionalCascade,PhotoSplinePEP>( g, make_shared<PhotoSplinePEP>(pss,"PhotoSplinePEP") ){}
    };

}

namespace bp=boost::python;

#define pandel_init_args bp::init<I3GeometryConstPtr,double>( (bp::arg("geometry"), bp::arg("jitter")=15.*I3Units::ns) )
#define photospline_init_args bp::init<I3GeometryConstPtr,I3PhotonicsServicePtr>( ( bp::arg("geometry"), bp::arg("photonics_service") ) )

#define PEP_CLASS_REG(eh,pep,ehpep,docline,initargs)                            \
    bp::class_<IPDF::PEPimpl<IPDF::eh,IPDF::pep>, boost::noncopyable >(         \
            "PEPimpl" # eh # pep,                                               \
            "auxiliary implementation class (do not use this directly)",        \
            bp::init<I3GeometryConstPtr,boost::shared_ptr< IPDF::pep > >(       \
                                (  bp::arg("geometry"),bp::arg(""#pep) ) ) );   \
    bp::class_<IPDF::ehpep,                                                     \
               bp::bases< IPDF::I3PEP, IPDF::PEPimpl<IPDF::eh,IPDF::pep> >,     \
               boost::shared_ptr< IPDF::ehpep >, boost::noncopyable >(""#ehpep, \
                                                                      docline,  \
                                                                      initargs) \
        .def("set_pulses", &IPDF::ehpep::SetPulses)                             \
        .def("get_pdf_first_hit", &IPDF::ehpep::getPdfFirstHit)                 \
        .def("get_log_pdf_first_hit", &IPDF::ehpep::getLogPdfFirstHit)          \
        .def("get_pdf_at_tq", &IPDF::ehpep::getPdfAtTQ)                         \
        .def("get_log_pdf_at_tq", &IPDF::ehpep::getLogPdfAtTQ)                  \
        .def("get_pdf", &IPDF::ehpep::getPdf)                                   \
        .def("get_log_pdf", &IPDF::ehpep::getLogPdf)                            \
        .def("get_hit_prob", &IPDF::ehpep::getHitProb)                          \
        .def("expected_npe", &IPDF::ehpep::expectedNPE) 

void register_I3PEP(){
    bp::class_<IPDF::I3PEP,boost::noncopyable>("I3PEP",
            "base class for pythonized ipdf photoelectron probabilities",
            bp::init<I3GeometryConstPtr>( ( bp::arg("geometry") ) ) );

    PEP_CLASS_REG(InfiniteMuon,gausspandel,muon_pandel_pep,"Gauss convoluted Pandel PDF for muons",pandel_init_args);
    PEP_CLASS_REG(InfiniteMuon,PhotoSplinePEP,muon_photospline_pep,"Photospline PDF for muons",photospline_init_args);
    PEP_CLASS_REG(DirectionalCascade,gausspandel,cascade_pandel_pep,"Gauss convoluted Pandel PDF for cascades",pandel_init_args);
    PEP_CLASS_REG(DirectionalCascade,PhotoSplinePEP,cascade_photospline_pep,"Photospline PDF for cascades",photospline_init_args);
}
