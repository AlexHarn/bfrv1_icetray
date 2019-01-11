#include <vector>
#include <string>
#include <boost/python.hpp>
#include <boost/shared_ptr.hpp>

#include <icetray/I3Frame.h>
#include <dataclasses/geometry/I3Geometry.h>
#include <gulliver/I3EventHypothesis.h>
#include <gulliver/I3LogLikelihoodFit.h>
#include <gulliver/I3Gulliver.h>

namespace bp = boost::python;

static bp::object
GetLLH(I3GulliverBase &self, const std::vector<double> &par, bool gradients = false)
{
    if (!gradients) {
        return bp::object(self(par));
    } else {
        std::vector<double> grad(par.size(), 0.0);
        double llh = self(par, grad);
        return bp::make_tuple(llh, grad);
    }
}

static boost::shared_ptr<I3Gulliver>
make_gulliver(const std::string &name, I3EventLogLikelihoodBasePtr llh,
    I3ParametrizationBasePtr par, I3MinimizerBasePtr min)
{
    return boost::shared_ptr<I3Gulliver>(new I3Gulliver(par, llh, min, name));
}

static boost::shared_ptr<I3Gulliver>
make_gulliver_nomin(const std::string &name, I3EventLogLikelihoodBasePtr llh,
    I3ParametrizationBasePtr par)
{
    return make_gulliver(name, llh, par, I3MinimizerBasePtr());
}

static boost::shared_ptr<I3LogLikelihoodFitParams>
Fit( I3Gulliver& self, I3FramePtr frame, I3EventHypothesisPtr seed ){
    I3LogLikelihoodFitPtr storefit( new I3LogLikelihoodFit(*seed) );
    bool success = self.Fit(*frame,storefit);
    if ( success ^ (storefit->hypothesis_->particle->GetFitStatus() == I3Particle::OK)){
        log_fatal("PROGRAMMING ERROR: gulliver fit returns %s while particle fit status is %s",
                  (success ? "TRUE" : "FALSE"), storefit->hypothesis_->particle->GetFitStatusString().c_str() );
    }
    return storefit->fitparams_;
}

static boost::shared_ptr<I3LogLikelihoodFitParams>
ReFit( I3Gulliver& self, I3EventHypothesisPtr seed ){
    I3LogLikelihoodFitPtr storefit( new I3LogLikelihoodFit(*seed) );
    bool success = self.Fit(storefit);
    if ( success ^ (storefit->hypothesis_->particle->GetFitStatus() == I3Particle::OK)){
        log_fatal("PROGRAMMING ERROR: gulliver fit returns %s while particle fit status is %s",
                  (success ? "TRUE" : "FALSE"), storefit->hypothesis_->particle->GetFitStatusString().c_str() );
    }
    return storefit->fitparams_;
}

void register_I3Gulliver()
{
    bp::class_<I3GulliverBase, boost::noncopyable>("I3GulliverBase",
                "I3GulliverBase, the base class of I3Gulliver, is a functor object that does not 'know' anything about physics.",
                bp::no_init)
        .def("__call__", GetLLH)
    ;

    bp::class_<I3Gulliver, bp::bases<I3GulliverBase> >("I3Gulliver",
                "The I3Gulliver manages/coordinates loglikelihood fits. The python interface is slightly different from the C++ interface.",
                bp::no_init)
        .def("__init__", bp::make_constructor(make_gulliver))
        .def("__init__", bp::make_constructor(make_gulliver_nomin))
        .def("SetGeometry", &I3Gulliver::SetGeometry,
                    "Call this whenever a geometry frame passes by, or at least once before you try to do a fit.")
        .def("SetEvent", &I3Gulliver::SetEvent,
                    "This is useful for making likelihood maps, "
                    "it is not necessary to call this before a call to Fit(frame,hypothesis).")
        .def("UseGradients", &I3Gulliver::UseGradients,
                    "This method is useful for making likelihood maps, "
                    "it is not necessary to call this before a call to Fit(frame,hypothesis) "
                    "because then the Gulliver will call this method to make sure that the likelihood and "
                    "parametrization service can support gradients if the minimizer wants to use them. "
                    "Note that if this is not the case (i.e.: minimizer does want to use gradients but one of "
                    "the other services does not support that) then log_fatal() will be called.")
        .def("Fit", Fit,
                    "This takes a frame and an event hypothesis as input, does a fit and returns the fit params "
                    "(object of class I3LogLikelihoodFitParams). The input event hypothesis object gets overwritten with "
                    "the fit result, so if you would like to reuse the same seed for several fits, make sure to pass only "
                    "copies of the original seed.")
        .def("Fit", ReFit,
                    "This takes only an event hypothesis as input (you should have provided a P-frame previously with SetEvent or with "
                    "a Fit(frame,hypothesis) call on the same frame), does a fit and returns the fit params "
                    "(object of class I3LogLikelihoodFitParams). The input event hypothesis object gets overwritten with "
                    "the fit result, so if you would like to reuse the same seed for several fits, make sure to pass only "
                    "copies of the original seed.")
    ;
}
