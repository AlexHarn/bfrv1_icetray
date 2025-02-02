/*
 * copyright  (C) 2010
 * The Icecube Collaboration
 *
 * $Id$
 *
 * @version $Revision$
 * @date $LastChangedDate$
 * @author Fabian Kislat <fabian.kislat@desy.de> Last changed by: $LastChangedBy$
 */

#ifndef G4TANKRESPONSE_G4ICETOPGENERALPHYSICS_H_INCLUDED
#define G4TANKRESPONSE_G4ICETOPGENERALPHYSICS_H_INCLUDED

#include <globals.hh>
#include <G4VPhysicsConstructor.hh>
#include <G4Decay.hh>

/**
 * Implementation of G4VPhysicsConstructor for decay processes and geantino.
 */
class G4IceTopGeneralPhysics : public G4VPhysicsConstructor
{
public:
  G4IceTopGeneralPhysics();
  virtual ~G4IceTopGeneralPhysics();

  void ConstructParticle();
  void ConstructProcess();

private:
  G4Decay decay;
};

#endif // G4TANKRESPONSE_G4ICETOPGENERALPHYSICS_H_INCLUDED
