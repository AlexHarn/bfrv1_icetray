/**
    copyright  (C) 2009
    the icecube collaboration
    $Id$

    @version $Revision$
    @date $Date$
    @author boersma
*/

#include <I3Test.h>

#include "icetray/I3Logging.h"
#include <string>
#include <sstream>
#include <cmath>
#include <cstdlib>

// #include "TestFrame.h"
#include "double-muon/I3ResponseMapSplitter.h"
#include "dataclasses/physics/I3RecoPulse.h"
#include "phys-services/I3Calculator.h"

// the tests in this source file all need a pulseseriesmap to split
// a geometry and some output pulseseriesmaps
// for convenience, we just generate DOMs on a rectangular lattice

class FissionMaterial {
public:
    FissionMaterial(int nx=10, int ny=10, int nz=60 ):
      geo(new I3Geometry),
      startpulses( new I3RecoPulseSeriesMap ),
      half1( new I3RecoPulseSeriesMap ),
      half2( new I3RecoPulseSeriesMap ){
        for ( int ix = 1; ix<=nx; ++ix ){
            for ( int iy = 0; iy<ny; ++iy ){
                for ( int iz = 1; iz<=nz; ++iz ){
                    OMKey om(ix+nx*iy,iz);
                    I3OMGeo omgeo;
                    double x = 100.*(ix-0.5*nx);
                    double y = 100.*(iy-0.5*ny);
                    double z = 16.*(iz-0.5*nz);
		    omgeo.position = I3Position(x,y,z);
                    omgeo.orientation = I3Orientation(0.,0.,-1., 1.,0.,0.);
                    omgeo.area = M_PI*5*5*2.54*2.54*I3Units::cm*I3Units::cm;
                    omgeo.omtype = I3OMGeo::IceCube;
                    geo->omgeo[om] = omgeo;
                }
            }
        }
    }

    I3GeometryPtr geo;
    I3RecoPulseSeriesMapPtr startpulses;
    I3RecoPulseSeriesMapPtr half1;
    I3RecoPulseSeriesMapPtr half2;
};

TEST_GROUP(TestResponseMapSplitter);

TEST(CheckTimeSplitting)
{
    FissionMaterial test10(10,10,60);
    FissionMaterial test1(10,10,60);
    OMKey om1(50,30);
    // create 2 fake events, each with 10 pulses
    // test10: each pulse in a different DOM
    // test1: all pulses in the same DOM
    for ( int ipulse = 0; ipulse<10; ++ipulse ){
        I3RecoPulse pulse;
        pulse.SetTime(10.*ipulse*I3Units::ns);
        pulse.SetCharge(1.0);
        pulse.SetWidth(20.*I3Units::ns);
        OMKey om10(1+ipulse*11,1+ipulse*5);
        I3RecoPulseSeries &ompulses10 = (*(test10.startpulses))[om10];
        ompulses10.push_back(pulse);
        I3RecoPulseSeries &ompulses1 = (*(test1.startpulses))[om1];
        ompulses1.push_back(pulse);
    }
    std::string name = "unittest";
    unsigned int n1=0,n2=0;

    // first test case all pulses in seperate DOMs
    I3ResponseMapSplitter::SplitByTime<I3RecoPulse>(
            test10.startpulses,
            test10.half1, test10.half2,
            n1, n2, false, I3ResponseMapSplitter::SW_OLD, name );

    ENSURE( test10.half1->size() == 5, "first half should have 5 hit DOMs");
    for ( int ipulse = 0; ipulse<5; ++ipulse ){
        OMKey om(1+ipulse*11,1+ipulse*5);
        I3RecoPulseSeriesMap::iterator iom = test10.half1->find(om);
        ENSURE(iom != test10.half1->end(),"missing OM in first half");
        ENSURE(iom->second.size() == 1,"missing pulse");
        ENSURE_DISTANCE(iom->second[0].GetTime(),10.*ipulse*I3Units::ns,0.001,
                        "wrong pulse time");
    }

    ENSURE( test10.half2->size() == 5, "second half should have 5 hit DOMs");
    for ( int ipulse = 5; ipulse<10; ++ipulse ){
        OMKey om(1+ipulse*11,1+ipulse*5);
        I3RecoPulseSeriesMap::iterator iom = test10.half2->find(om);
        ENSURE(iom != test10.half2->end(),"missing OM in second half");
        ENSURE(iom->second.size() == 1,"missing pulse");
        ENSURE_DISTANCE(iom->second[0].GetTime(),10.*ipulse*I3Units::ns,0.001,
                        "wrong pulse time");
    }

    // second test case all pulses in the same DOM
    n1=n2=0;
    I3ResponseMapSplitter::SplitByTime<I3RecoPulse>(
            test1.startpulses,
            test1.half1, test1.half2,
            n1, n2, false, I3ResponseMapSplitter::SW_OLD, name );

    ENSURE( n1 == 5, "first half should have 5 pulses");
    ENSURE( test1.half1->size() == 1, "first half should have 1 hit DOM");
    for ( int ipulse = 0; ipulse<5; ++ipulse ){
        I3RecoPulseSeriesMap::iterator iom = test1.half1->find(om1);
        ENSURE(iom != test1.half1->end(),"missing OM in first half");
        ENSURE(iom->second.size() == 5,"missing or extra pulses");
        ENSURE_DISTANCE(iom->second[ipulse].GetTime(),
                        10.*ipulse*I3Units::ns,0.001,
                        "wrong pulse time");
    }

    ENSURE( n2 == 5, "second half should have 5 pulses");
    ENSURE( test1.half2->size() == 1, "second half should have 1 hit DOM");
    for ( int ipulse = 5; ipulse<10; ++ipulse ){
        I3RecoPulseSeriesMap::iterator iom = test1.half2->find(om1);
        ENSURE(iom != test1.half2->end(),"missing OM in second half");
        ENSURE(iom->second.size() == 5,"missing or extra pulses");
        ENSURE_DISTANCE(iom->second[ipulse-5].GetTime(),
                        10.*ipulse*I3Units::ns,0.001,
                        "wrong pulse time");
    }

}

