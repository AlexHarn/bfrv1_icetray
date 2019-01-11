/**
    copyright  (C) 2004
    the icecube collaboration
    $Id$

    @version $Revision: 1.6 $
    @date $Date$
    @author S Robbins <robbins@physik.uni-wuppertal.de>
*/

#include <sstream>
#include <iomanip>
#include <memory>

#include <I3Test.h>
#include "ipdf/Hypotheses/InfiniteMuon.h"
#include "ipdf/Hypotheses/PointCascade.h"
#include "ipdf/Hypotheses/DirectionalCascade.h"
#include "ipdf/Simple/SimplePEHit.h"
#include "ipdf/Simple/SimpleOmReceiver.h"
#include "ipdf/Pandel/IceModel.h"
#include "ipdf/Pandel/UnConvolutedPEP.h"
#include "ipdf/Pandel/UPatchPEP.h"
#include "ipdf/Pandel/GConvolutePEP.h"
#include "ipdf/Pandel/GSemiConvolutePEP.h"
#include "ipdf/Pandel/GaussConvolutedPEP.h"

using std::cout;
using std::endl;

using IPDF::InfiniteMuon;
using IPDF::PointCascade;
using IPDF::DirectionalCascade;
using IPDF::SimplePEHit;
using IPDF::SimpleOmReceiver;

using namespace IPDF::Pandel;

typedef UnConvolutedPEP<H2> PandelPlain;
typedef UPatchPEP<H2> PandelUPatch;
typedef GConvolutePEP<H2> PandelGConvolute;
typedef GSemiConvolutePEP<H2> PandelGSemiConvolute;
typedef GaussConvolutedPEP<H2> PandelGaussConvoluted;

namespace {
  const double prec = 1.e-13;
}

