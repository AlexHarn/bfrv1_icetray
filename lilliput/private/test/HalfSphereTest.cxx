/**
    copyright  (C) 2007
    the icecube collaboration
    $Id$

    @version $Revision$
    @date $Date$
    @author boersma
*/

// generic stuff
#include <I3Test.h>
#include "icetray/I3Units.h"
#include "phys-services/I3Calculator.h"

// gulliver stuff
#include "gulliver/I3EventHypothesis.h"
#include "lilliput/parametrization/I3HalfSphereParametrization.h"


TEST_GROUP(HalfSphereTest);

TEST(DirTest)
{

    // only direction
    I3HalfSphereParametrization halfsphere(
            "test",
            1.0*I3Units::radian,
            0.0*I3Units::m,
            0.0,
            0.0*I3Units::ns );

    double zenith = 0.5*I3Units::radian;
    double azimuth = 0.4*I3Units::radian;

    // initialize with a vertical downgoing track
    I3EventHypothesisPtr eh(new I3EventHypothesis);
    eh->particle->SetDir(zenith,azimuth);
    halfsphere.SetHypothesisPtr(eh);

    // don't do anything, just get that pointer back and verify
    // that nothing changed.
    I3EventHypothesisPtr eh1 = halfsphere.GetHypothesisPtr();
    ENSURE_DISTANCE(eh1->particle->GetZenith(),zenith,0.0001,
                    "Zenith changed illegally");
    ENSURE_DISTANCE(eh1->particle->GetAzimuth(),azimuth,0.0001,
                    "Azimut changed illegally");
    // check also the original pointer.
    ENSURE_DISTANCE(eh->particle->GetZenith(),zenith,0.0001,
                    "Zenith changed illegally (using original pointer)");
    ENSURE_DISTANCE(eh->particle->GetAzimuth(),azimuth,0.0001,
                    "Azimut changed illegally (using original pointer)");

    // check parameters: should be (0,0)
    const std::vector<I3FitParameterInitSpecs> &specs = halfsphere.GetParInitSpecs(eh);
    ENSURE(specs.size()==2, "wrong number of parameters");
    ENSURE_DISTANCE(specs[0].initval_,0.,0.0001, "wrong initial parameter value");
    ENSURE_DISTANCE(specs[1].initval_,0.,0.0001, "wrong initial parameter value");

    // now change the angle by 45 degrees, by moving over a distance 1
    // in the projected plane. check various directions
    I3Particle startparticle(*(eh->particle));
    double diff8 = acos(0.25*sqrt(2.0)+0.5);
    for ( int i=0; i<8; ++i ){

        std::vector<double> newpar1(2);
        double azimuth = 2.0*i*M_PI/8.0;
        newpar1[0] = cos(azimuth);
        newpar1[1] = sin(azimuth);
        I3Particle p1( *( halfsphere.GetHypothesisPtr(newpar1)->particle ) );

        std::vector<double> newpar2(2);
        azimuth = 2.0*(i+1)*M_PI/8.0;
        newpar2[0] = cos(azimuth);
        newpar2[1] = sin(azimuth);
        I3Particle p2( *( halfsphere.GetHypothesisPtr(newpar2)->particle ) );

        ENSURE_DISTANCE( I3Calculator::Angle(startparticle, p1),
                         45*I3Units::degree, 0.0001,
                        "Wrong angle (1) wrt original particle "
                        "after changing parameters");

        ENSURE_DISTANCE( I3Calculator::Angle(startparticle, p2),
                         45*I3Units::degree, 0.0001,
                        "Wrong angle (2) wrt original particle "
                        "after changing parameters");

        ENSURE_DISTANCE( I3Calculator::Angle(p1, p2), diff8, 0.0001,
                        "Wrong angle (3) wrt original particle "
                        "after changing parameters");

    }

}