void TestTimeSplit(FissionMaterial& fm,
                   bool median,
                   I3ResponseMapSplitter::splitweight_t sw,
                   double t_true, unsigned int n1_true, unsigned int n2_true,
                   std::string name){
    unsigned int n1 = 0, n2 = 0;
    double tsmall = 0.00001;
    double t_split = I3ResponseMapSplitter::SplitByTime<I3RecoPulse>(
            fm.startpulses, fm.half1, fm.half2, n1, n2, median, sw, name );
    log_info("testing split %s", name.c_str());
    ENSURE_DISTANCE(t_true, t_split, tsmall, "split time wrong");
    ENSURE_EQUAL(n1_true, n1, "wrong number of pulses in first half");
    ENSURE_EQUAL(n2_true, n2, "wrong number of pulses in second half");

}


TEST(CheckNewTimeSplitting)
{
    FissionMaterial thorium(2,3,2);
    OMKey om1(1,1);
    OMKey om2(1,2);
    I3RecoPulseSeries &pseries1 = (*(thorium.startpulses))[om1];
    I3RecoPulseSeries &pseries2 = (*(thorium.startpulses))[om2];

    // 10 pulses with q=10PE in OM1
    for ( int ipulse = 0; ipulse<10; ++ipulse ){
        I3RecoPulse pulse;
        pulse.SetWidth(2.*I3Units::ns); // dummy
        pulse.SetTime(10.*ipulse*I3Units::ns);
        pulse.SetCharge(10.);
        pseries1.push_back(pulse);
    }
    // 5 pulses with q=2PE pulses in OM2
    for ( int ipulse = 0; ipulse<5; ++ipulse ){
        I3RecoPulse pulse;
        pulse.SetTime(100.1+10.*ipulse*I3Units::ns);
        pulse.SetCharge(2.0);
        pseries2.push_back(pulse);
    }

    double true_tmean_old = (70.+0.5/15)*I3Units::ns;
    double true_tmean_charge = 5701.0*I3Units::ns/110.; // = 51.82727 = (10.*(0+10+20+30+40+50+60+70+80+90)+2.*(100.1+110.1+120.1+130.1+140.1))/(10*10.+5*2.);
    double true_tmean_dom = (0.+100.1)*I3Units::ns/2;
    double true_tmedian_old = 0.5*(10.*6+10.*7)*I3Units::ns; // tsplit between 7th and 8th pulse
    double true_tmedian_charge = 0.5*(10.*4+10.*5.)*I3Units::ns; // qtot=110.5, qmed=55.25, tsplit between 5th and 6th)
    double true_tmedian_dom = 0.5*(0+100.1)*I3Units::ns; // tsplit in between time of first pulses

    I3ResponseMapSplitter::splitweight_t sw_old = I3ResponseMapSplitter::SW_OLD;
    I3ResponseMapSplitter::splitweight_t sw_charge = I3ResponseMapSplitter::SW_Charge;
    I3ResponseMapSplitter::splitweight_t sw_dom = I3ResponseMapSplitter::SW_DOM;

    TestTimeSplit(thorium, false, sw_old, true_tmean_old, 8, 7, "(old) unweighted mean of pulse times");
    TestTimeSplit(thorium, false, sw_charge, true_tmean_charge, 6, 9, "charge-weighted mean");
    TestTimeSplit(thorium, false, sw_dom, true_tmean_dom, 6, 9, "mean using only first pulses per DOM, unweighted");
    TestTimeSplit(thorium, true, sw_old, true_tmedian_old, 7, 8, "(old) unweighted median of pulse times");
    TestTimeSplit(thorium, true, sw_charge, true_tmedian_charge, 5, 10, "charge-weighted median of pulse times");
    TestTimeSplit(thorium, true, sw_dom, true_tmedian_dom, 6, 9, "median using only first pulses per DOM, unweighted");

}

