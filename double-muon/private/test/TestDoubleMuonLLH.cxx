/**
    copyright  (C) 2008
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

#include "TestFrame.h"
#include "double-muon/I3DoubleMuonLogLikelihood.h"

#include "lilliput/likelihood/I3GulliverIPDFPandel.h"
#include "ipdf/Likelihood/SPE1st.h"
#include "ipdf/Likelihood/MPE.h"
#include "ipdf/Pandel/IceModel.h"
#include "ipdf/Pandel/UPatchPEP.h"
#include "ipdf/AllOMsLikelihoodWithConstNoise.h"
#include "ipdf/Hypotheses/InfiniteMuon.h"
using IPDF::Pandel::GaussConvolutedPEP;
using IPDF::Pandel::H2;
using IPDF::Likelihood::SPE1st;
using IPDF::Likelihood::MPE;
using IPDF::InfiniteMuon;
using IPDF::AllOMsLikelihoodWithConstNoise;


TEST_GROUP(TestDoubleMuonLLH);

/**
 * Create two identical tracks
 * Check that the likelihood for this is the same as for a single track
 * Variation: second track is very far away
 */
TEST(CheckLLHOnTop)
{
    // make a frame, geometry with 4x4 strings, 30 DOMs per string
    const std::string pulses = "testpulses";
    const std::string hits = "testhits";
    TestFramePtr fptr( new TestFrame(pulses,hits,16,30));
    const I3Geometry& geo = fptr->Get<I3Geometry>();

    // make some random track 1
    I3ParticlePtr mu1(new I3Particle(I3Particle::InfiniteTrack,I3Particle::MuPlus));
    mu1->SetPos(10*I3Units::m,20*I3Units::m,30*I3Units::m);
    mu1->SetDir(10*I3Units::degree,20*I3Units::degree);
    mu1->SetEnergy(2*I3Units::TeV);
    mu1->SetTime(10*I3Units::microsecond);
    I3EventHypothesis ehsingle( mu1, I3FrameObjectPtr() );

    // make track 2 *identical* to track 1
    I3ParticlePtr mu2(new I3Particle(*mu1));
    I3EventHypothesis ehdouble(mu1,mu2);

    // make track 3 same to track 1, translated by a BIG distance
    I3ParticlePtr mu3(new I3Particle(*mu1));
    mu3->SetPos(10*I3Units::km,20*I3Units::km,30*I3Units::km);
    I3EventHypothesis ehdoubleFAR(mu1,mu3);

    // create a fake event with 30 pulses
    // with reasonable time resuduals w.r.t. track 1
    fptr->AddPulses( mu1, 30 );

    // single track likelihood (SPE, MPE)
    double jitter = 10.*I3Units::ns;
    double noiserate = 700.*I3Units::hertz;
    boost::shared_ptr< GaussConvolutedPEP<H2> >
        gconv( new GaussConvolutedPEP<H2>( boost::shared_ptr< H2 >( new H2 ), jitter ) );
    boost::shared_ptr< AllOMsLikelihoodWithConstNoise< SPE1st< GaussConvolutedPEP<H2> > > >
        spe1st ( new AllOMsLikelihoodWithConstNoise< SPE1st< GaussConvolutedPEP<H2> > >(
                    *new SPE1st<GaussConvolutedPEP<H2> >( gconv ), noiserate ) );
    I3EventLogLikelihoodBasePtr singleSPE(
            new I3GulliverIPDFPandel< AllOMsLikelihoodWithConstNoise< SPE1st< GaussConvolutedPEP<H2> > >, IPDF::InfiniteMuon >(
                "singleSPE", spe1st, pulses ) );
    singleSPE->SetGeometry(geo);
    singleSPE->SetEvent(*fptr);
    boost::shared_ptr< AllOMsLikelihoodWithConstNoise< MPE< GaussConvolutedPEP<H2> > > >
        mpe ( new AllOMsLikelihoodWithConstNoise< MPE< GaussConvolutedPEP<H2> > >(
                    *new MPE<GaussConvolutedPEP<H2> >( gconv ), noiserate ) );
    I3EventLogLikelihoodBasePtr singleMPE(
            new I3GulliverIPDFPandel< AllOMsLikelihoodWithConstNoise< MPE< GaussConvolutedPEP<H2> > >, IPDF::InfiniteMuon >(
                "singleMPE", mpe, pulses ) );
    singleMPE->SetGeometry(geo);
    singleMPE->SetEvent(*fptr);

    // double track likelihood (SPE, MPE)
    boost::shared_ptr< I3DoubleMuonLogLikelihood >
        doubleSPE( new I3DoubleMuonLogLikelihood(
                    "doubleSPE", pulses, "", noiserate, false ));
    doubleSPE->SetGeometry(geo);
    doubleSPE->SetEvent(*fptr);
    boost::shared_ptr< I3DoubleMuonLogLikelihood >
        doubleMPE( new I3DoubleMuonLogLikelihood(
                    "doubleMPE", pulses, "", noiserate, true ));
    doubleMPE->SetGeometry(geo);
    doubleMPE->SetEvent(*fptr);

    double llh1spe = -singleSPE->GetLogLikelihood(ehsingle);
    double llh1mpe = -singleMPE->GetLogLikelihood(ehsingle);
    double llh2spe = -doubleSPE->GetLogLikelihood(ehdouble);
    double llh2mpe = -doubleMPE->GetLogLikelihood(ehdouble);
    double llh2speFAR = -doubleSPE->GetLogLikelihood(ehdoubleFAR);
    double llh2mpeFAR = -doubleMPE->GetLogLikelihood(ehdoubleFAR);

    // silly checks
    ENSURE(std::isfinite(llh1spe)&&(llh1spe>0),
            "single SPE -log(L) should be finite and positive");
    ENSURE(std::isfinite(llh1mpe)&&(llh1mpe>0),
            "single MPE -log(L) should be finite and positive");
    ENSURE(std::isfinite(llh2spe)&&(llh2spe>0),
            "double SPE -log(L) (2 id. tracks) should be finite and positive");
    ENSURE(std::isfinite(llh2mpe)&&(llh2mpe>0),
            "double MPE -log(L) (2 id. tracks) should be finite and positive");
    ENSURE(std::isfinite(llh2speFAR)&&(llh2speFAR>0),
            "double SPE -log(L) (FAR track) should be finite and positive");
    ENSURE(std::isfinite(llh2mpeFAR)&&(llh2mpeFAR>0),
            "double MPE -log(L) (FAR track) should be finite and positive");

    // substantial checks
    ENSURE_DISTANCE( llh1spe,llh2spe,0.001*llh1spe,
            "SPE llh for 2 identical tracks should be equal to single track");
    ENSURE_DISTANCE( llh1mpe,llh2mpe,0.001*llh1mpe,
            "MPE llh for 2 identical tracks should be equal to single track");
    ENSURE_DISTANCE( llh1spe,llh2speFAR,0.001*llh1spe,
            "SPE llh for 1 track + very distant track should be equal to single track" );
    ENSURE_DISTANCE( llh1mpe,llh2mpeFAR,0.001*llh1mpe,
            "MPE llh for 1 track + very distant track should be equal to single track" );

}
