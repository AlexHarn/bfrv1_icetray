/**
 *@file
 *@brief Test lilliput wrapper of photonics-service in icetray framework using fake version of photonics.
 *
 * This test is mostly copied from photonics-service test, by Klas Hultqvist).
 *
 *@author David Boersma
 *@version $Id$
 *@date $Date$
 *(c) the IceCube Collaboration
 */

#include <I3Test.h>

#include "icetray/test/ConstructorTest.h"
#include "icetray/I3Tray.h"

#include "TestFrame.h"
#include "PSSTestModule.h"
#include "lilliput/likelihood/I3GulliverIPDFPandel.h"
#include "lilliput/likelihood/I3GulliverIPDFPandelFactory.h"

#include "boost/filesystem.hpp"
namespace bff = boost::filesystem;

TEST_GROUP(PhotoSplineTests);

TEST(icetray_inspect){
    clean_constructor_test<I3GulliverIPDFFactory>();
}

static const int N_PHYSICS_FRAMES(100);
static const int N_OTHER_FRAMES(3);
static const int NFRAMES(N_PHYSICS_FRAMES + N_OTHER_FRAMES);
static const std::string pulses = "bogopulses";
static const std::string track = "bogotrack";
static const std::string photonics = "PhotonicsService";

TEST(icetray){
    I3Tray tray;

    ENSURE(getenv("I3_TESTDATA")!=NULL,"I3_TESTDATA is not defined! It must be...");
    std::string I3_TESTDATA(getenv("I3_TESTDATA"));

    std::string absfile =  I3_TESTDATA+"/photospline/ems_z0_a0.pt.abs.fits";
    std::string probfile =  I3_TESTDATA+"/photospline/ems_z0_a0.pt.prob.fits";

    ENSURE( bff::exists(bff::path(absfile)), "abs fits file exists");
    ENSURE( bff::exists(bff::path(probfile)), "prob fits file exists");

    tray.AddModule("I3TestFrameSource","bogodata")
        ("PulsesName",pulses)
        ("TrackName",track);

    // Make new P frames
    // tray.AddModule("I3NullSplitter", "nullsplit");

    tray.AddService("I3PhotoSplineServiceFactory", photonics)
        ("AmplitudeTable",absfile)
        ("TimingTable",probfile)
        ("TimingSigma", 0.0);

    tray.AddService("I3GulliverIPDFFactory", "PhSplineSPE1st")
        ("InputReadout", pulses)
        ("Likelihood", "SPE1st")
        ("PEProb", photonics)
        ("NoiseProbability", 10000.0*I3Units::hertz );

    tray.AddService("I3GulliverIPDFFactory", "PhSplineSPEAll")
        ("InputReadout", pulses)
        ("Likelihood", "SPEAll")
        ("PEProb", photonics)
        ("NoiseProbability", 10000.0*I3Units::hertz );

    tray.AddService("I3GulliverIPDFFactory", "PhSplineMPE")
        ("InputReadout", pulses)
        ("Likelihood", "MPE")
        ("PEProb", photonics)
        ("NoiseProbability", 10000.0*I3Units::hertz );

    tray.AddModule("PSSTestModule", "testSPE1st")
        ("IPDFName","PhSplineSPE1st")
        ("TrackName",track);

    tray.AddModule("PSSTestModule", "testSPEAll")
        ("IPDFName","PhSplineSPEAll")
        ("TrackName",track);

    tray.AddModule("PSSTestModule", "testMPE")
        ("IPDFName","PhSplineMPE")
        ("TrackName",track);

    

    tray.Execute(NFRAMES);
    
}
