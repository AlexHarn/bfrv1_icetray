#include <I3Test.h>

#include "I3TestDummyParametrization.h"
#include "I3TestDummyMinimizer.h"
#include "I3TestDummyEventLogLikelihood.h"

#include "gulliver/I3Gulliver.h"
#include "dataclasses/I3Direction.h"
#include "dataclasses/physics/I3Particle.h"

// #include <cstdio>

// all tests use now "testdummy" implementations of the base classes
// those dummy implementations are just 'complicated' enough to test the system;
// they have no further useful meaning.

TEST_GROUP(Gulliver_Gradient);

// testing the parametrization mechanisms


// test that derivatives are computed correctly
// there should be somehow a templated or generalized version of this test
// such that when you write a new LLH service you can just give that
// to the test, with a few interesting seed values, and then the test rolls.
TEST(CheckDerivatives){

    // the "true" value of the fit values in this dummy example
    const double L = 5.5*I3Units::m;
    const double azi = 0.9*I3Units::radian;
    const double x = 4.1*I3Units::m;

    // deviation scale for dummy test loglikelihood
    const double L_sigma = 1.3*I3Units::m;
    const double azi_sigma = 0.5*I3Units::radian;
    const double x_sigma = 1.25*I3Units::m;

    // the "seed" values of the fit values in this dummy example
    const double L_seed = 6.5*I3Units::m;
    const double azi_seed = 1.5*I3Units::radian;
    const double x_seed = 3.1*I3Units::m;

    // the other variables (ignored)
    const double zen = 1.0*I3Units::radian;
    const double y = 5.0*I3Units::m;
    const double z = 6.0*I3Units::m;
    const double epsilon1 = 1.0e-2*I3Units::radian;
    const double epsilon2 = 1.0e-4*I3Units::radian;

    // seed hypothesis
    I3EventHypothesisPtr dumb_ptr( new I3EventHypothesis);
    I3LogLikelihoodFitPtr dumb_fitptr( new I3LogLikelihoodFit(*dumb_ptr) );
    dumb_ptr->particle->SetLength( L_seed );
    dumb_ptr->particle->SetDir( zen, azi_seed );
    dumb_ptr->particle->SetPos( x_seed, y, z );

    // test dummy likelihood service
    I3EventLogLikelihoodBasePtr
        dumb_llh( new I3TestDummyEventLogLikelihood(
                    L, azi, x, L_sigma, azi_sigma, x_sigma ) );

    // gradient object
    I3EventHypothesisPtr grad_ptr( new I3EventHypothesis);
    grad_ptr->particle->SetTime(0.);
    grad_ptr->particle->SetEnergy(0.);
    grad_ptr->particle->SetLength(1.);
    grad_ptr->particle->SetDir(0.,1.);
    grad_ptr->particle->SetPos(1.,0.,0.);

    // get LLH and the analytical gradient (for seed hypothesis)
    double llh_grad = dumb_llh->GetLogLikelihoodWithGradient(*dumb_ptr,*grad_ptr);

    // check that if you ask for ONLY the LLH, you get the SAME LLH
    double llh_dumb = dumb_llh->GetLogLikelihood(*dumb_ptr);
    ENSURE_DISTANCE( llh_dumb, llh_grad, epsilon1,
                     "LLH value should be the same regardless of "
                     "whether you ask for gradient or not");

    // check dLogL/dLength with numerical approximation
    dumb_ptr->particle->SetLength(L_seed+epsilon2);
    double llh_eps = dumb_llh->GetLogLikelihood(*dumb_ptr);
    dumb_ptr->particle->SetLength( L_seed ); // init away from minimum
    double llh = dumb_llh->GetLogLikelihood(*dumb_ptr);
    double dLogLdL_numerical = (llh_eps-llh)/epsilon2;
    double dLogLdL_analytical = grad_ptr->particle->GetLength();
    ENSURE_DISTANCE( dLogLdL_numerical, dLogLdL_analytical,
                     fabs(epsilon1*dLogLdL_numerical),
                     "analytical gradient (L) should close to numerical one");

    // check dLogL/dAzi with numerical approximation
    dumb_ptr->particle->SetDir( zen, azi_seed+epsilon2 );
    llh_eps = dumb_llh->GetLogLikelihood(*dumb_ptr);
    dumb_ptr->particle->SetDir( zen, azi_seed ); // init away from minimum
    llh = dumb_llh->GetLogLikelihood(*dumb_ptr);
    double dLogLdAzi_numerical = (llh_eps-llh)/epsilon2;
    double dLogLdAzi_analytical = grad_ptr->particle->GetDir().GetAzimuth();
    ENSURE_DISTANCE( dLogLdAzi_numerical, dLogLdAzi_analytical,
                     fabs(epsilon1*dLogLdAzi_numerical),
                     "analytical gradient (azi) should close to numerical one");

    // check dLogL/dX with numerical approximation
    dumb_ptr->particle->SetPos( x_seed+epsilon2, y, z );
    llh_eps = dumb_llh->GetLogLikelihood(*dumb_ptr);
    dumb_ptr->particle->SetPos( x_seed, y, z ); // init away from minimum
    llh = dumb_llh->GetLogLikelihood(*dumb_ptr);
    double dLogLdX_numerical = (llh_eps-llh)/epsilon2;
    double dLogLdX_analytical = grad_ptr->particle->GetPos().GetX();
    ENSURE_DISTANCE( dLogLdX_numerical, dLogLdX_analytical,
                     fabs(epsilon1*dLogLdX_numerical),
                     "analytical gradient (X) should close to numerical one");

}

