/**
    copyright  (C) 2004
    the icecube collaboration
    $Id$

    @version $Revision: 1.1 $
    @date $Date$
    @author S Robbins <robbins@physik.uni-wuppertal.de>

    @brief Test hooks for SimpleOmReceiver
*/

#include <I3Test.h>
#include "ipdf/Simple/SimpleOmReceiver.h"
using std::cout;
using std::endl;

using IPDF::SimpleOmReceiver;

TEST_GROUP(SimpleOmReceiverTest)

TEST(trivial)
  {
    const double prec = 1.e-15;
    double px=4., py=5., pz=-1.;
    double ori=-1., sigma=5.;
    SimpleOmReceiver* sOmR( new SimpleOmReceiver(px,py,pz,ori,sigma) );
	
    ENSURE_DISTANCE(sOmR->getX(),px,prec);
    ENSURE_DISTANCE(sOmR->getY(),py,prec);
    ENSURE_DISTANCE(sOmR->getZ(),pz,prec);
    ENSURE_DISTANCE(sOmR->getOrientation(),ori,prec);
    ENSURE_DISTANCE(sOmR->getJitter(),sigma,prec);

    delete sOmR;
  }
