#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/preprocessor.hpp>
#include "dataclasses/physics/I3RecoPulse.h"
#include "dataclasses/physics/I3Particle.h"
#include "ipdf/I3/I3PEHit.h"
#include "ipdf/I3/I3OmReceiver.h"
#include "ipdf/I3/I3DetectorConfiguration.h"
#include "ipdf/I3/I3DetectorResponse.h"
#include "ipdf/Pandel/GaussConvolutedPEP.h"
#include "ipdf/Pandel/IceModel.h"
#include "ipdf/PhotoSpline/PhotoSplinePEP.h"
#include "ipdf/Likelihood/SPE1st.h"
#include "ipdf/Likelihood/SPEAll.h"
#include "ipdf/Likelihood/MPE.h"
#include "ipdf/Hypotheses/InfiniteMuon.h"
#include "ipdf_pybindings_common.h"

using boost::make_shared;

namespace IPDF {

    class I3DOMLikelihood {
        public:
            I3DOMLikelihood(I3GeometryConstPtr g):cache_(make_shared<detail::ipdf_data_cache>(g)){}
            virtual ~I3DOMLikelihood(){}
            void SetPulses(I3RecoPulseSeriesMapConstPtr rpsm){
                cache_->SetPulses(rpsm);
            }
            double getLikelihood(I3ParticleConstPtr p, const OMKey& dom){
                if (!cache_->hasData()){
                    log_error("you should provide a recopulsemap (with set_event) before trying to compute a likelihood on any DOM.");
                    return NAN;
                }
                return this->_getLikelihood(p, cache_->getHitOm(dom));
            }
            double getLogLikelihood(I3ParticleConstPtr p, const OMKey& dom){
                if (!cache_->hasData()){
                    log_error("you should provide a recopulsemap (with set_event) before trying to compute a likelihood on any DOM.");
                    return NAN;
                }
                return this->_getLogLikelihood(p, cache_->getHitOm(dom));
            }
            double getLikelihoodAtTQ(I3ParticleConstPtr p, const OMKey& dom, double t, double q=1.){
                return this->_getLikelihood(p, cache_->makeHitOm(dom,t,q));
            }
            double getLogLikelihoodAtTQ(I3ParticleConstPtr p, const OMKey& dom, double t, double q=1.){
                return this->_getLogLikelihood(p, cache_->makeHitOm(dom,t,q));
            }
        protected:
            virtual double _getLikelihood(I3ParticleConstPtr p, const I3HitOm* hitom) const {
                // maybe raise some exception here; this class cannot be abstract, otherwise it won't work with boost::python
                log_error("using unoverridden virtual function?");
                return NAN;
            }
            virtual double _getLogLikelihood(I3ParticleConstPtr p, const I3HitOm* hitom ) const {
                // maybe raise some exception here; this class cannot be abstract, otherwise it won't work with boost::python
                log_error("using unoverridden virtual function?");
                return NAN; // or minus infinitiy
            }
            mutable boost::shared_ptr<detail::ipdf_data_cache> cache_;
    };

    template<typename EmissionHypothesis, typename ipdfLH>
    class DOMLikelihoodImpl : public I3DOMLikelihood {
        public:
            DOMLikelihoodImpl( I3GeometryConstPtr g, typename boost::shared_ptr<ipdfLH> ipdf_lh_ptr):
                I3DOMLikelihood(g), lh_(ipdf_lh_ptr){}
            ~DOMLikelihoodImpl(){}
        protected:
            virtual double _getLikelihood(I3ParticleConstPtr p, const I3HitOm* hitom) const {
                EmissionHypothesis eh(p);
                return lh_->getLikelihood(*hitom,eh);
            }
            virtual double _getLogLikelihood(I3ParticleConstPtr p, const I3HitOm* hitom ) const {
                EmissionHypothesis eh(p);
                return lh_->getLogLikelihood(*hitom,eh);
            }
        private:
            typename boost::shared_ptr<ipdfLH> lh_;
    };