// test fitter mechanism
TEST(GulliverFitWithGradient){

    // the "true" value of the fit values in this dummy example
    const double L = 5.5*I3Units::m;
    const double azi = 0.9*I3Units::radian;
    const double x = 4.1*I3Units::m;

    // deviation scale for dummy test loglikelihood
    const double L_sigma = 1.3*I3Units::m;
    const double azi_sigma = 0.5*I3Units::radian;
    const double x_sigma = 1.25*I3Units::m;

    // the "seed" values of the fit values in this dummy example
    // just a bit away from the "truth"
    const double L_seed = 6.5*I3Units::m;
    const double azi_seed = 1.5*I3Units::radian;
    const double x_seed = 3.1*I3Units::m;

    // the other variables (ignored)
    const double zen = 1.0*I3Units::radian;
    const double y = 5.0*I3Units::m;
    const double z = 6.0*I3Units::m;
    const double t = 12345.0*I3Units::ns;
    const double energy = 12345.0*I3Units::GeV;

    // initialize fit pointer
    I3EventHypothesisPtr seed_ptr( new I3EventHypothesis);
    I3LogLikelihoodFitPtr fitptr( new I3LogLikelihoodFit(*seed_ptr) );
    seed_ptr->particle->SetLength( L_seed );
    seed_ptr->particle->SetTime( t );
    seed_ptr->particle->SetEnergy( energy );
    seed_ptr->particle->SetDir( zen, azi_seed );
    seed_ptr->particle->SetPos( x_seed, y, z );

    // load up a gulliver object
    I3ParametrizationBasePtr
        dumb_param( new I3TestDummyTrackParametrization( seed_ptr, 0.1, 0.02, 0.5 ) );
    I3MinimizerBasePtr
        dumb_mini( new I3TestDummyMinimizer(0.001,0.7,100,true) );
    I3EventLogLikelihoodBasePtr
        dumb_llh( new I3TestDummyEventLogLikelihood(
                    L, azi, x, L_sigma, azi_sigma, x_sigma ) );
    I3Gulliver g(dumb_param, dumb_llh, dumb_mini, "GradientTest" );


    // do tests
    ENSURE( g.Fit( fitptr ), "that dumb fit converges");
    ENSURE_DISTANCE( fitptr->fitparams_->logl_, 0.0, 0.001,
            "that fitted -log(L) minimum "
            "is the one expected from I3TestDummyEventLogLikelihood" );

}
