/**
    copyright  (C) 2008
    the icecube collaboration
    $Id$

    @version $Revision$
    @date $Date$
    @author D.J. Boersma
*/

#include <sstream>
#include <iomanip>

#include <I3Test.h>
#include "ipdf/Hypotheses/InfiniteMuon.h"
#include "ipdf/Hypotheses/PointCascade.h"
#include "ipdf/Hypotheses/DirectionalCascade.h"
#include "ipdf/Simple/SimplePEHit.h"
#include "ipdf/Simple/SimpleHitOm.h"
#include "ipdf/Simple/SimpleOmReceiver.h"
#include "ipdf/Pandel/IceModel.h"
#include "ipdf/Pandel/UnConvolutedPEP.h"
#include "ipdf/Pandel/GSemiConvolutePEP.h"
#include "ipdf/Pandel/GaussConvolutedPEP.h"
#include "ipdf/Likelihood/MPE.h"

using std::cout;
using std::endl;

using IPDF::InfiniteMuon;
using IPDF::PointCascade;
using IPDF::DirectionalCascade;
using IPDF::SimplePEHit;
using IPDF::SimpleHitOm;
using IPDF::SimpleOmReceiver;
using IPDF::PEPBase;

using namespace IPDF::Pandel;

typedef UnConvolutedPEP<H2> PandelPlain;
typedef GSemiConvolutePEP<H2> PandelGSemi;
typedef GaussConvolutedPEP<H2> PandelGauss;

// typedef IPDF::PEPBase<UnConvolutedPEP<H2> > PandelPlainBase;
// typedef IPDF::PEPBase<GSemiConvolutePEP<H2> > PandelGSemiBase;
// typedef IPDF::PEPBase<GaussConvolutedPEP<H2> > PandelGaussBase;

template<typename PEProb, typename EmissionHypothesis>
inline void DoMpePandelFiniteTests(I3ParticlePtr p, boost::shared_ptr<PEProb> spp ){
    EmissionHypothesis eh(p);
    IPDF::Likelihood::MPE< PEProb > mpe(spp);
    for ( double dist=0.1; dist<5000; dist *= 2.0 ){
        SimpleOmReceiver omr(0,dist,0,-1.,4.,1.3);
        typename EmissionHypothesis::template EmissionGeometry<H2> eGeom(eh,omr);
        double tGeo = eGeom.geometricalTime();
        for ( double t=0.1; t<10000; t *= 2.0 ){
            std::vector<SimplePEHit*> hPlus( 1, new SimplePEHit(omr,tGeo+t) );
            std::vector<SimplePEHit*> hMinus( 1, new SimplePEHit(omr,tGeo-t) );
            SimpleHitOm omPlus(omr,hPlus);
            SimpleHitOm omMinus(omr,hMinus);
            double mpeplus = mpe.getLikelihood(omPlus,eh);
            double mpeminus = mpe.getLikelihood(omMinus,eh);
            std::ostringstream errmsg("not finite");
            // errmsg << " for particle=" << eh;
            errmsg << ", dist=" << dist << " and tres=";
            std::ostringstream errplus;
            std::ostringstream errminus;
            errplus  << errmsg.str() << double(+t) << "ns" << std::endl;
            errminus << errmsg.str() << double(-t) << "ns" << std::endl;
            ENSURE(finite(mpeplus),errplus.str());
            ENSURE(finite(mpeminus),errminus.str());
            // delete hPlus[0];
            // delete hMinus[0];
        }
    }
}


