/**
 * Copyright (C) 2007
 * The IceCube collaboration
 * ID: $Id$
 *
 * @file TopRecoFunctions.cxx
 * @version $Rev$
 * @date $Date$
 * @author $Author$
 */


#include "toprec/TopRecoFunctions.h"
#include "icetray/I3TrayHeaders.h"

#include "dataclasses/geometry/I3Geometry.h"
#include "dataclasses/geometry/I3TankGeo.h"
#include "dataclasses/geometry/I3OMGeo.h"

#include "dataclasses/status/I3DetectorStatus.h"
#include "dataclasses/status/I3DOMStatus.h"

#include "dataclasses/physics/I3RecoHit.h"
#include "dataclasses/physics/I3RecoPulse.h"

namespace TopRecoFunctions {

void OutputEmptyParticle(I3FramePtr frame, std::string outShowerName, I3Particle::FitStatus fitStatus){
  I3ParticlePtr outParticle (new I3Particle());
  outParticle->SetShape(I3Particle::TopShower);
  outParticle->SetFitStatus(fitStatus);
  frame->Put(outShowerName, outParticle);
}

/********************************************************************/

} // end namespace
