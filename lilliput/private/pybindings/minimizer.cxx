/**
 *
 * Implementation of Python bindings for minimizer services
 *
 * (c) 2017
 * the IceCube Collaboration
 * $Id$
 *
 * @file minimizer.cxx
 * @date $Date$
 * @author kkrings
 * @author kjmeagher
 *
 */
#include <string>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/python.hpp>

#include <gulliver/I3MinimizerBase.h>
#include <lilliput/minimizer/I3GulliverMinuit.h>

#ifdef USE_MINUIT2
#include <lilliput/minimizer/I3GulliverMinuit2.h>
#endif

#ifdef USE_NLOPT
#include <lilliput/minimizer/I3GulliverNLopt.hpp>
#endif

namespace bp = boost::python;

void register_minimizer()
{
    bp::class_<I3GulliverMinuit,
               bp::bases<I3MinimizerBase>,
               boost::shared_ptr<I3GulliverMinuit>,
               boost::noncopyable>(
        "I3GulliverMinuit",
        "Wraps TMinuit for Gulliver-based reconstructions.\n\n"
        "Parameters\n"
        "----------\n"
        "name : str, optional\n"
        "   Logging identifier\n"
        "tolerance : float, optional\n"
        "   Stop minimization after `tolerance` was reached.\n"
        "max_iterations : int, optional\n"
        "   Maximum number of iterations\n"
        "minuit_print_level : int, optional\n"
        "   Configure verbose level; the default is as quiet as possible.\n"
        "minuit_strategy : int, optional\n"
        "   Configure minimization strategy; the default optimizes for \n"
        "   accurancy rather than speed.\n"
        "algorithm : {SIMPLEX, MIGRAD}, optional\n"
        "   Configure minimization algorithm.\n"
        "flatness_check : bool, optional\n"
        "   Check if a real minimum was found or if the minimization stopped\n"
        "   because of no variation in the likelihood function. In case of\n"
        "   the latter, non-convergence will be reported.",
        bp::init<std::string, const double, unsigned int, int, int,
                 std::string, bool>(
            (
             bp::arg("name")="Minuit",
             bp::arg("tolerance")=0.1,
             bp::arg("max_iterations")=10000,
             bp::arg("minuit_print_level")=-2,
             bp::arg("minuit_strategy")=2,
             bp::arg("algorithm")="SIMPLEX",
             bp::arg("flatness_check")=false
             )
            )
        )
        .add_property(
            "name",
            &I3GulliverMinuit::GetName,
            "str: Logging identifier"
            )
        .add_property(
            "tolerance",
            &I3GulliverMinuit::GetTolerance,
            &I3GulliverMinuit::SetTolerance,
            "float: Stop minimization after `tolerance` was reached."
            )
        .add_property(
            "max_iterations",
            &I3GulliverMinuit::GetMaxIterations,
            &I3GulliverMinuit::SetMaxIterations,
            "int: Maximum number of iterations"
            )
        ;

#ifdef USE_MINUIT2
    bp::class_<I3GulliverMinuit2,
               bp::bases<I3MinimizerBase>,
               boost::shared_ptr<I3GulliverMinuit2>,
               boost::noncopyable>(
        "I3GulliverMinuit2",
        "Wraps Minuit2 for Gulliver-based reconstructions.\n\n"
        "Parameters\n"
        "----------\n"
        "name : str, optional\n"
        "   Logging identifier\n"
        "tolerance : float, optional\n"
        "   Stop minimization after `tolerance` was reached.\n"
        "max_iterations : int, optional\n"
        "   Maximum number of iterations\n"
        "minuit_print_level : int, optional\n"
        "   Configure verbose level; the default is as quiet as possible.\n"
        "minuit_strategy : int, optional\n"
        "   Configure minimization strategy; the default optimizes for \n"
        "   accurancy rather than speed.\n"
        "algorithm : {SIMPLEX, MIGRAD, COMBINED, FUMILI}, optional\n"
        "   Configure minimization algorithm.\n"
        "flatness_check : bool, optional\n"
        "   Check if a real minimum was found or if the minimization stopped\n"
        "   because of no variation in the likelihood function. In case of\n"
        "   the latter, non-convergence will be reported.\n"
        "with_gradients : bool, optional\n"
        "   Use analytic gradients; only supported by ``MIGRAD``.\n"
        "check_gradient : bool, optional\n"
        "   Check analytic gradients against the numerical gradient.\n"
        "ignore_edm : bool, optional\n"
        "   Treat ``MIGRAD`` fits that exceed the maximum\n"
        "   estimated-distance-to-minimum (EDM) as having converged.",
        bp::init<std::string, const double, unsigned int, int, int,
                 std::string, bool, bool, bool, bool>(
            (
             bp::arg("name")="Minuit2",
             bp::arg("tolerance")=0.1,
             bp::arg("max_iterations")=1000,
             bp::arg("minuit_print_level")=-2,
             bp::arg("minuit_strategy")=2,
             bp::arg("algorithm")="SIMPLEX",
             bp::arg("flatness_check")=true,
             bp::arg("with_gradients")=false,
             bp::arg("check_gradient")=false,
             bp::arg("ignore_edm")=false
             )
            )
        )
        .add_property(
            "name",
            &I3GulliverMinuit2::GetName,
            "str: Logging identifier"
            )
        .add_property(
            "tolerance",
            &I3GulliverMinuit2::GetTolerance,
            &I3GulliverMinuit2::SetTolerance,
            "float: Stop minimization after `tolerance` was reached."
            )
        .add_property(
            "max_iterations",
            &I3GulliverMinuit2::GetMaxIterations,
            &I3GulliverMinuit2::SetMaxIterations,
            "int: Maximum number of iterations"
            )
        .add_property(
            "uses_gradient",
            &I3GulliverMinuit2::UsesGradient,
            "bool: Check if gradients are supported and enabled."
            )
        .add_property(
            "check_gradient",
            &I3GulliverMinuit2::GetCheckGradient,
            &I3GulliverMinuit2::SetCheckGradient,
            "bool: Check analytic gradients against the numerical gradient."
            )
        ;
#endif // MINUIT2_FOUND

#ifdef USE_NLOPT
    bp::class_<I3GulliverNLopt,
               bp::bases<I3MinimizerBase>,
               boost::shared_ptr<I3GulliverNLopt>,
               boost::noncopyable>(
        "I3GulliverNLopt",
        "Wraps NLopt minimizer algorithms for Gulliver-based "
        "reconstructions.\n\n"
        "Parameters\n"
        "----------\n"
        "name : str, optional\n"
        "   Identifier for logging\n"
        "tolerance : float, optional\n"
        "   Stop iteration when the parameters change by less than this\n"
        "   relative amount.\n"
        "max_iterations : int, optional\n"
        "   Stop iteration when the number of iterations reaches this\n"
        "   number.\n"
        "algorithm : str, optional\n"
        "   Name of the minimization algorithm to use",
        bp::init<std::string, std::string, double, int>(
            (
             bp::arg("name")="I3GulliverNLopt",
             bp::arg("algorithm")="LN_BOBYQA",
             bp::arg("tolerance")=0.001,
             bp::arg("max_iterations")=10000
             )
            )
        )
        .add_property(
            "name",
            &I3GulliverNLopt::GetName,
            "str: Identifier for logging"
            )
        .add_property(
            "tolerance",
            &I3GulliverNLopt::GetTolerance,
            &I3GulliverNLopt::SetTolerance,
            "float: Stop iteration when the parameters change by less than "
            "this relative amount."
            )
        .add_property(
            "max_iterations",
            &I3GulliverNLopt::GetMaxIterations,
            &I3GulliverNLopt::SetMaxIterations,
            "int: Stop iteration when the number of iterations reaches this "
            "number."
            )
        ;
#endif // NLOPT_FOUND
}
