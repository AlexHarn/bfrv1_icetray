/**
    copyright  (C) 2006
    the icecube collaboration
    $Id$

    @version $Revision: 1.2 $
    @date $Date$
    @author S Robbins <robbins@physik.uni-wuppertal.de>
    @author C Wiebusch <wiebusch@physik.uni-wuppertal.de>

    @todo Add more tests for EmissionGeometry
*/
#include "ipdf/Hypotheses/DirectionalCascade.h"
#include "ipdf/Simple/SimpleOmReceiver.h"
#include "ipdf/Utilities/IPdfConstants.h"

#include "dataclasses/I3Position.h"
#include "dataclasses/I3Direction.h"

#include <I3Test.h>
#include <string>
#include <iostream>
#include <iomanip>
#include <cmath>

using std::string;
using std::cout;
using std::endl;

using IPDF::SimpleOmReceiver;
using IPDF::DirectionalCascade;

namespace {
  const double prec = 1.e-14;
}

TEST_GROUP(DirectionalCascadeTest)

  /// Test setting of parameters via ctor 
  /// (particularly normalisation of directional cosine)
  TEST(CTOR)
  {
    double vx=1., vy=2., vz=3.;
    double dx=4., dy=5., dz=-1.;
    double energy=1000.0, t0=1234.5;
    DirectionalCascade* cascade =
        new DirectionalCascade(vx,vy,vz,dx,dy,dz,energy,t0);
    
    ENSURE_DISTANCE(cascade->getPointX(),1.,prec);
    ENSURE_DISTANCE(cascade->getPointY(),2.,prec);
    ENSURE_DISTANCE(cascade->getPointZ(),3.,prec);
    double dn = sqrt(dx*dx + dy*dy + dz*dz );
    dx=dx/dn; dy=dy/dn; dz=dz/dn;
    ENSURE_DISTANCE(cascade->getDircosX(),dx,prec);
    ENSURE_DISTANCE(cascade->getDircosY(),dy,prec);
    ENSURE_DISTANCE(cascade->getDircosZ(),dz,prec);
    ENSURE_DISTANCE(cascade->getEnergy(),energy,prec*energy);
    ENSURE_DISTANCE(cascade->getTZero(),t0,prec*t0);

    delete cascade;
  }

  /// Test propagation distance calculation 
  /// @todo loop over trk params
  TEST(propagationDistance1)
  {
    double px=1., py=2., pz=3.;
    double dx=3., dy=4., dz=-1.;
    DirectionalCascade* cascade = new DirectionalCascade(px,py,pz, dx,dy,dz);
    
    const SimpleOmReceiver omr(px,py,pz,1.,1.);
    
    double min_dist( cascade->min_dist );
    const DirectionalCascade::EmissionGeometry<double> egeom(*cascade,omr);
    ENSURE_DISTANCE(egeom.propagationDistance(),
		    min_dist,
		    prec);

    delete cascade;
  }

  /// Test propagation distance calculation 
  /// @todo loop over trk params
  TEST(propagationDistance2)
  {
    double px=1., py=2., pz=3.;
    double dx=3., dy=4., dz=-1.;
    DirectionalCascade* cascade = new DirectionalCascade(px,py,pz, dx,dy,dz);

    double testx = px + dz;
    double testy = py + 0.;  // ignore y-dirn
    double testz = pz - dx;
    const SimpleOmReceiver omr2(testx,testy,testz,1.,1.);

    double prop_dist = sqrt( dx*dx + dz*dz );

    const DirectionalCascade::EmissionGeometry<double> egeom(*cascade,omr2);
    ENSURE_DISTANCE(egeom.propagationDistance(),
		    prop_dist,
		    prec);

    delete cascade;
  }

  /// Test emissionGeometry() calculation for trival up-going cascade
  TEST(emissionGeometry)
  {
    double px=1., py=0., pz=0.;
    double dx=0., dy=0., dz=1.;  // up-going
    DirectionalCascade* cascade = new DirectionalCascade(px,py,pz, dx,dy,dz);

    double testx = 0.;
    double testy = 0.;
    double testz = 0.;
    const SimpleOmReceiver omr(testx,testy,testz,1.,1.);
    const DirectionalCascade::EmissionGeometry<double> egeom(*cascade,omr);

    // cosine Cherenkov angle:
    double emiss_ang = 0.; // cos(M_PI/2.)
    ENSURE_DISTANCE(egeom.cosEmissionAngle(),
		    emiss_ang,
		    prec);

    // geometrical time from R0 to OM
    double geo_time = 1. / IPDF::IPdfConstants::C_ICE_G;
    ENSURE_DISTANCE(egeom.geometricalTime(),
		    geo_time,
		    prec);

    delete cascade;
  }

  /// Test emission angle calculation for a contrived case
  TEST(emissionAngle)
  {
    double px=-234., py=962., pz=-667.33;
    double dx=477., dy=139., dz=-113.;
    DirectionalCascade* cascade = new DirectionalCascade(px,py,pz, dx,dy,dz);

    double testx = px + dz;
    double testy = py + 0.;  // ignore y-dirn
    double testz = pz - dx;
    const SimpleOmReceiver omr(testx,testy,testz,1.,1.);

    // double emiss_ang( 0.9730682469634644 );
    double emiss_ang = 0;

    const DirectionalCascade::EmissionGeometry<double> egeom(*cascade,omr);
    ENSURE_DISTANCE(egeom.cosEmissionAngle(),
		    emiss_ang,
		    prec);

    delete cascade;
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
    DirectionalCascade* cascade = new DirectionalCascade(px,py,pz, dx,dy,dz);

    ENSURE_DISTANCE(cascade->getTheta(),
		    theta,
		    prec);

    ENSURE_DISTANCE(cascade->getPhi(),
		    phi,
		    prec);

    delete cascade;
  }
  
  /// @todo Test construction from theta, phi system:
  TEST(ctorThetaPhi)
  {
    double px=1., py=2., pz=3.;
    double theta=1.1, phi=2.2;
    I3Position  posn(px,py,pz);
    I3Direction dirn; dirn.SetThetaPhi(theta,phi);

    DirectionalCascade* cascade = new DirectionalCascade(posn,dirn);

    ENSURE_DISTANCE(cascade->getTheta(),
		    theta,
		    prec);

    ENSURE_DISTANCE(cascade->getPhi(),
		    phi,
		    prec);

    delete cascade;
  }
  
  /// Test illegal input params
  TEST(illegalInput1)
  {
    double px=1., py=2., pz=3.;  // irrelevant
    double dx=0., dy=0., dz=0.;  // illegal
    DirectionalCascade* cascade;
    try {
      cascade = new DirectionalCascade(px,py,pz, dx,dy,dz);
    }
    catch(IPDF::IllegalInputParameter& exp) {
      // expected exception thrown
      return;
    }
    catch(...) {
      FAIL("Wrong exception from DirectionalCascade ctor, expected "
	   "IPDF::IllegalInputParameter.");
    }
    FAIL("Expected exception not thrown by DirectionalCascade ctor.");

    delete cascade;
  }
  
  /// Test illegal input params
  TEST(illegalInput2)
  {
    double px=1., py=2., pz=3.;  // irrelevant
    double dx=0., dy=0., dz=1.;
    double energy=NAN, t0=NAN;  // illegal
    DirectionalCascade* cascade;
    try {
      cascade = new DirectionalCascade(px,py,pz, dx,dy,dz,energy,t0);
    }
    catch(IPDF::IllegalInputParameter& exp) {
      // expected exception thrown
      return;
    }
    catch(...) {
      FAIL("Wrong exception from DirectionalCascade ctor, expected "
	   "IPDF::IllegalInputParameter.");
    }
    FAIL("Expected exception not thrown by DirectionalCascade ctor.");

    delete cascade;
  }
