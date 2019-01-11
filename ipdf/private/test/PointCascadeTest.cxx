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
#include "ipdf/Hypotheses/PointCascade.h"
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
using IPDF::PointCascade;

namespace {
  const double prec = 1.e-14;
}

TEST_GROUP(PointCascadeTest)

  /// Test setting of parameters via ctor 
  /// (particularly normalisation of directional cosine)
  TEST(CTOR)
  {
    double px=4., py=5., pz=-1.,energy=1000.0,t0=1234.5;
    PointCascade* cascade = new PointCascade(px,py,pz,energy,t0);
    
    ENSURE_DISTANCE(cascade->getPointX(),px,prec);
    ENSURE_DISTANCE(cascade->getPointY(),py,prec);
    ENSURE_DISTANCE(cascade->getPointZ(),pz,prec);
    ENSURE_DISTANCE(cascade->getEnergy(),energy,prec*energy);
    ENSURE_DISTANCE(cascade->getTZero(),t0,prec*t0);

    delete cascade;
  }

  /// Test perpendicular distance calculation 
  /// @todo loop over trk params
  TEST(propagationDistance1)
  {
    double px=1., py=2., pz=3.;
    PointCascade* cascade = new PointCascade(px,py,pz);
    
    const SimpleOmReceiver omr(px,py,pz,1.,1.);
    
    double min_dist( cascade->min_dist );
    const PointCascade::EmissionGeometry<double> egeom(*cascade,omr);
    ENSURE_DISTANCE(egeom.propagationDistance(),
		    min_dist,
		    prec);

    delete cascade;
  }

  /// Test distance calculation 
  /// @todo loop over trk params
  TEST(propagationDistance2)
  {
    double px=1., py=2., pz=3.;
    double dx=3., dy=4., dz=-1.;
    PointCascade* cascade = new PointCascade(px,py,pz);

    double testx = px + dz;
    double testy = py + dy;
    double testz = pz - dx;
    const SimpleOmReceiver omr2(testx,testy,testz,1.,1.);

    double prop_dist = sqrt( dx*dx + dy*dy + dz*dz );

    const PointCascade::EmissionGeometry<double> egeom(*cascade,omr2);
    ENSURE_DISTANCE(egeom.propagationDistance(),
		    prop_dist,
		    prec);

    delete cascade;
  }

  /// Test emissionGeometry() calculation for trival cascade
  TEST(emissionGeometry)
  {
    double px=1., py=0., pz=0.;
    PointCascade* cascade = new PointCascade(px,py,pz);

    double testx = 0.;
    double testy = 0.;
    double testz = 0.;
    const SimpleOmReceiver omr(testx,testy,testz,1.,1.);
    const PointCascade::EmissionGeometry<double> egeom(*cascade,omr);

    // cosine Cherenkov angle:
    // double emiss_ang = 0.; // cos(M_PI/2.)
    // ENSURE_DISTANCE(egeom.cosEmissionAngle(),
		    // emiss_ang,
		    // prec);

    // geometrical time from R0 to OM
    double geo_time = 1. / IPDF::IPdfConstants::C_ICE_G;
    ENSURE_DISTANCE(egeom.geometricalTime(),
		    geo_time,
		    prec);

    delete cascade;
  }
  
  /// Test illegal input params
  TEST(illegalInput)
  {
    double px=1., py=2., pz=3.;  // irrelevant
    double energy=NAN, t0=NAN;  // illegal
    PointCascade* cascade;
    try {
      cascade = new PointCascade(px,py,pz,energy,t0);
    }
    catch(IPDF::IllegalInputParameter& exp) {
      // expected exception thrown
      return;
    }
    catch(...) {
      FAIL("Wrong exception from PointCascade ctor, expected "
	   "IPDF::IllegalInputParameter.");
    }
    FAIL("Expected exception not thrown by PointCascade ctor.");

    delete cascade;
  }
