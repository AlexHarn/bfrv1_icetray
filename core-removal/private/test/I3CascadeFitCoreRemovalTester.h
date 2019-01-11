/**
 *
 * Declaration of I3CascadeFitCoreRemovalTester
 *
 * (c) 2009
 * the IceCube Collaboration
 * $Id$
 *
 * @file I3CascadeFitCoreRemovalTester.h
 * @date $Date$
 * @author panknin
 *
 */

#ifndef __I3_CASCADE_FIT_CORE_REMOVAL_TESTER_H_
#define __I3_CASCADE_FIT_CORE_REMOVAL_TESTER_H_

#include "boost/shared_ptr.hpp"
#include "dataclasses/physics/I3Particle.h"
#include "core-removal/I3CascadeFitCoreRemoval.h"

class I3CascadeFitCoreRemovalTester
{
  public:
    I3CascadeFitCoreRemovalTester();

  double CalculateSPERadius(I3ParticleConstPtr vertex) {
    return coreRemover_->CalculateSPERadius (vertex);
  };
  double CalculateSPERadius(double energy) {
    return coreRemover_->CalculateSPERadius (energy);
  };

  private:
    boost::shared_ptr<I3CascadeFitCoreRemoval > coreRemover_;
    SET_LOGGER("I3CascadeFitCoreRemovalTester");
};

#endif
