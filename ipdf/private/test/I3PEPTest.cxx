/**
    copyright  (C) 2004
    the icecube collaboration
    $Id$

    @version $Revision: 1.1 $
    @date $Date$
    @author S Robbins <robbins@physik.uni-wuppertal.de>
 *
 *  @brief Test hooks for I3 interface classes.
*/

#include <iomanip>

#include <I3Test.h>
#include "ipdf/AllOMsLikelihood.h"
#include "ipdf/Simple/SimplePEP.h"
#include "ipdf/Pandel/IceModel.h"
#include "ipdf/Pandel/UnConvolutedPEP.h"
#include "ipdf/Likelihood/SPE1st.h"
#include "ipdf/Hypotheses/InfiniteMuon.h"
#include "ipdf/I3/I3OmReceiver.h"
#include "ipdf/I3/I3PEHit.h"
#include "ipdf/I3/I3HitOm.h"
#include "ipdf/I3/I3DetectorResponse.h"

using std::cout;
using std::endl;

using IPDF::AllOMsLikelihood;
using IPDF::SimplePEP;
using IPDF::Likelihood::SPE1st;
using IPDF::InfiniteMuon;
using IPDF::I3OmReceiver;
using IPDF::I3PEHit;
using IPDF::I3HitOm;
using IPDF::I3DetectorResponse;

using namespace IPDF::Pandel;

typedef UnConvolutedPEP<H2> Pandel;

TEST_GROUP(I3PEPTest)

  /// Test basic getPdf()
  TEST(getPdf)
  {
    SimplePEP* simplepdf( new SimplePEP() );

    double px=0., py=0., pz=0.;
    double dx=0., dy=0., dz=1.;
    InfiniteMuon* sInfMuon( new InfiniteMuon(px,py,pz, dx,dy,dz) );

    double omx=0., omy=0., omz=0.;
    const I3OmReceiver omr(omx,omy,omz,1.,1.);

    InfiniteMuon::EmissionGeometry<double> egeom(*sInfMuon,omr);
    double le( 10. + egeom.geometricalTime() );
    I3PEHit pehit( omr, le );

    double pdf = simplepdf->getPdf(pehit,*sInfMuon);
    ENSURE_DISTANCE(pdf,0.,1.e-14,
	"Unexpected result of Simple PDF");

    delete simplepdf;
    delete sInfMuon;
  }

  /// Test with SPE1st
  TEST(basic)
  {
    double px=0., py=0., pz=0.;
    double dx=0., dy=0., dz=1.;
    InfiniteMuon* sInfMuon( new InfiniteMuon(px,py,pz, dx,dy,dz) );

    double omx=0., omy=0., omz=0.;
    const I3OmReceiver omr(omx,omy,omz,1.,1.);

    InfiniteMuon::EmissionGeometry<double> egeom(*sInfMuon,omr);
    double le( 10. + egeom.geometricalTime() );
    I3PEHit* pehit(new I3PEHit( omr, le ));

    I3HitOm* hitom(new I3HitOm( omr, pehit ));
    I3DetectorResponse response( hitom );

    AllOMsLikelihood< SPE1st<SimplePEP> > spe;

    double like = spe.getLikelihood(response,*sInfMuon);
    ENSURE_DISTANCE(like,0.,1.e-16,
	"Unexpected result of Simple PDF");

    delete sInfMuon;
  }

  /// Test with SPE1st
  TEST(realistic)
  {
    double px=0., py=0., pz=0.;
    double dx=0., dy=0., dz=1.;
    InfiniteMuon* sInfMuon( new InfiniteMuon(px,py,pz, dx,dy,dz) );

    double omx=10., omy=0., omz=0.;
    const I3OmReceiver omr(omx,omy,omz,1.,1.);

    InfiniteMuon::EmissionGeometry<double> egeom(*sInfMuon,omr);
    double le( 10. + egeom.geometricalTime() );
    I3PEHit* pehit(new I3PEHit( omr, le ));

    I3HitOm* hitom(new I3HitOm( omr, pehit ));
    I3DetectorResponse response( hitom );

    AllOMsLikelihood< SPE1st<Pandel> > spe;

    double like = spe.getLikelihood(response,*sInfMuon);
    ENSURE_DISTANCE(like,0.01069923609731488,1.e-16,
	"Unexpected result of Simple PDF");

    delete sInfMuon;
  }
