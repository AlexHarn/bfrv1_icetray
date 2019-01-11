/**
    copyright  (C) 2004
    the icecube collaboration
    $Id$

    @version $Revision: 1.2 $
    @date $Date$
    @author S Robbins <robbins@physik.uni-wuppertal.de>

    @brief Test hooks for Simple classes.
*/

#include <I3Test.h>
#include "ipdf/Simple/SimplePEP.h"
#include "ipdf/Likelihood/SPE1st.h"
#include "ipdf/Hypotheses/InfiniteMuon.h"
#include "ipdf/Simple/SimpleOmReceiver.h"
#include "ipdf/Simple/SimplePEHit.h"
#include "ipdf/Simple/SimpleHitOm.h"
#include "ipdf/Simple/SimpleDetectorResponse.h"

using std::cout;
using std::endl;

using IPDF::SimplePEP;
using IPDF::Likelihood::SPE1st;
using IPDF::InfiniteMuon;
using IPDF::SimpleOmReceiver;
using IPDF::SimplePEHit;
using IPDF::SimpleHitOm;
using IPDF::SimpleDetectorResponse;

TEST_GROUP(SimplePEPTest)

TEST(CTOR)
  {
    // Test construct/destruct
    SimplePEP* simplepdf( new SimplePEP() );
    delete simplepdf;
  }


TEST(getPdf)
  /// Test basic getPdf()
  {
    boost::shared_ptr<SimplePEP> simplepdf( new SimplePEP() );

    double px=0., py=0., pz=0.;
    double dx=0., dy=0., dz=1.;
    InfiniteMuon* sInfMuon( new InfiniteMuon(px,py,pz, dx,dy,dz) );

    double omx=0., omy=0., omz=0.;
    const SimpleOmReceiver omr(omx,omy,omz,1.,1.);

    const InfiniteMuon::EmissionGeometry<double> egeom(*sInfMuon,omr);
    double le( 10. + egeom.geometricalTime() );
    SimplePEHit pehit( omr, le );

    double pdf = simplepdf->getPdf(pehit,*sInfMuon);
    ENSURE_DISTANCE(pdf,0.,1.e-13,
	"Unexpected result of Simple PDF");

    delete sInfMuon;
  }


TEST(spe1st)
  {
    boost::shared_ptr<SimplePEP> simplepdf( new SimplePEP() );

    double px=0., py=0., pz=0.;
    double dx=0., dy=0., dz=1.;
    InfiniteMuon* sInfMuon( new InfiniteMuon(px,py,pz, dx,dy,dz) );

    double omx=0., omy=0., omz=0.;
    const SimpleOmReceiver omr(omx,omy,omz,1.,1.);

    const InfiniteMuon::EmissionGeometry<double> egeom(*sInfMuon,omr);
    double le( 10. + egeom.geometricalTime() );
    SimplePEHit* pehit = new SimplePEHit( omr, le );

    SimpleHitOm* hitom = new SimpleHitOm( omr, pehit );

    SPE1st<SimplePEP> spe( simplepdf );

    double like = spe.getLikelihood(*hitom,*sInfMuon);
    ENSURE_DISTANCE(like,0.,1.e-16,
	"Unexpected result of Simple PDF");

    delete sInfMuon;
  }

TEST(getIntPdf)
{
    boost::shared_ptr<SimplePEP> simplepdf( new SimplePEP() );
    double px=0., py=0., pz=0.;
    double dx=0., dy=0., dz=1.;
    InfiniteMuon* sInfMuon( new InfiniteMuon(px,py,pz, dx,dy,dz) );

    double omx=0., omy=0., omz=0.;
    const SimpleOmReceiver omr(omx,omy,omz,1.,1.);
    const InfiniteMuon::EmissionGeometry<double> egeom(*sInfMuon,omr);
    double le( 10. + egeom.geometricalTime() );
    SimplePEHit* pehit = new SimplePEHit( omr, le );
    
    /// @todo Improve implementation of PEPBase::getIntPdf()
    double intpdf = simplepdf->getIntPdf(*pehit,*sInfMuon);
    ENSURE_DISTANCE(intpdf,0.,1.e-16,
	"Unexpected result of Simple getIntPdf");
}