template<typename PEProb, typename EmissionHypothesis>
inline void DoPandelIntegralTests(
        I3ParticlePtr p,
        PEProb &spp,
        double tolerance,
        double mindist=10., double mintres=5.0 ){
    EmissionHypothesis eh(p);

    for ( double dist=mindist; dist<641.; dist *= 2.0 ){
        SimpleOmReceiver omr(0,dist,0,-1.,4.,1.3);
        typename EmissionHypothesis::template EmissionGeometry<H2> eGeom(eh,omr);
        double tGeo = eGeom.geometricalTime();

        // check extremes
        double epsilon = 1.e-7;
        SimplePEHit plusinf(  omr, tGeo+1.e6 );
        std::ostringstream errplus;
        errplus << spp;
        errplus << " integral of tres from -inf to +inf should give 1.0 "
                << "at d=" << dist;
        ENSURE_DISTANCE(spp.getIntPdf(plusinf,eh),1.0,epsilon,errplus.str());
        SimplePEHit mininf( omr, tGeo-1.e3 );
        std::ostringstream errminus;
        errminus << spp;
        errminus << "integral -inf to -1us should give 0.0 at d=" << dist;
        ENSURE_DISTANCE(spp.getIntPdf(mininf,eh),0.0,epsilon,errminus.str());

        // check "normal" values
        for ( double t=mintres; t<1000.; t *= 2.0 ){
            SimplePEHit hPlus(  omr, tGeo+t );
            SimplePEHit hMinus( omr, tGeo-t );
            double intplus = spp.getIntPdf(hPlus,eh);
            double intminus = spp.getIntPdf(hMinus,eh);
            double numintplus = spp.PEPBase<PEProb>::getIntPdf(hPlus,eh);
            double numintminus = spp.PEPBase<PEProb>::getIntPdf(hMinus,eh);
            std::ostringstream errmsg("integral differs from numerical int");
            errmsg << " for particle=" << eh;
            errmsg << ", dist=" << dist << " and tres=";
            std::ostringstream errplus;
            std::ostringstream errminus;
            errplus << errmsg.str() << "+" << t;
            errminus << errmsg.str() << "-" << t;
            errplus << " intpdf=" << intplus;
            errminus << " intpdf=" << intminus;
            errplus << " numintpdf=" << numintplus << std::endl;
            errminus << " numintpdf=" << numintminus << std::endl;
            ENSURE_DISTANCE(intplus,numintplus,tolerance,errplus.str());
            ENSURE_DISTANCE(intminus,numintminus,tolerance,errminus.str());
        }
    }
}

// sorry
#define MAKE_EVENTHYPOTHESES \
    I3Particle p; \
    p.SetPos(0.,0.,0.); \
    p.SetDir(1.,0.,0.); \
    p.SetTime(0.); \
    p.SetEnergy(1.0*I3Units::TeV); \
    I3ParticlePtr muon(new I3Particle(p)); \
    muon->SetShape(I3Particle::InfiniteTrack); \
    muon->SetType(I3Particle::MuPlus); \
    I3ParticlePtr pcasc(new I3Particle(p)); \
    pcasc->SetShape(I3Particle::Cascade); \
    pcasc->SetType(I3Particle::EPlus); \
    I3ParticlePtr dcasc(new I3Particle(*pcasc));

TEST_GROUP(MpePandelTests)

// This test was added because Hagar found cases where MPE crashed
// because of a too large NPE (due to a calibration error).
TEST(MpePandelFiniteTests)
{
    MAKE_EVENTHYPOTHESES;

    boost::shared_ptr<H2> h2(new H2);

    // plain Pandel
    boost::shared_ptr<PandelPlain>
        plain( new PandelPlain(h2) );
    DoMpePandelFiniteTests<PandelPlain,InfiniteMuon>(muon,plain);
    DoMpePandelFiniteTests<PandelPlain,PointCascade>(pcasc,plain);
    DoMpePandelFiniteTests<PandelPlain,DirectionalCascade>(dcasc,plain);

    // semi-convoluted Pandel, old implementation
    boost::shared_ptr<PandelGSemi>
        gsemi( new PandelGSemi(h2,10.) );
    DoMpePandelFiniteTests<PandelGSemi,InfiniteMuon>(muon,gsemi);
    DoMpePandelFiniteTests<PandelGSemi,PointCascade>(pcasc,gsemi);
    DoMpePandelFiniteTests<PandelGSemi,DirectionalCascade>(dcasc,gsemi);

    // semi-convoluted Pandel, new implementation
    boost::shared_ptr<PandelGauss>
        gaussFP( new PandelGauss(h2,10.,IPDF::Pandel::IntFastPlain) );
    DoMpePandelFiniteTests<PandelGauss,InfiniteMuon>(muon,gaussFP);
    DoMpePandelFiniteTests<PandelGauss,PointCascade>(pcasc,gaussFP);
    DoMpePandelFiniteTests<PandelGauss,DirectionalCascade>(dcasc,gaussFP);

    // convoluted Pandel with more accurate fast approximation
    boost::shared_ptr<PandelGauss>
        gaussFA( new PandelGauss(h2,10.,IPDF::Pandel::IntFastApproximation) );
    DoMpePandelFiniteTests<PandelGauss,InfiniteMuon>(muon,gaussFA);
    DoMpePandelFiniteTests<PandelGauss,PointCascade>(pcasc,gaussFA);
    DoMpePandelFiniteTests<PandelGauss,DirectionalCascade>(dcasc,gaussFA);

}

