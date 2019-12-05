#include <I3Test.h>
#include "gulliver/I3Gulliver.h"
#include "I3TestDummyParametrization.h"
#include "I3TestDummyMinimizer.h"
#include "I3TestDummyEventLogLikelihood.h"
#include "dataclasses/I3Direction.h"
#include "dataclasses/physics/I3Particle.h"
#include <cstdio>

// all tests use now "testdummy" implementations of the base classes
// those dummy implementations are just 'complicated' enough to test the system;
// they have no further useful meaning.

TEST_GROUP(Gulliver_Interfaces);

TEST(DefaultConstructor) {
    I3EventHypothesisPtr dumb_ptr(new I3EventHypothesis);
    dumb_ptr->particle = I3ParticlePtr(new I3Particle);
    dumb_ptr->particle->SetLength(3.0);
    dumb_ptr->particle->SetDir(M_PI / 2, M_PI / 2);
    I3ParametrizationBasePtr
    dumb_param(new I3TestDummyTrackParametrization(dumb_ptr, 0.1, 0.02, 0.5));
    I3MinimizerBasePtr
    dumb_mini(new I3TestDummyMinimizer);
    I3EventLogLikelihoodBasePtr
    dumb_llh(new I3TestDummyEventLogLikelihood(10.0/*L*/, 2.0/*azi*/, 30.0/*X*/, 4.0/*dL*/, 5.0/*dAzi*/, 5.0/*dX*/));
    I3Gulliver g(dumb_param, dumb_llh, dumb_mini, "LikelihoodInterface");
}

TEST(ChangeParametrization) {
    I3EventHypothesisPtr dumb_ptr(new I3EventHypothesis);
    dumb_ptr->particle = I3ParticlePtr(new I3Particle);
    dumb_ptr->particle->SetLength(3.0);
    dumb_ptr->particle->SetDir(M_PI / 2, M_PI / 2);
    I3ParametrizationBasePtr
    dumb_param(new I3TestDummyTrackParametrization(dumb_ptr, 0.1, 0.02, 0.5));
    I3MinimizerBasePtr
    dumb_mini(new I3TestDummyMinimizer);
    I3EventLogLikelihoodBasePtr
    dumb_llh(new I3TestDummyEventLogLikelihood(10.0/*L*/, 2.0/*azi*/, 30.0/*X*/, 4.0/*dL*/, 5.0/*dAzi*/, 5.0/*dX*/));
    I3Gulliver g(dumb_param, dumb_llh, dumb_mini, "LikelihoodInterface");

    vector<double> mypar(3,42.0);
    double val1 = g(mypar);

    I3ParametrizationBasePtr
    dumb_param2(new I3TestDummyTrackParametrization(dumb_ptr, 0.1, 0.02, 0.5));
    g.ChangeParametrization(dumb_param2);
    double val2 = g(mypar);

    ENSURE(val1 == val2);
}

TEST(ChangeMinimizer) {
    I3EventHypothesisPtr dumb_ptr(new I3EventHypothesis);
    dumb_ptr->particle = I3ParticlePtr(new I3Particle);
    dumb_ptr->particle->SetLength(3.0);
    dumb_ptr->particle->SetDir(M_PI / 2, M_PI / 2);
    I3ParametrizationBasePtr
    dumb_param(new I3TestDummyTrackParametrization(dumb_ptr, 0.1, 0.02, 0.5));
    I3MinimizerBasePtr
    dumb_mini(new I3TestDummyMinimizer);
    I3EventLogLikelihoodBasePtr
    dumb_llh(new I3TestDummyEventLogLikelihood(10.0/*L*/, 2.0/*azi*/, 30.0/*X*/, 4.0/*dL*/, 5.0/*dAzi*/, 5.0/*dX*/));
    I3Gulliver g(dumb_param, dumb_llh, dumb_mini, "LikelihoodInterface");

    vector<double> mypar(3,42.0);
    double val1 = g(mypar);

    I3MinimizerBasePtr
    dumb_mini2(new I3TestDummyMinimizer);
    g.ChangeMinimizer(dumb_mini2);
    double val2 = g(mypar);

    ENSURE(val1 == val2);
}

// testing the parametrization mechanisms

