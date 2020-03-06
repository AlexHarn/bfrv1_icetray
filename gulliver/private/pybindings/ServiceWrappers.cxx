#include <dataclasses/geometry/I3Geometry.h>
#include <icetray/I3Frame.h>
#include <gulliver/I3EventHypothesis.h>

#include <gulliver/I3EventLogLikelihoodBase.h>
#include <gulliver/I3MinimizerBase.h>
#include <gulliver/I3ParametrizationBase.h>
#include <gulliver/I3SeedServiceBase.h>

#include <icetray/python/context_suite.hpp>
#define WRAP_CONTEXT(base) .def(icetray::python::context_suite<base>())

namespace bp = boost::python;

double (I3EventLogLikelihoodBase::*GetLLHSimple)(const I3EventHypothesis &)
    = &I3EventLogLikelihoodBase::GetLogLikelihood;

static bp::object
GetLLHGradient(I3EventLogLikelihoodBase &self, const I3EventHypothesis &hypo, bool maximize, bool with_gradient)
{
    I3EventHypothesisPtr gradient;
    if (with_gradient)
        gradient = I3EventHypothesisPtr(new I3EventHypothesis);

    double llh = self.GetLogLikelihood(hypo, gradient.get(), maximize);
    return bp::make_tuple(llh, gradient);
}

static std::vector<double>
GetGradient(I3ParametrizationBase &self)
{
    std::vector<double> grad;
    self.GetGradient(grad);

    return grad;
}

I3EventHypothesisPtr (I3ParametrizationBase::*GetHypothesisPtr)()
    = &I3ParametrizationBase::GetHypothesisPtr;

I3EventHypothesisConstPtr (I3ParametrizationBase::*CalcHypothesis)(const vector<double> &)
    = &I3ParametrizationBase::GetHypothesisPtr;

struct I3SeedServiceWrapper : public I3SeedServiceBase, public bp::wrapper<I3SeedServiceBase> {
    virtual unsigned int SetEvent(const I3Frame &f)
    {
        return this->get_override("SetEvent")(f);
    }
    virtual I3EventHypothesis GetSeed(unsigned int iseed) const
    {
        return this->get_override("GetSeed")(iseed);
    }
    virtual I3EventHypothesis GetDummy() const
    {
        return this->get_override("GetDummy")();
    }
    virtual I3EventHypothesis GetCopy(const I3EventHypothesis &eh) const
    {
        return this->get_override("GetCopy")(eh);
    }
    virtual void Tweak(I3EventHypothesis &eh) const
    {
        this->get_override("Tweak")(eh);
    }
    virtual void FillInTheBlanks(I3EventHypothesis &eh) const
    {
        this->get_override("FillInTheBlanks")(eh);
    }
};

struct I3ParametrizationWrapper : public I3ParametrizationBase, public bp::wrapper<I3ParametrizationBase> {
    I3ParametrizationWrapper(const std::string &name) :
        I3ParametrizationBase(I3EventHypothesisPtr(new I3EventHypothesis)), name_(name) {};
    virtual void UpdatePhysicsVariables()
    {
        this->get_override("UpdatePhysicsVariables")();
    }
    virtual void UpdateParameters()
    {
        this->get_override("UpdateParameters")();
    }
    virtual void ApplyChainRule()
    {
        if (bp::override f = this->get_override("ApplyChainRule"))
            f();
    }
    virtual bool InitChainRule(bool wantgradient)
    {
        if (bp::override f = this->get_override("InitChainRule"))
            return f();
        else
            return false;
    }
    virtual void SetEvent(const I3Frame& frame)
    {
        if (bp::override f = this->get_override("SetEvent"))
            f(frame);
    }
    virtual const std::string GetName() const
    {
        return name_;
    }

