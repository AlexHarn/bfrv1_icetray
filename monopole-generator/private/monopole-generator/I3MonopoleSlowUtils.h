#ifndef MONOPOLE_PROPAGATOR_MONOPOLESLOWUTILS_H
#define MONOPOLE_PROPAGATOR_MONOPOLESLOWUTILS_H

#include <phys-services/I3RandomService.h>
#include <dataclasses/physics/I3MCTreeUtils.h>

namespace I3MonopoleSlowUtils {

    void PropagateSlowMonopole(
            I3FramePtr,
            std::string,
            I3MCTreeConstPtr,
            I3MCTreePtr,
            // params
            I3RandomServicePtr,
            double,
            bool,
            double);
}


#endif //MONOPOLE_PROPAGATOR_MONOPOLESLOWUTILS_H