TEST(Parametrization)
{

    // make a track and its parametrization
    double rho = 3.0;
    double XYZ = 10.0;
    I3EventHypothesisPtr dumb_ptr( new I3EventHypothesis );
    dumb_ptr->particle = I3ParticlePtr(new I3Particle);
    dumb_ptr->particle->SetLength( rho );
    dumb_ptr->particle->SetDir( M_PI/2, M_PI/2 );
    dumb_ptr->particle->SetPos( XYZ, XYZ, XYZ );
    I3ParametrizationBasePtr dumb_param( new I3TestDummyTrackParametrization( dumb_ptr, 0.1, 0.02, 0.5 ) );

    // check dimension of parametrization
    ENSURE( dumb_param->GetNPar()==3,
        "that trivial parametrization has 3 parameters for a dummy track");

    // check that parameters were initialized correctly from the "physics" variables
    const std::vector<I3FitParameterInitSpecs>& parspecs = dumb_param->GetParInitSpecs();
    ENSURE_DISTANCE( parspecs[0].initval_, 0.0, 0.1,
        "that trivial parametrization [0] == 3.0 * cos(pi/2) = 0.0 " );
    ENSURE_DISTANCE( parspecs[1].initval_, rho, 0.1,
        "that trivial parametrization [1] == 3.0 * sin(pi/2) = 3.0 " );
    ENSURE_DISTANCE( parspecs[2].initval_, 10.0, 0.1,
        "that trivial parametrization [2] == 10.0 " );

    // get physics variable from new parameters
    vector<double> p(3);
    p[0] = sqrt(2.0);
    p[1] = sqrt(2.0);
    p[2] = 30.0;
    const I3EventHypothesisConstPtr d = dumb_param->GetHypothesisPtr( p );
    ENSURE_DISTANCE( d->particle->GetLength(), 2.0, 0.01,
            "that Length is two when par[0] and par[1] of dummy parametrization are both sqrt(2)" );
    ENSURE_DISTANCE( d->particle->GetAzimuth(), M_PI/4, 0.01,
            "that Azimuth is pi/4 when par[0] and par[1] of dummy parametrization are both sqrt(2)" );
    ENSURE_DISTANCE( d->particle->GetPos().GetX(), 30.0, 0.01,
            "that X is 30.0 when par[2] of dummy parametrization is 30.0" );

}


// test likelihood calculation interface for minimizer
TEST(LikelihoodInterface){

    // same init as before
    I3EventHypothesisPtr dumb_ptr( new I3EventHypothesis );
    dumb_ptr->particle = I3ParticlePtr(new I3Particle);
    dumb_ptr->particle->SetLength( 3.0 );
    dumb_ptr->particle->SetDir( M_PI/2, M_PI/2 );
    I3ParametrizationBasePtr
        dumb_param( new I3TestDummyTrackParametrization( dumb_ptr, 0.1, 0.02, 0.5 ) );
    I3MinimizerBasePtr
        dumb_mini( new I3TestDummyMinimizer );
    I3EventLogLikelihoodBasePtr
        dumb_llh( new I3TestDummyEventLogLikelihood(10.0/*L*/,2.0/*azi*/,30.0/*X*/,4.0/*dL*/,5.0/*dAzi*/,5.0/*dX*/) );
    I3Gulliver g(dumb_param, dumb_llh, dumb_mini, "LikelihoodInterface" );

    vector<double> mypar(3,42.0);
    double via_gulliver = g( mypar );
    double direct_llh = -1*dumb_llh->GetLogLikelihood( *(dumb_param->GetHypothesisPtr(mypar) ));
    ENSURE_DISTANCE( via_gulliver, direct_llh,
            0.001,
            "that likelihood calculated via parametrization is same as the direct calculation" );

}