// geo test:
// put a bunch of pulses in string 1
// put a bunch of pulses in string 100
// create a track that "points" from one to the other
// geo split 
// first half should only contain pulses in string 1
// second half should only contain pulses in string 100
TEST(CheckGeoSplitting)
{
    FissionMaterial test(10,10,60);
    I3ParticlePtr track(new I3Particle(
                I3Particle::InfiniteTrack, I3Particle::MuMinus ) );
    track->SetFitStatus(I3Particle::OK);
    track->SetPos( 0.0, 0.0, 0.0 );
    track->SetDir( cos(45*I3Units::degree),sin(45*I3Units::degree), 0. );
    track->SetTime(0.);
    track->SetEnergy(1.*I3Units::TeV);
    for ( int ipulse = 0; ipulse<10; ++ipulse ){
        OMKey om((ipulse&1)?100:1,1+ipulse*5);
        I3RecoPulseSeries &ompulses = (*(test.startpulses))[om];
        I3RecoPulse pulse;
        pulse.SetTime( (ipulse&1) ? 1000.*I3Units::ns : 0.);
        pulse.SetCharge(1.0);
        pulse.SetWidth(20.*I3Units::ns);
        ompulses.push_back(pulse);
    }
    std::string name = "unittest";
    unsigned int n1=0,n2=0;
    I3ResponseMapSplitter::SplitByTrackAndCOG<I3RecoPulse>(
            track,
            test.geo,
            test.startpulses,
            test.half1, test.half2,
            n1, n2, name );

    I3RecoPulseSeriesMap::iterator imap;

    ENSURE( test.half1->size() == 5, "first half should have 5 hit DOMs");
    for ( imap = test.half1->begin(); imap != test.half1->end(); ++imap ){
        ENSURE(imap->first.GetString() == 1,"should be on string 1");
        ENSURE(imap->second.size() == 1,"should have exactly 1 pulse");
    }

    ENSURE( test.half2->size() == 5, "second half should have 5 hit DOMs");
    for ( imap = test.half2->begin(); imap != test.half2->end(); ++imap ){
        ENSURE(imap->first.GetString() == 100,"should be on string 100");
        ENSURE(imap->second.size() == 1,"should have exactly 1 pulse");
    }

}

TEST(CheckTresSplitting)
{
    unsigned int nx=5,ny=5,nz=10;
    FissionMaterial testTRES(nx,ny,nz);
    I3ParticlePtr track(new I3Particle(
                I3Particle::InfiniteTrack, I3Particle::MuMinus ) );
    track->SetFitStatus(I3Particle::OK);
    track->SetPos( 0.,0.,0. ); // easy position
    track->SetDir( 0.8*I3Units::radian, 0.3*I3Units::radian ); // arb. dir.
    track->SetTime(0.); // convenient for later
    track->SetEnergy(1.*I3Units::TeV); // arb. energy
    double dt = 1.0*I3Units::ns;
    double tres0 = -30.0*I3Units::ns;
    unsigned int nearly = 20;
    unsigned int ngood = 40;
    unsigned int nlate = 40;
    unsigned int npulses = nearly + ngood + nlate;
    double minTRES = tres0 + nearly*dt;
    double maxTRES = minTRES + ngood*dt;
    srand48(123457);
    for ( unsigned int ipulse = 0; ipulse<npulses; ++ipulse ){
        OMKey om(1+(lrand48()%(nx*ny)),1+(lrand48()%nz));
        I3RecoPulseSeries &ompulses = (*(testTRES.startpulses))[om];
        const I3Position &ompos = testTRES.geo->omgeo[om].position;
        I3RecoPulse pulse;
        double tres = tres0 + (ipulse+0.5)*dt;
        pulse.SetTime( tres - I3Calculator::TimeResidual(*track,ompos,0.0) );
        pulse.SetCharge(1.0);
        pulse.SetWidth(20.*I3Units::ns);
        ompulses.push_back(pulse);
    }
    std::string name = "unittest";
    unsigned int n1=0,n2=0;
    // I3ResponseMapSplitter::SplitByTimeResidual(
    I3ResponseMapSplitter::SplitByTimeResidual<I3RecoPulse>(
            minTRES,maxTRES,
            track,
            testTRES.geo,
            testTRES.startpulses,
            testTRES.half1, testTRES.half2,
            n1, n2, name );
   
    char message[256];
    sprintf(message, "n1+n2=%u+%u != npulses=%u",
                     n1,n2,npulses);
    ENSURE( n1+n2 == npulses, message );
    sprintf(message, "got n1=%u, expected %u nch=%zu nch1=%zu nch2=%zu",
                     n1,ngood,
                     testTRES.startpulses->size(),
                     testTRES.half1->size(),testTRES.half2->size());
    ENSURE( n1 == ngood, message );
    sprintf(message, "got n2=%u, expected %u",n2,npulses-ngood);
    ENSURE( n2 == npulses-ngood, message );
}

