/**
 *
 * Implementation of Python bindings for parametrization services
 *
 * (c) 2015
 * the IceCube Collaboration
 * $Id$
 *
 * @file parametrization.cxx
 * @date $Date$
 * @author boersma
 *
 */
#include <string>
#include <vector>

#include <boost/preprocessor.hpp>
#include <boost/python.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>

// context_suite assumes that this is #included already
#include <icetray/I3Frame.h>
#include <icetray/python/context_suite.hpp>
#include <gulliver/I3ParametrizationBase.h>
#include <lilliput/parametrization/I3SimpleParametrization.h>
#include <lilliput/parametrization/I3HalfSphereParametrization.h>

#define PARAMETRIZATIONS_WRAP_CONTEXT(base) \
    .def(icetray::python::context_suite<base>())

#define PARAMETRIZATIONS_ENUM_VALUE(r, name, key) \
    .value( \
        BOOST_PP_STRINGIZE(key), \
        name::key \
    )

namespace bp = boost::python;

namespace
{
    /**
     * @brief Wrap the `I3SimpleParametrization` constructor.
     *
     * Extract steps, absolute and relative bounds, which are C++ vectors, from
     * supported Python objects like list, tuple, and so on.
     *
     */
    I3SimpleParametrizationPtr
    InitSimpleParametrization(
        const std::string& name,
        const bp::object& steps,
        const bp::object& absbounds,
        const bp::object& relbounds,
        int vertexmode,
        bool trace
    )
    {
        using Steps = std::vector<double>;
        using StepsPtr = boost::shared_ptr<Steps>;

        auto stepsVec = boost::make_shared<Steps>(
            I3SimpleParametrization::no_steps_
        );

        if (!steps.is_none())
        {
            stepsVec = bp::extract<StepsPtr>(steps);
        }

        using Bounds = std::vector<std::vector<double> >;
        using BoundsPtr = boost::shared_ptr<Bounds>;

        auto absBoundsVec = boost::make_shared<Bounds>(
            I3SimpleParametrization::all_nobounds_
        );

        if (!absbounds.is_none())
        {
            absBoundsVec = bp::extract<BoundsPtr>(absbounds);
        }

        auto relBoundsVec = boost::make_shared<Bounds>(
            I3SimpleParametrization::all_nobounds_
        );

        if (!relbounds.is_none())
        {
            relBoundsVec = bp::extract<BoundsPtr>(relbounds);
        }

        return boost::make_shared<I3SimpleParametrization>(
            name, *stepsVec, *absBoundsVec, *relBoundsVec, vertexmode, trace
        );
    }
}

