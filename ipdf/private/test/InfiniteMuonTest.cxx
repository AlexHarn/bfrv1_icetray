/**
    copyright  (C) 2004
    the icecube collaboration
    $Id$

    @version $Revision: 1.2 $
    @date $Date$
    @author S Robbins <robbins@physik.uni-wuppertal.de>
*/

#include <string>
#include <iostream>
#include <iomanip>
#include <cmath>

#include <I3Test.h>
#include "ipdf/Simple/SimpleOmReceiver.h"
#include "ipdf/Hypotheses/InfiniteMuon.h"
#include "ipdf/Utilities/IPdfException.h"
#include "ipdf/Utilities/IPdfConstants.h"

#include "dataclasses/I3Position.h"
#include "dataclasses/I3Direction.h"

using std::string;
using std::cout;
using std::endl;

using IPDF::SimpleOmReceiver;
using IPDF::InfiniteMuon;

namespace {
  const double prec = 1.e-14;
}

TEST_GROUP(InfiniteMuonTest)

  /// Test setting of parameters via ctor 
  /// (particularly normalisation of directional cosine)
  TEST(CTOR)
  {
    double dx=4., dy=5., dz=-1.;
    InfiniteMuon* sInfMuon = new InfiniteMuon(1.,2.,3., dx,dy,dz);
    
    ENSURE_DISTANCE(sInfMuon->getPointX(),1.,prec);
    ENSURE_DISTANCE(sInfMuon->getPointY(),2.,prec);
    ENSURE_DISTANCE(sInfMuon->getPointZ(),3.,prec);
    double dn = sqrt(dx*dx + dy*dy + dz*dz );
    dx=dx/dn; dy=dy/dn; dz=dz/dn;
    ENSURE_DISTANCE(sInfMuon->getDircosX(),dx,prec);
    ENSURE_DISTANCE(sInfMuon->getDircosY(),dy,prec);
    ENSURE_DISTANCE(sInfMuon->getDircosZ(),dz,prec);

    delete sInfMuon;
  }

  /// Test perpendicular distance calculation 
  /// @todo loop over trk params
  TEST(perpendicularDistance1)
  {
    double px=1., py=2., pz=3.;
    double dx=3., dy=4., dz=-1.;
    InfiniteMuon* sInfMuon = new InfiniteMuon(px,py,pz, dx,dy,dz);
    
    const SimpleOmReceiver omr(px,py,pz,1.,1.);
    
    double min_dist( sInfMuon->min_dist );
    const InfiniteMuon::EmissionGeometry<double> egeom(*sInfMuon,omr);
    ENSURE_DISTANCE(egeom.perpendicularDistance(),
		    min_dist,
		    prec);

    delete sInfMuon;
  }

  /// Test perpendicular distance calculation 
  /// @todo loop over trk params
  TEST(perpendicularDistance2)
  {
    double px=1., py=2., pz=3.;
    double dx=3., dy=4., dz=-1.;
    InfiniteMuon* sInfMuon = new InfiniteMuon(px,py,pz, dx,dy,dz);

    double testx = px + dz;
    double testy = py + 0.;  // ignore y-dirn
    double testz = pz - dx;
    const SimpleOmReceiver omr2(testx,testy,testz,1.,1.);

    double perp_dist = sqrt( dx*dx + dz*dz );

    const InfiniteMuon::EmissionGeometry<double> egeom(*sInfMuon,omr2);
    ENSURE_DISTANCE(egeom.perpendicularDistance(),
		    perp_dist,
		    prec);

    delete sInfMuon;
  }

  /// Test emissionGeometry() calculation for trival up-going muon
  TEST(emissionGeometry)
  {
    double px=1., py=0., pz=0.;
    double dx=0., dy=0., dz=1.;  // up-going
    InfiniteMuon* sInfMuon = new InfiniteMuon(px,py,pz, dx,dy,dz);

    double testx = 0.;
    double testy = 0.;
    double testz = 0.;
    const SimpleOmReceiver omr(testx,testy,testz,1.,1.);
    const InfiniteMuon::EmissionGeometry<double> egeom(*sInfMuon,omr);

    // cosine Cherenkov angle:
    double emiss_ang = -IPDF::IPdfConstants::CS_CER_ICE_P;
    ENSURE_DISTANCE(egeom.cosEmissionAngle(),
		    emiss_ang,
		    prec);

    // geometrical time from R0 to OM
    double geo_time = IPDF::IPdfConstants::TG_CER_TKOEFF
	/ IPDF::IPdfConstants::C_VACUUM;
    ENSURE_DISTANCE(egeom.geometricalTime(),
		    geo_time,
		    prec);

    delete sInfMuon;
  }

  /// Test emission angle calculation for a contrived case
  TEST(emissionAngle)
  {
    double px=-234., py=962., pz=-667.33;
    double dx=477., dy=139., dz=-113.;
    InfiniteMuon* sInfMuon = new InfiniteMuon(px,py,pz, dx,dy,dz);

    double testx = px + dz;
    double testy = py + 0.;  // ignore y-dirn
    double testz = pz - dx;
    const SimpleOmReceiver omr(testx,testy,testz,1.,1.);

    double emiss_ang( 0.8029173859252237 );

    const InfiniteMuon::EmissionGeometry<double> egeom(*sInfMuon,omr);
    ENSURE_DISTANCE(egeom.cosEmissionAngle(),
		    emiss_ang,
		    prec);

    delete sInfMuon;
  }
  
  /// Test conversion to theta, phi system:
  TEST(getThetaPhi)
  {
    double theta=0.3463;
    double phi  =2.3679;
    double ctht = cos(theta);
    double stht = sin(theta);
    double cphi = cos(phi);
    double sphi = sin(phi);

    double px=1., py=2., pz=3.;  // irrelevant
    double dx= stht*cphi;
    double dy= stht*sphi;
    double dz= ctht;
    InfiniteMuon* sInfMuon = new InfiniteMuon(px,py,pz, dx,dy,dz);

    ENSURE_DISTANCE(sInfMuon->getTheta(),
		    theta,
		    prec);

    ENSURE_DISTANCE(sInfMuon->getPhi(),
		    phi,
		    prec);

    delete sInfMuon;
  }
  
  /// @todo Test construction from theta, phi system:
  TEST(ctorThetaPhi)
  {
    double px=1., py=2., pz=3.;
    double theta=1.1, phi=2.2;
    I3Position  posn(px,py,pz);
    I3Direction dirn; dirn.SetThetaPhi(theta,phi);

    InfiniteMuon* sInfMuon = new InfiniteMuon(posn,dirn);

    ENSURE_DISTANCE(sInfMuon->getTheta(),
		    theta,
		    prec);

    ENSURE_DISTANCE(sInfMuon->getPhi(),
		    phi,
		    prec);

    delete sInfMuon;
  }
  
  /// Test illegal input params
  TEST(illegalInput)
  {
    double px=1., py=2., pz=3.;  // irrelevant
    double dx=0., dy=0., dz=0.;  // illegal
    InfiniteMuon* sInfMuon;
    try {
      sInfMuon = new InfiniteMuon(px,py,pz, dx,dy,dz);
    }
    catch(IPDF::IllegalInputParameter& exp) {
      // expected exception thrown
      return;
    }
    catch(...) {
      FAIL("Wrong exception from InfiniteMuon ctor, expected "
	   "IPDF::IllegalInputParameter.");
    }
    FAIL("Expected exception not thrown by InfiniteMuon ctor.");

    delete sInfMuon;
  }
