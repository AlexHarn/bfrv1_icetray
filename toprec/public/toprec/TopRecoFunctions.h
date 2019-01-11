/**
 * Copyright (C) 2007
 * The IceCube collaboration
 * ID: $Id$
 *
 * @file TopRecoFunctions.h
 * @version $Rev$
 * @date $Date$
 * @author $Author$
 */

#ifndef __TOPRECOFUNCTIOS_H_
#define __TOPRECOFUNCTIOS_H_

#include <cstdlib>
#include <vector>
#include "dataclasses/physics/I3Particle.h"
#include "icetray/I3TrayHeaders.h"


namespace TopRecoFunctions {

void OutputEmptyParticle(I3FramePtr frame, std::string outShowerName, I3Particle::FitStatus fitStatus);


} // end namespace

#endif