void register_parametrization()
{
    auto simpleParametrization =
    bp::class_<
        I3SimpleParametrization,
        bp::bases<I3ParametrizationBase>,
        I3SimpleParametrizationPtr,
        boost::noncopyable
    >
    (
        "I3SimpleParametrization",
        "Simple parametrization service\n"
        "\n"
        "The fit parameters are equal the variables given\n"
        "by `simplepar_t`. There are no transformations except for the\n"
        "energy, which is converted to base 10 logarithm of energy.\n"
        "\n"
        "Parameters\n"
        "----------\n"
        "name : str, optional\n"
        "    Logging identifier\n"
        "steps : array_like, optional\n"
        "    Sequence of step sizes along the fit parameters\n"
        "absbounds : array_like, optional\n"
        "    Sequence of lower and upper bounds along the fit parameters\n"
        "relbounds : array_like, optional\n"
        "    Sequence of lower and upper relative bounds w.r.t. the seed\n"
        "    along the fit parameters\n"
        "vertexmode : vertexmode_t, optional\n"
        "    Specify the actual meaning of the vertex position.\n"
        "trace : bool, optional\n"
        "    If `True`, the seed particle and a sequence of\n"
        "    all `I3Particle` objects created during a fit will be\n"
        "    stored and made available as 'Diagnostics'.\n",
        bp::no_init
    )
    .def(
        "GetName",
        &I3SimpleParametrization::GetName
    )
    .def(
        "InitChainRule",
        &I3SimpleParametrization::InitChainRule
    )
    .def(
        "SetEvent",
        &I3SimpleParametrization::SetEvent
    )
    .def(
        "SetHypothesisPtr",
        &I3SimpleParametrization::SetHypothesisPtr
    )
    .def(
        "GetGradientPtr",
        &I3SimpleParametrization::GetGradientPtr
    )
    .def(
        "SetStep",
        &I3SimpleParametrization::SetStep,
        (
            bp::arg("param"),
            bp::arg("step")=0.,
            bp::arg("verbose")=false
        ),
        "Set the step size for the fit parameter `param`.\n"
        "\n"
        "Parameters\n"
        "----------\n"
        "param : simplepar_t\n"
        "    Fit parameter\n"
        "step : float, optional\n"
        "    Step size\n"
        "verbose : bool, optional\n"
        "    Run some basic sanity checks."
    )
    .def(
        "SetAbsBounds",
        &I3SimpleParametrization::SetAbsBounds,
        (
            bp::arg("param"),
            bp::arg("vmin")=0.,
            bp::arg("vmax")=0.,
            bp::arg("verbose")=false
        ),
        "Set the absolute bounds for the fit parameter `param`.\n"
        "\n"
        "Parameters\n"
        "----------\n"
        "param : simplepar_t\n"
        "    Fit parameter\n"
        "vmin, vmax : float, optional\n"
        "    Lower and upper bounds\n"
        "verbose : bool, optional\n"
        "    Run some basic sanity checks."
    )
    .def(
        "SetRelBounds",
        &I3SimpleParametrization::SetRelBounds,
        (
            bp::arg("param"),
            bp::arg("vmin")=0.,
            bp::arg("vmax")=0.,
            bp::arg("verbose")=false
        ),
        "Set the relative bounds for the fit parameter `param`.\n"
        "\n"
        "Parameters\n"
        "----------\n"
        "param : simplepar_t\n"
        "    Fit parameter\n"
        "vmin, vmax : float, optional\n"
        "    Lower and upper bounds\n"
        "verbose : bool, optional\n"
        "    Run some basic sanity checks."
    )
    .def(
        "GetNPar", &I3SimpleParametrization::GetNPar
    )
    .def(
        "GetParInitSpecs",
        &I3SimpleParametrization::GetParInitSpecs,
        bp::return_internal_reference<>()
    )
    PARAMETRIZATIONS_WRAP_CONTEXT(I3SimpleParametrization)
    ;

    {
        bp::scope simpleParametrizationScope(simpleParametrization);

        bp::enum_<I3SimpleParametrization::vertexmode_t>(
            "vertexmode_t",
            "Specifies the actual meaning of the vertex position.\n"
            "\n"
            "Attributes\n"
            "----------\n"
            "VERTEX_Default\n"
            "    Default definition; depends on the particle's shape.\n"
            "VERTEX_Start\n"
            "    Starting point\n"
            "VERTEX_Stop\n"
            "    Stopping point\n"
            "VERTEX_N\n"
            "    Number of vertex parametrization modes"
        )
        BOOST_PP_SEQ_FOR_EACH(
            PARAMETRIZATIONS_ENUM_VALUE,
            I3SimpleParametrization::vertexmode_t,
            I3SIMPLEPARAMETRIZATION_VERTEXMODE_T_SEQ
        )
        .export_values()
        ;

        bp::enum_<I3SimpleParametrization::simplepar_t>(
            "simplepar_t",
            "Supported fit parameters\n"
            "\n"
            "Attributes\n"
            "----------\n"
            "PAR_T\n"
            "    Vertex time\n"
            "PAR_X, PAR_Y, PAR_Y\n"
            "    Vertex position (x, y, and z coordinates)\n"
            "PAR_Zen, PAR_Azi\n"
            "    Direction (zenith and azimuth angles)\n"
            "PAR_LinE, PAR_LogE\n"
            "    Energy (linear or base 10 logarithm)\n"
            "PAR_LinL, PAR_LogL\n"
            "    Track length (linear or base 10 logarithm)\n"
            "PAR_N\n"
            "    Number of supported fit parameters"
        )
        BOOST_PP_SEQ_FOR_EACH(
            PARAMETRIZATIONS_ENUM_VALUE,
            I3SimpleParametrization::simplepar_t,
            I3SIMPLEPARAMETRIZATION_SIMPLEPAR_T_SEQ
        )
        .export_values()
        ;
    }

    static bp::object none;

    simpleParametrization.def(
        "__init__",
        bp::make_constructor(
            InitSimpleParametrization,
            bp::default_call_policies(),
            (
                bp::arg("name")="I3SimpleParametrization",
                bp::arg("steps")=none,
                bp::arg("absbounds")=none,
                bp::arg("relbounds")=none,
                bp::arg("vertexmode")=
                    I3SimpleParametrization::vertexmode_t::VERTEX_Default,
                bp::arg("trace")=false
            )
        )
    );

    bp::def("identity", identity_<I3SimpleParametrization::vertexmode_t>);

    bp::class_<
        I3HalfSphereParametrization,
        bp::bases<I3ParametrizationBase>,
        I3HalfSphereParametrizationPtr,
        boost::noncopyable
    >
    (
        "I3HalfSphereParametrization",
        "I3HalfSphereParametrization("
        "self, "
        "name='I3HalfSphereParametrization', "
        "ddir=0.3, "
        "dxyz=30., "
        "dLogE=0., "
        "dt=0."
        ")\n"
        "\n"
        "Simple half-sphere parametrization service\n"
        "\n"
        "A simple parametrization that restricts the direction phase\n"
        "space to one hemisphere centered around the seed track\n"
        "direction.\n"
        "\n"
        "Parameters\n"
        "----------\n"
        "name : str, optional\n"
        "    Logging identifier\n"
        "ddir : float, optional\n"
        "    Positive step size along direction\n"
        "dxyz : float, optional\n"
        "    Positive step size along vertex position; if `0.`, the\n"
        "    vertex position remains fixed.\n"
        "dLogE : float, optional\n"
        "    Positive step size along energy (base 10 logarithm);\n"
        "    if `0`, the energy remains fixed.\n"
        "dt : float, optional\n"
        "    Positive step size along vertex time; if `0`, the vertex\n"
        "    time remains fixed.\n"
        "\n"
        "See Also\n"
        "--------\n"
        "I3SimpleParametrization\n"
        "\n",
        bp::init<const std::string&, double, double, double, double>(
            (
                bp::arg("name")="I3HalfSphereParametrization",
                bp::arg("ddir")=0.3*I3Units::radian,
                bp::arg("dxyz")=30.*I3Units::m,
                bp::arg("dLogE")=0.,
                bp::arg("dt")=0.
            )
        )
    )
    .def(
        "GetName",
        &I3HalfSphereParametrization::GetName
    )
    .def(
        "InitChainRule",
        &I3HalfSphereParametrization::InitChainRule
    )
    .def(
        "SetEvent",
        &I3HalfSphereParametrization::SetEvent
        )
    .def(
        "SetHypothesisPtr",
        &I3HalfSphereParametrization::SetHypothesisPtr
    )
    .def(
        "GetGradientPtr",
        &I3HalfSphereParametrization::GetGradientPtr
    )
    .def(
        "GetNPar",
        &I3HalfSphereParametrization::GetNPar
    )
    .def(
        "GetParInitSpecs",
        &I3HalfSphereParametrization::GetParInitSpecs,
        bp::return_internal_reference<>()
    )
    PARAMETRIZATIONS_WRAP_CONTEXT(I3HalfSphereParametrization)
    ;
}