// test fitter mechanism
TEST(GulliverFit){

    I3EventHypothesisPtr dumb_ptr( new I3EventHypothesis);
    I3LogLikelihoodFitPtr dumb_fitptr( new I3LogLikelihoodFit(*dumb_ptr) );
    dumb_fitptr->hypothesis_->particle->SetLength( 4.5 ); // init away from minimum
    dumb_fitptr->hypothesis_->particle->SetDir( 1.0, 1.9 ); // init away from minimum
    dumb_fitptr->hypothesis_->particle->SetPos( 4.0, 5.0, 5.0 ); // init away from minimum
    I3ParametrizationBasePtr
        dumb_param( new I3TestDummyTrackParametrization( dumb_ptr, 0.1, 0.02, 0.5 ) );
    I3MinimizerBasePtr
        dumb_mini( new I3TestDummyMinimizer(0.001,0.85,1000) );
    I3EventLogLikelihoodBasePtr
        dumb_llh( new I3TestDummyEventLogLikelihood(5.5/*L*/,0.9/*azi*/,4.1/*X*/,1.3/*dL*/,0.15/*dAzi*/,1.25/*dX*/) );
    I3Gulliver g(dumb_param, dumb_llh, dumb_mini, "GulliverFit" );

    ENSURE( g.Fit( dumb_fitptr ), "that dumb fit converges");
    ENSURE_DISTANCE( dumb_fitptr->fitparams_->logl_, 0.0, 0.1,
            "that fitted loglh max is the one expected from I3TestDummyEventLogLikelihood" );
    // ENSURE_DISTANCE( dumb_fitptr->fitparams_->logl_, -1.0, 0.001,
      //       "that fitted loglh max is the one expected from I3TestDummyEventLogLikelihood" );

}

// test tracing mechanism
TEST(GulliverTraceTest){

    I3EventHypothesisPtr dumb_ptr( new I3EventHypothesis);
    I3ParametrizationBasePtr
        dumb_param( new I3TestDummyTrackParametrization( dumb_ptr, 0.1, 0.02, 0.5 ) );
    I3MinimizerBasePtr
        dumb_mini( new I3TestDummyMinimizer(0.001,0.85,1000,false) );
    I3EventLogLikelihoodBasePtr
        dumb_llh( new I3TestDummyEventLogLikelihood(5.5/*L*/,0.9/*azi*/,4.1/*X*/,1.3/*dL*/,0.15/*dAzi*/,1.25/*dX*/) );
    I3Gulliver g(dumb_param, dumb_llh, dumb_mini, "GulliverFit" );

    log_info("so far so good 1");
    g.Trace();
    std::vector<double> testpar(3);
    size_t expsize = 8; // 2*(3+1)
    std::vector<double> expected_trace(expsize); // 2*(3+1)
    expected_trace[0] = testpar[0] = 7.;
    expected_trace[1] = testpar[1] = 6.;
    expected_trace[2] = testpar[2] = 5.;
    expected_trace[3] = g(testpar);
    expected_trace[4] = testpar[0] = 4.;
    expected_trace[5] = testpar[1] = 3.;
    expected_trace[6] = testpar[2] = 2.;
    expected_trace[7] = g(testpar);
    log_info("so far so good 2a");
    I3VectorDoublePtr trace = g.GetTrace();

    log_info("so far so good 2b");
    ENSURE( (bool)trace, "that we get a non-NULL trace");
    log_info("so far so good 3");
    ENSURE( trace->size()==expsize, "that the trace has the expected length");
    log_info("so far so good 4");
    double eps = 0.0001;
    for (size_t i=0; i<expsize; i++){
        ENSURE_DISTANCE( expected_trace.at(i), trace->at(i),eps,
                         "that trace element has the expected value");
    }
    // ENSURE( expected_trace == *trace, "that the trace has the expected values");
    log_info("so far so good 5");

    g.Trace();
    log_info("so far so good 6");
    trace = g.GetTrace();
    log_info("so far so good 7");
    ENSURE( trace->empty(), "that the trace is properly reset");
    log_info("so far so good 8");

    g.NoTrace();
    log_info("so far so good 9");
    trace = g.GetTrace();
    log_info("so far so good 10");
    ENSURE( !trace, "that the trace is properly discarded");
    log_info("so far so good 11");

    // now the same with gradients
    I3MinimizerBasePtr
        dumb_mini_grad( new I3TestDummyMinimizer(0.001,0.85,1000,true) );
    I3Gulliver ggrad(dumb_param, dumb_llh, dumb_mini_grad, "GulliverFitWithGradient" );
    ggrad.UseGradients(true);

    log_info("so far so good 12");
    ggrad.Trace();
    log_info("so far so good 13");
    std::vector<double> testgrad(3);
    expsize = 14; // 2*(3+1)
    expected_trace.resize(expsize); // 2*(2*3+1)
    expected_trace[0] = testpar[0] = 7.;
    expected_trace[1] = testpar[1] = 6.;
    expected_trace[2] = testpar[2] = 5.;
    log_info("so far so good 13a");
    expected_trace[6] = ggrad(testpar,testgrad);
    log_info("so far so good 13b");
    expected_trace[3] = testgrad[0];
    expected_trace[4] = testgrad[1];
    expected_trace[5] = testgrad[2];
    expected_trace[7] = testpar[0] = 4.;
    expected_trace[8] = testpar[1] = 3.;
    expected_trace[9] = testpar[2] = 2.;
    log_info("so far so good 14a");
    expected_trace[13] = ggrad(testpar,testgrad);
    log_info("so far so good 14b");
    expected_trace[10] = testgrad[0];
    expected_trace[11] = testgrad[1];
    expected_trace[12] = testgrad[2];
    log_info("so far so good 14");
    trace = ggrad.GetTrace();
    log_info("so far so good 15");

    ENSURE( (bool)trace, "that we get a non-NULL trace");
    log_info("so far so good 16");
    ENSURE( trace->size()==expsize, "that the trace has the expected length");
    log_info("so far so good 17");
    for (size_t i=0; i<expsize; i++){
        ENSURE_DISTANCE( expected_trace.at(i), trace->at(i),eps,
                         "that trace element has the expected value");
    }
    // ENSURE( expected_trace == *trace, "that the trace has the expected values");

    log_info("so far so good 18");
    ggrad.Trace();
    log_info("so far so good 19");
    trace = ggrad.GetTrace();
    log_info("so far so good 20");
    ENSURE( trace->empty(), "that the trace is properly reset");
    log_info("so far so good 21");

    ggrad.NoTrace();
    log_info("so far so good 22");
    trace = ggrad.GetTrace();
    log_info("so far so good 23");
    ENSURE( !trace, "that the trace is properly discarded");
    log_info("so far so good 24");

}