// This test was added to assess the precision of the "semi-convoluted"
// approximation: compute the integral of the convoluted Pandel using
// the plain Pandel.
TEST(IntegralApproximationTests)
{

    MAKE_EVENTHYPOTHESES;
    boost::shared_ptr<H2> h2(new H2);
    double prec = 0.05;
    double hiprec = 0.005;

    PandelGSemi gsemi4(h2,4.);
    PandelGSemi gsemi15(h2,15.);
    PandelGauss gauss4FP(h2,4.,IPDF::Pandel::IntFastPlain);
    PandelGauss gauss15FP(h2,15.,IPDF::Pandel::IntFastPlain);
    PandelGauss gauss4FA(h2,4.,IPDF::Pandel::IntFastApproximation);
    PandelGauss gauss15FA(h2,15.,IPDF::Pandel::IntFastApproximation);

    DoPandelIntegralTests<PandelGSemi,InfiniteMuon>(muon, gsemi4, prec);
    DoPandelIntegralTests<PandelGSemi,InfiniteMuon>(muon, gsemi15, prec);
    DoPandelIntegralTests<PandelGauss,InfiniteMuon>(muon, gauss4FP, prec);
    DoPandelIntegralTests<PandelGauss,InfiniteMuon>(muon, gauss15FP, prec);
    DoPandelIntegralTests<PandelGauss,InfiniteMuon>(muon, gauss4FA, hiprec,
            1.0,0.5);
    DoPandelIntegralTests<PandelGauss,InfiniteMuon>(muon, gauss15FA, hiprec,
            1.0,0.5);

    DoPandelIntegralTests<PandelGSemi,PointCascade>(pcasc, gsemi4, prec);
    DoPandelIntegralTests<PandelGSemi,PointCascade>(pcasc, gsemi15, prec);
    DoPandelIntegralTests<PandelGauss,PointCascade>(pcasc, gauss4FP, prec);
    DoPandelIntegralTests<PandelGauss,PointCascade>(pcasc, gauss15FP, prec);
    DoPandelIntegralTests<PandelGauss,PointCascade>(pcasc, gauss4FA, hiprec,
            1.0,0.5);
    DoPandelIntegralTests<PandelGauss,PointCascade>(pcasc, gauss15FA, hiprec,
            1.0,0.5);

    DoPandelIntegralTests<PandelGSemi,DirectionalCascade>(dcasc, gsemi4, prec);
    DoPandelIntegralTests<PandelGSemi,DirectionalCascade>(dcasc, gsemi15, prec);
    DoPandelIntegralTests<PandelGauss,DirectionalCascade>(dcasc, gauss4FP, prec);
    DoPandelIntegralTests<PandelGauss,DirectionalCascade>(dcasc, gauss15FP, prec);
    DoPandelIntegralTests<PandelGauss,DirectionalCascade>(dcasc, gauss4FA, hiprec,
            1.0,0.5);
    DoPandelIntegralTests<PandelGauss,DirectionalCascade>(dcasc, gauss15FA, hiprec,
            1.0,0.5);

}
