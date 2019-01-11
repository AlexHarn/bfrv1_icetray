/**
    copyright  (C) 2006
    the icecube collaboration
    $Id$

    @version $Revision$
    @date $Date$
    @author boersma
*/

extern "C" {
double rdmc_pt_lgtd_patched(double delay, double perp_dist, double cs_ori);
}

#include <iostream>

#include "I3Test.h"
#include "icetray/I3Units.h"
#include "ipdf/Likelihood/SPE1st.h"
#include "ipdf/Pandel/UPatchPEP.h"
#include "ipdf/AllOMsLikelihood.h"
#include "lilliput/likelihood/I3GulliverIPDFPandel.h"

#include "TestFrame.h"

using IPDF::Pandel::UPatchPEP;
using IPDF::Pandel::H2;
using IPDF::AllOMsLikelihood;
using IPDF::Likelihood::SPE1st;

TEST_GROUP(PandelTest);

TEST(SingleHitTest)
{
    std::string pulses = "testpulses";
    double absorptionlength = 98.0*I3Units::m;
    double jitter = 15.0*I3Units::ns;
    double noiserate = 1000.0*I3Units::hertz;

    // for IPDF, the absorption is fixed in the icemodel (here: H2)
    // for IPDF, there is no noise rate
    boost::shared_ptr< UPatchPEP<H2> >
        upatch( new UPatchPEP<H2>( boost::shared_ptr< H2 >( new H2 ), jitter ) );
    boost::shared_ptr< AllOMsLikelihood< SPE1st< UPatchPEP<H2> > > >
        spe1st ( new AllOMsLikelihood< SPE1st< UPatchPEP<H2> > >( *new SPE1st<UPatchPEP<H2> >( upatch ) ) );
    I3EventLogLikelihoodBasePtr ipdfpandel(
            new I3GulliverIPDFPandel< AllOMsLikelihood< SPE1st< UPatchPEP<H2> > >, IPDF::InfiniteMuon >( "testipdfpandel",
                spe1st, pulses ) );

    // make a hit at an OM for which a direct photon would have been
    // emitted 30m from the vertex on and go 55m through ice.
    // choose time delay of 10ns.
    double tvertex = 1000*I3Units::ns;
    double perpdist = 12.0*I3Units::m;
    double tdelay = 5*I3Units::ns;
    double chpoint = 1.0*I3Units::m;
    double chazi = 0;

    TestFrame testframe( pulses );
    I3EventHypothesis hypo;
    hypo.particle->SetPos( 0, 0, 0 );
    hypo.particle->SetDir( 0, 0, 1 );
    hypo.particle->SetTime( tvertex );
    hypo.particle->SetType( I3Particle::MuMinus );
    hypo.particle->SetShape( I3Particle::InfiniteTrack );
    double amplitude = 5.0; // PE
    double width = 30*I3Units::ns;
    double cs_ori = testframe.AddPulse( hypo.particle, tdelay, perpdist,
                                        chpoint, chazi, amplitude, width );
    double expected_llh =
        rdmc_pt_lgtd_patched( tdelay/I3Units::ns, perpdist/I3Units::m, cs_ori );
    char message[128];
    sprintf( message,"dist=%f tdelay=%f cs_ori=%f llh=%f",
                     perpdist/I3Units::m,tdelay/I3Units::ns, cs_ori,expected_llh );

    const I3Geometry& geo = testframe.Get<I3Geometry>();
    ipdfpandel->SetGeometry(geo);
    ipdfpandel->SetEvent(testframe);

    double llh_ipdf = ipdfpandel->GetLogLikelihood( hypo );
    ENSURE_DISTANCE(llh_ipdf,expected_llh,0.0001,message);
}