    std::vector<double>& GetPar() { return par_; }
    std::vector<double>& GetParGradient() { return par_gradient_; }
    std::vector<I3FitParameterInitSpecs>& GetParSpecs() { return parspecs_; }
        I3EventHypothesisPtr GetHypothesis() { return hypothesis_; }
        void SetHypothesis(I3EventHypothesisPtr h) { hypothesis_ = h; }
        I3EventHypothesisPtr GetGradient() { return gradient_; }
    void SetGradient(I3EventHypothesisPtr h) { gradient_ = h; }

    std::string name_;

};


struct I3EventLogLikelihoodWrapper : public I3EventLogLikelihoodBase, public bp::wrapper<I3EventLogLikelihoodBase> {
    virtual void SetGeometry(const I3Geometry &g)
    {
        if (bp::override f = get_override("SetGeometry"))
            f(g);
    }
    virtual void SetEvent(const I3Frame &fr)
    {
        if (bp::override f = get_override("SetEvent"))
            f(fr);
        else {
            PyErr_SetString(PyExc_NotImplementedError, "Python subclasses of I3EventLogLikelihood must implement SetEvent()");
            throw bp::error_already_set();
        }
    }
    virtual double GetLogLikelihood(const I3EventHypothesis &ehypo)
    {
        if (bp::override f = get_override("GetLogLikelihood"))
            return f(ehypo);
        else {
            PyErr_SetString(PyExc_NotImplementedError, "Python subclasses of I3EventLogLikelihood must implement GetLogLikelihood()");
            throw bp::error_already_set();
            return 0.;
        }
    }
    virtual double GetLogLikelihoodWithGradient(const I3EventHypothesis &ehypo,
        I3EventHypothesis &gradient, double weight)
    {
        if (bp::override f = get_override("GetLogLikelihoodWithGradient"))
            return f(ehypo, gradient, weight);
        else
            return I3EventLogLikelihoodBase::GetLogLikelihoodWithGradient(ehypo, gradient, weight);
    }
    virtual bool HasGradient()
    {
        if (bp::override f = get_override("HasGradient"))
            return f();
        else
            return I3EventLogLikelihoodBase::HasGradient();
    }
    virtual unsigned int GetMultiplicity()
    {
        if (bp::override f = get_override("GetMultiplicity"))
            return f();
        else {
            PyErr_SetString(PyExc_NotImplementedError, "Python subclasses of I3EventLogLikelihood must implement GetMultiplicity()");
            throw bp::error_already_set();
            return 0;
        }
    }
    virtual I3FrameObjectPtr GetDiagnostics( const I3EventHypothesis &fitresult )
    {
        if (bp::override f = get_override("GetDiagnostics"))
            return f(fitresult);
        else
            return I3EventLogLikelihoodBase::GetDiagnostics(fitresult);
    }
    virtual const std::string GetName() const
    {
        if (bp::override f = get_override("GetName"))
            return f();
        else {
            PyErr_SetString(PyExc_NotImplementedError, "Python subclasses of I3EventLogLikelihood must implement GetName()");
            throw bp::error_already_set();
            return std::string();
        }
    }
};

struct I3MinimizerWrapper : public I3MinimizerBase, public bp::wrapper<I3MinimizerBase> {

    double GetTolerance() const { return tolerance_; }
    void SetTolerance(double newtol ){ tolerance_ = newtol; }
    unsigned int GetMaxIterations() const { return maxIterations_; }
    void SetMaxIterations(unsigned int newmaxi ){ maxIterations_ = newmaxi; }

    virtual const std::string GetName() const
    {
        if (bp::override f = get_override("GetName"))
            return f();
        else {
            PyErr_SetString(PyExc_NotImplementedError, "Python subclasses of I3MinimizerWrapper must implement GetName()");
            throw bp::error_already_set();
            return std::string();
        }
    }

    virtual bool UsesGradient()
    {
        if (bp::override f = get_override("UsesGradient"))
            return f();
        else
            return I3MinimizerWrapper::UsesGradient();
    }