TEST(CheckBrightStSplitting)
{
    unsigned int nx=20,ny=20,nz=10;
    FissionMaterial Brightest(nx,ny,nz);
    srand48(123459);
    int superstring = 1+lrand48()%(nx*ny);
    unsigned n2pulses = 500;
    for ( unsigned int ipulse = 0; ipulse<2*n2pulses; ipulse+=2 ){
        OMKey superOM(superstring,1+(lrand48()%nz));
        int randstring = 1+(lrand48()%(nx*ny-1)); 
        if ( randstring >= superstring ) ++randstring;
        OMKey randOM(randstring,1+(lrand48()%nz));
        I3RecoPulse pulse;
        pulse.SetTime( 5. );
        pulse.SetCharge(1.0);
        pulse.SetWidth(20.*I3Units::ns);
        I3RecoPulseSeries &randOMpulses = (*(Brightest.startpulses))[randOM];
        randOMpulses.push_back(pulse);
        I3RecoPulseSeries &superOMpulses = (*(Brightest.startpulses))[superOM];
        superOMpulses.push_back(pulse);
    }
    std::string name = "unittest";
    unsigned int n1=0;
    unsigned int n2=0;

    // TEST 1: brightness split with very SHORT include distance
    // ==> take only pulses on the brightest string into 1st map, rest in 2nd
    // ==> i.e. n2pulses=500 in half1 and also n2pulses=500 in half2
    double Dshort=0.5*I3Units::m;
    I3ResponseMapSplitter::SplitByBrightness<I3RecoPulse>(
            Dshort,
            Brightest.geo,
            Brightest.startpulses,
            Brightest.half1, Brightest.half2,
            n1, n2, name );
    char message[256];
    sprintf(message, "n1+n2=%u+%u != npulses=%u",
                     n1,n2,2*n2pulses);
    ENSURE( n1+n2 == 2*n2pulses, message );
    sprintf(message, "got n1=%u, expected %u nch=%zu nch1=%zu nch2=%zu",
                     n1,n2pulses,
                     Brightest.startpulses->size(),
                     Brightest.half1->size(),Brightest.half2->size());
    ENSURE( n1 == n2pulses, message );
    sprintf(message, "got n2=%u, expected %u",n2,n2pulses);
    ENSURE( n2 == n2pulses, message );

    // TEST 2: brightness split with very LONG include distance
    // ==> take all pulses into 1st map, nothing in 2nd
    // ==> i.e. 2*n2pulses=2*500 in half1 and zero=0 in half2
    double Dlong=100000.0*I3Units::m;
    I3ResponseMapSplitter::SplitByBrightness<I3RecoPulse>(
            Dlong,
            Brightest.geo,
            Brightest.startpulses,
            Brightest.half1, Brightest.half2,
            n1, n2, name );
    sprintf(message, "n1+n2=%u+%u != npulses=%u",
                     n1,n2,2*n2pulses);
    ENSURE( n1+n2 == 2*n2pulses, message );
    sprintf(message, "got n1=%u, expected %u nch=%zu nch1=%zu nch2=%zu",
                     n1,2*n2pulses,
                     Brightest.startpulses->size(),
                     Brightest.half1->size(),Brightest.half2->size());
    ENSURE( n1 == 2*n2pulses, message );
    sprintf(message, "got n2=%u, expected %u",n2,0);
    ENSURE( n2 == 0, message );

}