    typedef Pandel::GaussConvolutedPEP<Pandel::H2> gausspandel;
    typedef Likelihood::SPE1st< gausspandel > pandel_spe1st;
    typedef Likelihood::SPEAll< gausspandel > pandel_speall;
    typedef Likelihood::MPE< gausspandel > pandel_mpe;
    typedef Likelihood::SPE1st< PhotoSplinePEP > photospline_spe1st;
    typedef Likelihood::SPEAll< PhotoSplinePEP > photospline_speall;
    typedef Likelihood::MPE< PhotoSplinePEP > photospline_mpe;

#define SEQ_ALL_PANDEL_LH (pandel_spe1st)(pandel_speall)(pandel_mpe)
#define SEQ_ALL_SPLINE_LH (photospline_spe1st)(photospline_speall)(photospline_mpe)
#define PANDEL_CTR_ARG double jitter
#define PANDEL_CTR_ARGVAL make_shared< gausspandel >( make_shared<Pandel::H2>(),jitter)
#define SPLINE_CTR_ARG I3PhotonicsServicePtr pss
#define SPLINE_CTR_ARGVAL make_shared< PhotoSplinePEP >( pss, "PhotoSplinePEP")
#define PY_DOM_LH_CLASS(r,data,domlh) \
    class BOOST_PP_CAT( BOOST_PP_SEQ_ELEM(0,data), domlh ) : public DOMLikelihoodImpl< BOOST_PP_SEQ_ELEM( 1 , data ) , domlh > { \
        public: \
            BOOST_PP_CAT( BOOST_PP_SEQ_ELEM( 0 , data ) , domlh )(I3GeometryConstPtr g, BOOST_PP_SEQ_ELEM( 2 , data ) ): \
                DOMLikelihoodImpl< BOOST_PP_SEQ_ELEM( 1 , data ) , domlh >( g, make_shared<domlh>( BOOST_PP_SEQ_ELEM( 3 , data ) ) ){} \
    };
BOOST_PP_SEQ_FOR_EACH(PY_DOM_LH_CLASS,
                      (muon_)(InfiniteMuon)(PANDEL_CTR_ARG)(PANDEL_CTR_ARGVAL),
                      SEQ_ALL_PANDEL_LH)
BOOST_PP_SEQ_FOR_EACH(PY_DOM_LH_CLASS,
                      (cascade_)(DirectionalCascade)(PANDEL_CTR_ARG)(PANDEL_CTR_ARGVAL),
                      SEQ_ALL_PANDEL_LH)
BOOST_PP_SEQ_FOR_EACH(PY_DOM_LH_CLASS,
                      (muon_)(InfiniteMuon)(SPLINE_CTR_ARG)(SPLINE_CTR_ARGVAL),
                      SEQ_ALL_SPLINE_LH)
BOOST_PP_SEQ_FOR_EACH(PY_DOM_LH_CLASS,
                      (cascade_)(DirectionalCascade)(SPLINE_CTR_ARG)(SPLINE_CTR_ARGVAL),
                      SEQ_ALL_SPLINE_LH)

/*
e.g.:
    class muon_pandel_spe1st: public DOMLikelihoodImpl<InfiniteMuon,pandel_spe1st> {
        public:
            muon_pandel_spe1st(I3GeometryConstPtr g, double jitter):
                DOMLikelihoodImpl( g, make_shared<pandel_spe1st>( make_shared< gausspandel >( make_shared<Pandel::H2>(),jitter) ) ){}
    };
*/

}

namespace bp=boost::python;

#define pandel_init_args bp::init<I3GeometryConstPtr,double>( (bp::arg("geometry"), bp::arg("jitter")=15.*I3Units::ns) )
#define photospline_init_args bp::init<I3GeometryConstPtr,I3PhotonicsServicePtr>( ( bp::arg("geometry"), bp::arg("photonics_service") ) )