TEST_GROUP(PandelPEPTest)

  /// Test of ctor and print state (via operator<<)
  TEST(print)
  {
    PandelPlain* pandel( new PandelPlain() );
    std::ostringstream ssdummy;
    ssdummy<<(*pandel);
    std::string pandel_state( ssdummy.str() );
    delete pandel;
  }

  TEST(copy_plain)
  {
    PandelPlain pandel;
    for(int i=0; i<=2; ++i) {
      PandelPlain copy = pandel;
      // avoid "warning: unused variable `PandelPlain copy'"
      copy.getPdf(SimplePEHit(SimpleOmReceiver(0.,0.,0.,-1.,15.,1.3),10.),InfiniteMuon(10.,10.,10., 0.,0.,-1.,1.e6));

//      pandel = copy; // disabled for Pandel::UnConvolutedPEP
    }
  }

  TEST(copy_upatch)
  {
    PandelUPatch pandel;
    for(int i=0; i<=2; ++i) {
      PandelUPatch copy = pandel;
//      pandel = copy; // disabled for Pandel::UPatchPEP
    }
  }

  TEST(copy_gc)
  {
    PandelGConvolute pandel;
    for(int i=0; i<=2; ++i) {
      PandelGConvolute copy = pandel;
      pandel = copy;
      pandel = pandel;
    }
  }

  TEST(copy_gsemic)
  {
    PandelGSemiConvolute pandel;
    for(int i=0; i<=2; ++i) {
      PandelGSemiConvolute copy = pandel;
      pandel = copy;
      pandel = pandel;
    }
  }

  TEST(copy_gaussc)
  {
    PandelGaussConvoluted pandel;
    for(int i=0; i<=2; ++i) {
      PandelGaussConvoluted copy = pandel;
      pandel = copy;
      pandel = pandel;
    }
  }

  /// Test basic getPdf() with a known configuration
  /// IceModel H2, UnConvoluted
  TEST(Plain_getPdf)
  {
    double px=1., py=2., pz=3.;
    double dx=4., dy=5., dz=-1.;
    InfiniteMuon* sInfMuon = new InfiniteMuon(px,py,pz, dx,dy,dz,1.e6);
    double omx=0., omy=0., omz=0.;
    SimpleOmReceiver omr(omx,omy,omz,-1.,15.,1.3);

    const InfiniteMuon::EmissionGeometry<double> egeom(*sInfMuon,omr);
    double le( 10. + egeom.geometricalTime() );
    SimplePEHit pehit( omr, le );
 
    PandelPlain* pandel( new PandelPlain() );
    double pdf = pandel->getPdf(pehit,*sInfMuon);
    ENSURE_DISTANCE(pdf,0.01229061086462096,prec,
	"Unexpected result of UnConvoluted Pandel PDF");

    delete pandel;
    delete sInfMuon;
  }

  /// Test basic getIntPdf() with a known configuration
  TEST(Plain_getIntPdf)
  {
    double px=1., py=2., pz=3.;
    double dx=4., dy=5., dz=-1.;
    InfiniteMuon* sInfMuon( new InfiniteMuon(px,py,pz, dx,dy,dz,1.e6) );
    double omx=0., omy=0., omz=0.;
    const SimpleOmReceiver omr(omx,omy,omz,-1.,15.,1.3);

    const InfiniteMuon::EmissionGeometry<double> egeom(*sInfMuon,omr);
    double le( 10. + egeom.geometricalTime() );
    SimplePEHit pehit( omr, le );
 
    PandelPlain* pandel( new PandelPlain() );
    double pdf = pandel->getIntPdf(pehit,*sInfMuon);
    ENSURE_DISTANCE(pdf,0.366148939015478,prec,
	"Unexpected result of UnConvoluted Pandel PDF");

    delete pandel;
    delete sInfMuon;
  }

  /// Test basic getLogPdf() with a known configuration
  TEST(Plain_getLogPdf)
  {
    double px=1., py=2., pz=3.;
    double dx=4., dy=5., dz=-1.;
    InfiniteMuon* sInfMuon = new InfiniteMuon(px,py,pz, dx,dy,dz,1.e6);
    double omx=0., omy=0., omz=0.;
    SimpleOmReceiver omr(omx,omy,omz,-1.,15.,1.3);

    const InfiniteMuon::EmissionGeometry<double> egeom(*sInfMuon,omr);
    double le( 10. + egeom.geometricalTime() );
    SimplePEHit pehit( omr, le );
 
    PandelPlain* pandel( new PandelPlain() );
    double pdf = pandel->getLogPdf(pehit,*sInfMuon);
    ENSURE_DISTANCE(pdf,-4.39891965243916,1.e-14,
	"Unexpected result of UnConvoluted Pandel PDF");

    delete pandel;
    delete sInfMuon;
  }

  /// Test basic getPdf() with a known configuration
  /// IceModel H2, UPatch
  TEST(UPatch_getPdf)
  {
    double px=1., py=2., pz=3.;
    double dx=4., dy=5., dz=-1.;
    InfiniteMuon* sInfMuon = new InfiniteMuon(px,py,pz, dx,dy,dz,1.e6);
    double omx=0., omy=0., omz=0.;
    SimpleOmReceiver omr(omx,omy,omz,-1.,15.,1.3);

    PandelUPatch* ppandel( new PandelUPatch() );

    const InfiniteMuon::EmissionGeometry<double> egeom(*sInfMuon,omr);
    double le( 10. + egeom.geometricalTime() );
    SimplePEHit pehit( omr, le );
 
    double pdf = ppandel->getPdf(pehit,*sInfMuon);
    ENSURE_DISTANCE(pdf,0.01119688725636414,prec,
		    "Unexpected result of UPatch Pandel PDF");

    delete ppandel;
    delete sInfMuon;
  }

  /// Test basic getIntPdf() with a known configuration
  /// IceModel H2, UPatch
  TEST(UPatch_getIntPdf)
  {
    double px=1., py=2., pz=3.;
    double dx=4., dy=5., dz=-1.;
    InfiniteMuon* sInfMuon = new InfiniteMuon(px,py,pz, dx,dy,dz,1.e6);
    double omx=0., omy=0., omz=0.;
    SimpleOmReceiver omr(omx,omy,omz,-1.,15.,1.3);

    PandelUPatch* ppandel( new PandelUPatch() );

    const InfiniteMuon::EmissionGeometry<double> egeom(*sInfMuon,omr);
    double le( 10. + egeom.geometricalTime() );
    SimplePEHit pehit( omr, le );
 
    double pdf = ppandel->getIntPdf(pehit,*sInfMuon);
    double expect = 0.3517278274398375;
    std::ostringstream error;
    error<<"Unexpected result of UPatch Pandel PDF, expected "
	 <<std::setprecision(16)<<expect;
    ENSURE_DISTANCE(pdf,expect,prec,error.str());

    delete ppandel;
    delete sInfMuon;
  }

  /// Test basic getLogPdf() with a known configuration
  /// IceModel H2, UPatch
  TEST(UPatch_getLogPdf)
  {
    double px=1., py=2., pz=3.;
    double dx=4., dy=5., dz=-1.;
    InfiniteMuon* sInfMuon = new InfiniteMuon(px,py,pz, dx,dy,dz,1.e6);
    double omx=0., omy=0., omz=0.;
    SimpleOmReceiver omr(omx,omy,omz,-1.,15.,1.3);

    PandelUPatch* ppandel( new PandelUPatch() );

    const InfiniteMuon::EmissionGeometry<double> egeom(*sInfMuon,omr);
    double le( 10. + egeom.geometricalTime() );
    SimplePEHit pehit( omr, le );
 
    double pdf = ppandel->getLogPdf(pehit,*sInfMuon);
    double expect = -4.492119462847908;
    std::ostringstream error;
    error<<"Unexpected result of UPatch Pandel PDF, expected "
	 <<std::setprecision(16)<<expect;
    ENSURE_DISTANCE(pdf,expect,prec,error.str());


    delete ppandel;
    delete sInfMuon;
  }

  /// Test basic getPdf() with a known configuration
  /// IceModel H2, GConvolute
  TEST(GConvolute_getPdf)
  {
    double px=1., py=2., pz=3.;
    double dx=4., dy=5., dz=-1.;
    InfiniteMuon* sInfMuon = new InfiniteMuon(px,py,pz, dx,dy,dz,1.e6);
    double omx=0., omy=0., omz=0.;
    SimpleOmReceiver omr(omx,omy,omz,-1.,15.,1.3);

    PandelGConvolute* cpandel( new PandelGConvolute() );

    const InfiniteMuon::EmissionGeometry<double> egeom(*sInfMuon,omr);
    double le( 10. + egeom.geometricalTime() );
    SimplePEHit pehit( omr, le );
 
    double pdf = cpandel->getPdf(pehit,*sInfMuon);
    ENSURE_DISTANCE(pdf,0.01232066763269721,prec,
		    "Unexpected result of GConvolute Pandel PDF");

    delete cpandel;
    delete sInfMuon;
  }

  /// Test basic getLogPdf() with a known configuration
  /// IceModel H2, GConvolute
  TEST(GConvolute_getLogPdf)
  {
    double px=1., py=2., pz=3.;
    double dx=4., dy=5., dz=-1.;
    InfiniteMuon* sInfMuon = new InfiniteMuon(px,py,pz, dx,dy,dz,1.e6);
    double omx=0., omy=0., omz=0.;
    SimpleOmReceiver omr(omx,omy,omz,-1.,15.,1.3);

    PandelGConvolute* cpandel( new PandelGConvolute() );

    const InfiniteMuon::EmissionGeometry<double> egeom(*sInfMuon,omr);
    double le( 10. + egeom.geometricalTime() );
    SimplePEHit pehit( omr, le );
 
    double pdf = cpandel->getLogPdf(pehit,*sInfMuon);
    ENSURE_DISTANCE(pdf,-4.396477131379358,1.e-14,
		    "Unexpected result of GConvolute Pandel PDF");

    delete cpandel;
    delete sInfMuon;
  }

  /// Test basic getIntPdf() with a known configuration
  /// IceModel H2, GConvolute
  TEST(GConvolute_getIntPdf)
  {
    double px=1., py=2., pz=3.;
    double dx=4., dy=5., dz=-1.;
    InfiniteMuon* sInfMuon = new InfiniteMuon(px,py,pz, dx,dy,dz,1.e6);
    double omx=0., omy=0., omz=0.;
    SimpleOmReceiver omr(omx,omy,omz,-1.,15.,1.3);

    PandelGConvolute* cpandel( new PandelGConvolute() );

    const InfiniteMuon::EmissionGeometry<double> egeom(*sInfMuon,omr);
    double le( 10. + egeom.geometricalTime() );
    SimplePEHit pehit( omr, le );
 
    double pdf = cpandel->getIntPdf(pehit,*sInfMuon);
    ENSURE_DISTANCE(pdf,0.3016971145262044,prec,
		    "Unexpected result of GConvolute Pandel PDF");

    delete cpandel;
    delete sInfMuon;
  }

  /// Test basic getPdf() with a known configuration
  /// IceModel H2, GaussConvoluted
  /// TODO: check also points in regions 2, 3, 4, and 5 and outside.
  TEST(GaussConvoluted_getPdf)
  {
    double px=1., py=2., pz=3.;
    double dx=4., dy=5., dz=-1.;
    InfiniteMuon* sInfMuon = new InfiniteMuon(px,py,pz, dx,dy,dz,1.e6);
    double omx=0., omy=0., omz=0.;
    SimpleOmReceiver omr(omx,omy,omz,-1.,15.,1.3);

    PandelGaussConvoluted* gpandel( new PandelGaussConvoluted() );

    const InfiniteMuon::EmissionGeometry<double> egeom(*sInfMuon,omr);
    double le( 10. + egeom.geometricalTime() );
    SimplePEHit pehit( omr, le );
 
    double pdf = gpandel->getPdf(pehit,*sInfMuon);
    ENSURE_DISTANCE(pdf,0.01232066763269721,prec,
		    "Unexpected result of GConvolute Pandel PDF");

    delete gpandel;
    delete sInfMuon;
  }

  /// Test basic getLogPdf() with a known configuration
  /// IceModel H2, GaussConvoluted
  /// TODO: check also points in regions 2, 3, 4, and 5 and outside.
  TEST(GaussConvoluted_getLogPdf)
  {
    double px=1., py=2., pz=3.;
    double dx=4., dy=5., dz=-1.;
    InfiniteMuon* sInfMuon = new InfiniteMuon(px,py,pz, dx,dy,dz,1.e6);
    double omx=0., omy=0., omz=0.;
    SimpleOmReceiver omr(omx,omy,omz,-1.,15.,1.3);

    PandelGaussConvoluted* gpandel( new PandelGaussConvoluted() );

    const InfiniteMuon::EmissionGeometry<double> egeom(*sInfMuon,omr);
    double le( 10. + egeom.geometricalTime() );
    SimplePEHit pehit( omr, le );
 
    double pdf = gpandel->getLogPdf(pehit,*sInfMuon);
    ENSURE_DISTANCE(pdf,-4.396477131379358,1.e-14,
		    "Unexpected result of GaussConvoluted Pandel PDF");

    delete gpandel;
    delete sInfMuon;
  }

  /// Test basic getIntPdf() with a known configuration
  /// IceModel H2, GaussConvoluted
  /// TODO: check also points in regions 2, 3, 4, and 5 and outside.
  TEST(GaussConvoluted_getIntPdf)
  {
    double px=1., py=2., pz=3.;
    double dx=4., dy=5., dz=-1.;
    InfiniteMuon* sInfMuon = new InfiniteMuon(px,py,pz, dx,dy,dz,1.e6);
    double omx=0., omy=0., omz=0.;
    SimpleOmReceiver omr(omx,omy,omz,-1.,15.,1.3);

    PandelGaussConvoluted* gpandel( new PandelGaussConvoluted() );
    PandelPlain* ppandel( new PandelPlain() );

    const InfiniteMuon::EmissionGeometry<double> egeom(*sInfMuon,omr);
    double le( 10. + egeom.geometricalTime() );
    SimplePEHit pehit( omr, le );
 
    double pdfgauss = gpandel->getIntPdf(pehit,*sInfMuon);
    double pdfplain = ppandel->getIntPdf(pehit,*sInfMuon);
    ENSURE_DISTANCE(pdfgauss,pdfplain,prec,
		    "Unexpected result of GaussConvoluted Pandel PDF");

    delete gpandel;
    delete ppandel;
    delete sInfMuon;
  }

  /// Test getHitProb() with a known configuration
  /// IceModel H2, UnConvoluted
  TEST(getHitProb)
  {
    double px=1., py=2., pz=3.;
    double dx=4., dy=5., dz=-1.;
    InfiniteMuon* sInfMuon = new InfiniteMuon(px,py,pz, dx,dy,dz,1.e3/*GeV*/);
    double omx=0., omy=0., omz=0.;
    SimpleOmReceiver omr(omx,omy,omz,-1.,15.,1.3);

    PandelPlain* pandel( new PandelPlain() );
    double hitprob = pandel->getHitProb(omr,*sInfMuon);
    ENSURE_DISTANCE(hitprob,0.9884610890515714,prec,
		    "Unexpected result of Pandel getHitProb");

    delete pandel;
    delete sInfMuon;
  }

  TEST(getHitProbPointCascade){
    double px=1., py=2., pz=3.;
    double energy=1.e6/*GeV*/;
    double t0 = 0;
    PointCascade* sPointCasc = new PointCascade(px,py,pz,energy,t0);
    double omx=0., omy=0., omz=0.;
    SimpleOmReceiver omr(omx,omy,omz,-1.,15.,1.3);
    PandelPlain* pandel( new PandelPlain() );
    double dist = sqrt((px-omx)*(px-omx)+(py-omy)*(py-omy)+(pz-omz)*(pz-omz))
                  +0.1;
    double expected_mean = (1.e6*1.4/dist)*exp(-dist/29.0);
    double expected_phit = 1.0-exp(-expected_mean);
    double hitprob = pandel->getHitProb(omr,*sPointCasc);
    ENSURE_DISTANCE(hitprob,expected_phit,prec,
		    "Unexpected result of Pandel getHitProb for point cascade");
    delete pandel;
    delete sPointCasc;
  }

  TEST(getHitProbDirectionalCascade){
    // upgoing 1PeV cascade 10m above a DOM
    double px=0., py=0., pz=10.;
    double dx=0., dy=0., dz=-1.;
    DirectionalCascade* sDirectionalCasc =
        new DirectionalCascade(px,py,pz, dx,dy,dz,1.e6/* GeV */,0);
    double omx=0., omy=0., omz=0.;
    SimpleOmReceiver omr(omx,omy,omz,-1.,15.,1.3);
    PandelPlain* pandel( new PandelPlain() );
    double dist = 10.1;
    double expected_mean = (1.e6*0.45/dist)*exp(-dist/28.08);
    expected_mean *= -1.0*-1.0 + 3.364*-1.0 + 2.566;
    double expected_phit = 1.0-exp(-expected_mean);
    double hitprob = pandel->getHitProb(omr,*sDirectionalCasc);
    ENSURE_DISTANCE(hitprob,expected_phit,prec,
		    "Unexpected result of Pandel getHitProb for directional cascade");
    delete pandel;
    delete sDirectionalCasc;
  }

  /// Test expectedNPE() with a known configuration
  /// IceModel H2, UnConvoluted
  TEST(expectedNPE)
  {
    double px=1., py=2., pz=3.;
    double dx=4., dy=5., dz=-1.;
    InfiniteMuon* sInfMuon = new InfiniteMuon(px,py,pz, dx,dy,dz,1.e3/* GeV */);
    double omx=0., omy=0., omz=0.;
    SimpleOmReceiver omr(omx,omy,omz,-1.,15.,1.3);

    PandelPlain* pandel( new PandelPlain() );
    double hitprob = pandel->expectedNPE(omr,*sInfMuon);
    ENSURE_DISTANCE(hitprob,4.462030394242091,prec,
		    "Unexpected result of Pandel expectedNPE");

    delete pandel;
    delete sInfMuon;
  }

  /// Test Plain Pandel is independent of the emitter depth
  /// @todo this is only accurate to 1.e-13 - investigate.
  TEST(emitter_depth)
  {
    PandelPlain* pandel( new PandelPlain() );
    double px=1., py=0.;
    double dx=0., dy=0., dz=-1.;
    double omx=0., omy=0., omz=0.;
    SimpleOmReceiver omr(omx,omy,omz,-1.,15.,1.3);

    for(double depth=-1000.; depth<=1000.; depth+=100.) {
      std::auto_ptr<InfiniteMuon> sInfMuon(new InfiniteMuon(px,py,depth, dx,dy,dz,1.e6));

      const InfiniteMuon::EmissionGeometry<double> egeom(*sInfMuon,omr);
      double le( 10. + egeom.geometricalTime() );
      SimplePEHit pehit( omr, le );

      double pdf = pandel->getPdf(pehit,*sInfMuon);

      std::ostringstream error;
      error<<"Unexpected result of UnConvoluted Pandel: at depth "
	   <<depth<<" is "<<std::setprecision(16)<<pdf;
      ENSURE_DISTANCE(pdf,0.01221388519297869,1.e-12,error.str());
    }

    delete pandel;
  }