namespace testing_stuff {
    class MyTestDirection{
        public:
        MyTestDirection(double z, double a, double t, bool i, std::string s):
            dir(z,a),tolerance(t),inrange(i),msg(s){}
        ~MyTestDirection(){}
        I3Direction dir;
        double tolerance;
        bool inrange;
        std::string msg;
    };
};

TEST(AnglesInRange){

    using namespace testing_stuff;

    std::vector<MyTestDirection> testdirs;
    testdirs.push_back(
            MyTestDirection(999.9,1003.9,0.001,true,"medium large angle") );
    testdirs.push_back(
            MyTestDirection(0.9,1.9,0.0001,true,"perfectly normal angle") );
    testdirs.push_back(
            MyTestDirection(-0.9,-1.9,0.0001,true,"normal size negative angle") );
    testdirs.push_back(
            MyTestDirection(-999.9,-1003.9,0.001,true,"medium large negative angle") );
    testdirs.push_back(
            MyTestDirection(999999.9,-999999.9,0.1,true,"huge angle") );
    std::string x = "x";
    std::string y = "y";
    std::string z = "z";
    std::string failmsg = "-coordinate of direction fixed incorrectly for ";
    std::vector<MyTestDirection>::iterator itest;
    for ( itest = testdirs.begin(); itest != testdirs.end(); ++itest ){
        double tolerance = itest->tolerance;
        I3Particle p;
        p.SetDir(itest->dir);
        bool fixed = I3Gulliver::AnglesInRange(p, itest->msg);
        if ( itest->inrange ){
            ENSURE( fixed,
                    itest->msg + ": angles in range check/fix failed");
        } else {
            ENSURE( !fixed,
                    itest->msg + ": angles in range check/fix failed to flag pathological angle");
            continue;
        }
        const I3Direction &fixed_dir = p.GetDir();
        ENSURE_DISTANCE( itest->dir.GetX(), fixed_dir.GetX(), tolerance,
                         x+failmsg+itest->msg );
        ENSURE_DISTANCE( itest->dir.GetY(), fixed_dir.GetY(), tolerance,
                         y+failmsg+itest->msg );
        ENSURE_DISTANCE( itest->dir.GetZ(), fixed_dir.GetZ(), tolerance,
                         z+failmsg+itest->msg );
    }
    I3Particle p;
    p.SetDir(NAN,NAN);
    bool fixed = I3Gulliver::AnglesInRange(p, "no direction (angles are NANs)");
    ENSURE( fixed, "AnglesInRange running on directionless particles should be a no-op.");
    ENSURE( std::isnan(p.GetZenith()), "directionless particles: zenith should remain NAN.");
    ENSURE( std::isnan(p.GetAzimuth()), "directionless particles: azimuth should remain NAN.");
}
