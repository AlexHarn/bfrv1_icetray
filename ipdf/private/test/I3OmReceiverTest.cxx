/**
    copyright  (C) 2004
    the icecube collaboration
    $Id$

    @version $Revision: 1.1 $
    @date $Date$
    @author S Robbins <robbins@physik.uni-wuppertal.de>

    @brief Test hooks for I3OmReceiver
*/

#include <I3Test.h>
#include "ipdf/I3/I3OmReceiver.h"
using std::cout;
using std::endl;

using IPDF::I3OmReceiver;

TEST_GROUP(I3OmReceiverTest)

TEST(trivial)
  {
    const double prec = 1.e-14;
    double px=4., py=5., pz=-1.;
    double ori=-1., sigma=5.;
    I3OmReceiver* sOmR( new I3OmReceiver(px,py,pz,ori,sigma) );
	
    ENSURE_DISTANCE(sOmR->getX(),px,prec);
    ENSURE_DISTANCE(sOmR->getY(),py,prec);
    ENSURE_DISTANCE(sOmR->getZ(),pz,prec);
    ENSURE_DISTANCE(sOmR->getOrientation(),ori,prec);
    ENSURE_DISTANCE(sOmR->getJitter(),sigma,prec);

    delete sOmR;
  }
