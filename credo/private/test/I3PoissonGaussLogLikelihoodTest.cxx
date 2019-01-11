/*
 *  $Id$
 *  @version $Revision$
 *  @date $Date$
 *  @author emiddell
*/

#include <I3Test.h>
#include "credo/I3PoissonGaussLogLikelihood.h"
#include "photonics-service/I3DummyPhotonicsService.h"

TEST_GROUP(I3PoissonGaussLogLikelihood);

I3PoissonGaussLogLikelihoodPtr create() {
    I3PhotonicsServicePtr photonics = I3PhotonicsServicePtr(new I3DummyPhotonicsService());
    I3PoissonGaussLogLikelihoodPtr llh = I3PoissonGaussLogLikelihoodPtr( \
        new I3PoissonGaussLogLikelihood( "testservice",         // name
                                         photonics,             // pdf service
                                         "somepulses",          // pulsemap
                                         700*I3Units::hertz,    // noise rate
                                         -1,                    // event length
                                         450*I3Units::m,        // active volume
                                         10000,                 // gaussian error const.
                                         false,                 // only ATWD
                                         true,                  // use base contrib
                                         false,                 // use empty pulses
                                         1.0));                 // light_scale
    return llh;
}
    
TEST(Creation) {
    I3PoissonGaussLogLikelihoodPtr llh = create();
    ENSURE( llh->GetName() == "testservice" );
}

TEST(StaticBadDOMs) {
    I3PoissonGaussLogLikelihoodPtr llh = create();
    std::vector<OMKey> baddoms;
    baddoms.push_back(OMKey(1,1));
    baddoms.push_back(OMKey(1,2));
    baddoms.push_back(OMKey(1,2)); // duplicate
    baddoms.push_back(OMKey(1,3));
    llh->SetBadDOMs("baddomsinframe", baddoms);

    const std::vector<OMKey>& list = llh->GetStaticBadDOMList();
    ENSURE(list.size() == 3);
    // assume that order hasn't change.
    ENSURE(list.at(0) == OMKey(1,1));
    ENSURE(list.at(1) == OMKey(1,2));
    ENSURE(list.at(2) == OMKey(1,3));

}

