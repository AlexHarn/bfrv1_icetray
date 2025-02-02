/*
 * copyright  (C) 2010
 * The Icecube Collaboration
 *
 * $Id$
 *
 * @version $Revision$
 * @date $LastChangedDate$
 * @author Fabian Kislat <fabian.kislat@desy.de>, Peter Nießen (tanktop)
 * Last changed by: $LastChangedBy$
 */

#ifndef G4TANKRESPONSE_G4ICETOPEMPHYSICS_H_INCLUDED
#define G4TANKRESPONSE_G4ICETOPEMPHYSICS_H_INCLUDED

#include <globals.hh>
#include <G4VPhysicsConstructor.hh>
#include <G4PhotoElectricEffect.hh>
#include <G4ComptonScattering.hh>
#include <G4GammaConversion.hh>
#include <G4eMultipleScattering.hh>
#include <G4eIonisation.hh>
#include <G4eBremsstrahlung.hh>
#include <G4eplusAnnihilation.hh>

/**
   @class G4IceTopEMPhysics
   @brief Electromagnetic physics. Used only if Geant4 version is earlier than 4.10.

   This class implements the electromagnetic interactions
   - Photoelectric effect
   - Compton scattering
   - Gamma conversion
   - Multiple scattering
   - Ionisation/Bremsstrahlung for electrons
   - Positron annihilation
*/
class G4IceTopEMPhysics : public G4VPhysicsConstructor {
public:
  G4IceTopEMPhysics();
  ~G4IceTopEMPhysics();

  void ConstructParticle();
  void ConstructProcess();

private:
  // Gamma physics
  G4PhotoElectricEffect photoEffect;
  G4ComptonScattering comptonEffect;
  G4GammaConversion pairProduction;

  // Electron physics
  G4eMultipleScattering electronMultipleScattering;
  G4eIonisation electronIonisation;
  G4eBremsstrahlung electronBremsStrahlung;
  
  //Positron physics
  G4eMultipleScattering positronMultipleScattering;
  G4eIonisation positronIonisation; 
  G4eBremsstrahlung positronBremsStrahlung;  
  G4eplusAnnihilation annihilation;
};

#endif // G4TANKRESPONSE_G4ICETOPEMPHYSICS_H_INCLUDED