#define DOMLIKELIHOOD_CLASS_REG(eh,domlh,pydomlh,docline,initargs)                            \
    bp::class_<IPDF::DOMLikelihoodImpl<IPDF::eh,IPDF::domlh>, boost::noncopyable >( \
            "DOMLikelihoodImpl" # eh # domlh, \
            "auxiliary implementation class (do not use directly)", \
            bp::init<I3GeometryConstPtr,boost::shared_ptr< IPDF::domlh > >(       \
                                (  bp::arg("geometry"),bp::arg(""#domlh) ) ) );   \
    bp::class_<IPDF::pydomlh, \
               bp::bases< IPDF::I3DOMLikelihood, IPDF::DOMLikelihoodImpl<IPDF::eh,IPDF::domlh> >, \
               boost::shared_ptr<IPDF::pydomlh>, boost::noncopyable >(""#pydomlh, \
                                                        docline, \
                                                        initargs) \
        .def("set_pulses", &IPDF::pydomlh::SetPulses) \
        .def("get_likelihood", &IPDF::pydomlh::getLikelihood) \
        .def("get_log_likelihood", &IPDF::pydomlh::getLogLikelihood) \
        .def("get_likelihood_at_tq", &IPDF::pydomlh::getLikelihoodAtTQ) \
        .def("get_log_likelihood_at_tq", &IPDF::pydomlh::getLogLikelihoodAtTQ)

void register_I3DOMLikelihood(){
    bp::class_<IPDF::I3DOMLikelihood,boost::noncopyable>("I3DOMLikelihood",
            "python base class for ipdf photoelectron probabilities",
            bp::init<I3GeometryConstPtr>( ( bp::arg("geometry") ) ) );

    DOMLIKELIHOOD_CLASS_REG(InfiniteMuon,pandel_spe1st,muon_pandel_spe1st,"SPE1st for muons, using Gauss-convoluted Pandel.",pandel_init_args);
    DOMLIKELIHOOD_CLASS_REG(DirectionalCascade,pandel_spe1st,cascade_pandel_spe1st,"SPE1st for cascades, using Gauss-convoluted Pandel.",pandel_init_args);
    DOMLIKELIHOOD_CLASS_REG(InfiniteMuon,pandel_speall,muon_pandel_speall,"SPEAll for muons, using Gauss-convoluted Pandel.",pandel_init_args);
    DOMLIKELIHOOD_CLASS_REG(DirectionalCascade,pandel_speall,cascade_pandel_speall,"SPEAll for cascades, using Gauss-convoluted Pandel.",pandel_init_args);
    DOMLIKELIHOOD_CLASS_REG(InfiniteMuon,pandel_mpe,muon_pandel_mpe,"MPE for muons, using Gauss-convoluted Pandel.",pandel_init_args);
    DOMLIKELIHOOD_CLASS_REG(DirectionalCascade,pandel_mpe,cascade_pandel_mpe,"MPE for cascades, using Gauss-convoluted Pandel.",pandel_init_args);

    DOMLIKELIHOOD_CLASS_REG(InfiniteMuon,photospline_spe1st,muon_photospline_spe1st,"SPE1st for muons, using photospline.",photospline_init_args);
    DOMLIKELIHOOD_CLASS_REG(DirectionalCascade,photospline_spe1st,cascade_photospline_spe1st,"SPE1st for cascades, using photospline.",photospline_init_args);
    DOMLIKELIHOOD_CLASS_REG(InfiniteMuon,photospline_speall,muon_photospline_speall,"SPEAll for muons, using photospline.",photospline_init_args);
    DOMLIKELIHOOD_CLASS_REG(DirectionalCascade,photospline_speall,cascade_photospline_speall,"SPEAll for cascades, using photospline.",photospline_init_args);
    DOMLIKELIHOOD_CLASS_REG(InfiniteMuon,photospline_mpe,muon_photospline_mpe,"MPE for muons, using photospline.",photospline_init_args);
    DOMLIKELIHOOD_CLASS_REG(DirectionalCascade,photospline_mpe,cascade_photospline_mpe,"MPE for cascades, using photospline.",photospline_init_args);

    /*
    bp::class_<IPDF::DOMLikelihoodImpl<IPDF::InfiniteMuon,IPDF::pandelSPE1st>, boost::noncopyable >("DOMLikelihoodImplInfiniteMuonPandelSPE1st",
                                                                     "auxiliary implementation class (do not use directly)",
                                                                     bp::init<I3GeometryConstPtr,IPDF::pandelSPE1stptr>(
                                                                         (bp::arg("geometry"),bp::arg("pandelSPE1st") ) ) );
    bp::class_<IPDF::muon_pandel_SPE1st,
               bp::bases< IPDF::I3DOMLikelihood, IPDF::DOMLikelihoodImpl<IPDF::InfiniteMuon,IPDF::pandelSPE1st> >,
               IPDF::muon_pandel_SPE1stptr, boost::noncopyable >("muon_pandel_SPE1st",
                                                        "SPE1st for muons using a Gauss convoluted Pandel PDF",
                                                        bp::init<I3GeometryConstPtr,double>(
                                                            (bp::arg("geometry"),
                                                             bp::arg("jitter")=15.*I3Units::ns)
                                                            )
                                                        )
        .def("set_pulses", &IPDF::muon_pandel_SPE1st::SetPulses)
        .def("get_likelihood", &IPDF::muon_pandel_SPE1st::getLikelihood)
        .def("get_log_likelihood", &IPDF::muon_pandel_SPE1st::getLogLikelihood)
        .def("get_likelihood_at_tq", &IPDF::muon_pandel_SPE1st::getLikelihoodAtTQ)
        .def("get_log_likelihood_at_tq", &IPDF::muon_pandel_SPE1st::getLogLikelihoodAtTQ)
        ;
    */
}