    virtual I3MinimizerResult Minimize(I3GulliverBase &g, const std::vector<I3FitParameterInitSpecs> &parspecs )
    {
        if (bp::override f = get_override("Minimize"))
            return f(boost::ref(g), parspecs);
        else {
            PyErr_SetString(PyExc_NotImplementedError, "Python subclasses of I3MinimizerWrapper must implement Minimize()");
            throw bp::error_already_set();
            return I3MinimizerResult(0);
        }
    }

    double tolerance_;
    unsigned int maxIterations_;
};

void register_ServiceWrappers()
{
    bp::class_<I3EventLogLikelihoodBase, boost::shared_ptr<I3EventLogLikelihoodBase>,
        boost::noncopyable>("I3EventLogLikelihoodBase", bp::no_init)
        .def("SetEvent", &I3EventLogLikelihoodBase::SetEvent)
        .def("SetGeometry", &I3EventLogLikelihoodBase::SetGeometry)
        .def("GetName", &I3EventLogLikelihoodBase::GetName)
        .def("GetMultiplicity", &I3EventLogLikelihoodBase::GetMultiplicity)
        .def("GetDiagnostics", &I3EventLogLikelihoodBase::GetDiagnostics)
        .def("GetLogLikelihood", GetLLHSimple, (bp::args("self"), bp::arg("hypothesis")))
        .def("GetLogLikelihood", GetLLHGradient, (bp::args("self"), bp::arg("hypothesis"),
            bp::arg("maximize_extra_dimensions"), bp::arg("with_gradient")=false))
        .def("GetLogLikelihoodWithGradient", &I3EventLogLikelihoodBase::GetLogLikelihoodWithGradient,
            bp::arg("weight")=1)
        .def("HasGradient", &I3EventLogLikelihoodBase::HasGradient)
        WRAP_CONTEXT(I3EventLogLikelihoodBase)
    ;

    bp::class_<I3EventLogLikelihoodWrapper, boost::shared_ptr<I3EventLogLikelihoodWrapper>,
        boost::noncopyable>("I3EventLogLikelihood")
        .def("SetEvent", &I3EventLogLikelihoodBase::SetEvent)
        .def("SetGeometry", &I3EventLogLikelihoodBase::SetGeometry)
        .def("GetName", &I3EventLogLikelihoodBase::GetName)
        .def("GetMultiplicity", &I3EventLogLikelihoodBase::GetMultiplicity)
        .def("GetDiagnostics", &I3EventLogLikelihoodBase::GetDiagnostics)
        .def("GetLogLikelihood", GetLLHSimple, (bp::args("self"), bp::arg("hypothesis")))
        .def("GetLogLikelihood", GetLLHGradient, (bp::args("self"), bp::arg("hypothesis"),
            bp::arg("maximize_extra_dimensions"), bp::arg("with_gradient")=false))
        .def("GetLogLikelihoodWithGradient", &I3EventLogLikelihoodBase::GetLogLikelihoodWithGradient,
            bp::arg("weight")=1)
        .def("HasGradient", &I3EventLogLikelihoodBase::HasGradient)
        WRAP_CONTEXT(I3EventLogLikelihoodBase)
    ;

    bp::class_<I3MinimizerBase, boost::shared_ptr<I3MinimizerBase>,
        boost::noncopyable>("I3MinimizerBase", bp::no_init)
        WRAP_CONTEXT(I3MinimizerBase)
    ;

    bp::class_<I3MinimizerWrapper, boost::shared_ptr<I3MinimizerWrapper>,
        boost::noncopyable>("I3Minimizer")
        WRAP_CONTEXT(I3MinimizerBase)
    #define PROPS (Tolerance)(MaxIterations)
        BOOST_PP_SEQ_FOR_EACH(WRAP_PROP, I3MinimizerWrapper, PROPS)
    #undef PROPS
    ;

    bp::class_<I3SeedServiceBase, boost::shared_ptr<I3SeedServiceBase>,
        boost::noncopyable>("I3SeedServiceBase", bp::no_init)
        .def("SetEvent", &I3SeedServiceBase::SetEvent)
        .def("GetSeed", &I3SeedServiceBase::GetSeed)
        .def("GetDummy", &I3SeedServiceBase::GetDummy)
        .def("GetCopy", &I3SeedServiceBase::GetCopy)
        .def("Tweak", &I3SeedServiceBase::Tweak)
        .def("FillInTheBlanks", &I3SeedServiceBase::FillInTheBlanks)
        WRAP_CONTEXT(I3SeedServiceBase)
    ;

    bp::class_<I3SeedServiceWrapper, boost::shared_ptr<I3SeedServiceWrapper>,
        boost::noncopyable>("I3SeedService")
        .def("SetEvent", &I3SeedServiceBase::SetEvent)
        .def("GetSeed", &I3SeedServiceBase::GetSeed)
        .def("GetDummy", &I3SeedServiceBase::GetDummy)
        .def("GetCopy", &I3SeedServiceBase::GetCopy)
        .def("Tweak", &I3SeedServiceBase::Tweak)
        .def("FillInTheBlanks", &I3SeedServiceBase::FillInTheBlanks)
        WRAP_CONTEXT(I3SeedServiceBase)
    ;

    bp::class_<I3ParametrizationBase, boost::shared_ptr<I3ParametrizationBase>,
        boost::noncopyable>("I3ParametrizationBase", bp::no_init)
        .def("GetName", &I3ParametrizationBase::GetName)
        .def("InitChainRule", &I3ParametrizationBase::InitChainRule)
        .def("SetEvent", &I3ParametrizationBase::SetEvent)
        .def("SetHypothesisPtr", &I3ParametrizationBase::SetHypothesisPtr)  
        .def("GetGradientPtr", &I3ParametrizationBase::GetGradientPtr)
        .def("GetGradient", GetGradient)
        .def("GetNPar", &I3ParametrizationBase::GetNPar)
        .def("GetParInitSpecs", &I3ParametrizationBase::GetParInitSpecs,
            bp::return_internal_reference<>())
        .def("GetHypothesisPtr", GetHypothesisPtr)
        .def("CalculateHypothesis", CalcHypothesis)
        WRAP_CONTEXT(I3ParametrizationBase)
    ;

    bp::class_<I3ParametrizationWrapper, boost::shared_ptr<I3ParametrizationWrapper>,
        boost::noncopyable>("I3Parametrization", bp::init<const std::string &>())
        .def("GetName", &I3ParametrizationBase::GetName)
        .def("InitChainRule", &I3ParametrizationBase::InitChainRule)
        .def("SetEvent", &I3ParametrizationBase::SetEvent)
        .def("SetHypothesisPtr", &I3ParametrizationBase::SetHypothesisPtr)  
        .def("GetGradientPtr", &I3ParametrizationBase::GetGradientPtr)
        .def("GetGradient", GetGradient)
        .def("GetNPar", &I3ParametrizationBase::GetNPar)
        .def("GetParInitSpecs", &I3ParametrizationBase::GetParInitSpecs,
            bp::return_internal_reference<>())
        .def("GetHypothesisPtr", GetHypothesisPtr)
        .def("CalculateHypothesis", CalcHypothesis)
        .add_property("par", bp::make_function(&I3ParametrizationWrapper::GetPar, bp::return_internal_reference<>()))
        .add_property("par_gradient", bp::make_function(&I3ParametrizationWrapper::GetParGradient, bp::return_internal_reference<>()))
        .add_property("parspecs", bp::make_function(&I3ParametrizationWrapper::GetParSpecs, bp::return_internal_reference<>()))
        .add_property("hypothesis", &I3ParametrizationWrapper::GetHypothesis, &I3ParametrizationWrapper::SetHypothesis)
        .add_property("gradient", &I3ParametrizationWrapper::GetGradient, &I3ParametrizationWrapper::SetGradient)
        WRAP_CONTEXT(I3ParametrizationBase)
    ;
}
