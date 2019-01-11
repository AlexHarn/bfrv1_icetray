/**
 * Copyright (C) 2009
 * The IceCube collaboration
 * ID: $Id$
 *
 * @file G4IceTopPhysicsList.cxx
 * @version $Rev$
 * @date $Date$
 * @author Tilo Waldenmaier, Thomas Melzig, Javier Gonzalez
 */


#include <globals.hh>
#include <G4Version.hh>
#include "G4IceTopPhysicsList.h"
#include "G4IceTopGeneralPhysics.h"
#if G4VERSION_NUMBER < 1000
#include "G4IceTopEMPhysics.h"
#include "G4IceTopMuonPhysics.h"
#include "G4IceTopHadronPhysics.h"
#include "G4IceTopIonPhysics.h"
#else
#include "G4DecayPhysics.hh"
#include "G4EmStandardPhysics.hh"
#include "G4EmExtraPhysics.hh"
#include "G4IonPhysics.hh"
#include "G4StoppingPhysics.hh"
#include "G4HadronElasticPhysics.hh"
//#include "G4NeutronTrackingCut.hh"
#include "G4DataQuestionaire.hh"
#include "G4HadronPhysicsFTFP_BERT.hh"
#include <FTFP_BERT.hh>
#endif

#include <G4ProcessManager.hh>
#include <G4ParticleTypes.hh>
#include <G4UserSpecialCuts.hh>


G4IceTopPhysicsList::G4IceTopPhysicsList()
  : G4VUserPhysicsList()
{
  defaultCutValue = 0.7*CLHEP::mm;
  SetVerboseLevel(1);

  RegisterPhysics(new G4IceTopGeneralPhysics);
#if G4VERSION_NUMBER < 1000
  RegisterPhysics(new G4IceTopEMPhysics);
  RegisterPhysics(new G4IceTopMuonPhysics);
  RegisterPhysics(new G4IceTopHadronPhysics);
  RegisterPhysics(new G4IceTopIonPhysics);
#else
  // The following is basically Geant4's FTFP_BERT physics list
  G4DataQuestionaire it(photon); // this checks that G4LEVELGAMMADATA is there

  RegisterPhysics(new G4EmStandardPhysics());
  RegisterPhysics(new G4EmExtraPhysics());
  RegisterPhysics(new G4DecayPhysics());
  RegisterPhysics(new G4HadronElasticPhysics());
  RegisterPhysics(new G4HadronPhysicsFTFP_BERT());
  RegisterPhysics(new G4StoppingPhysics());
  RegisterPhysics(new G4IonPhysics());
  //RegisterPhysics(new G4NeutronTrackingCut());
#endif
}


G4IceTopPhysicsList::~G4IceTopPhysicsList()
{
}


void G4IceTopPhysicsList::SetCuts()
{
  //G4VUserPhysicsList::SetCutsWithDefault method sets
  //the default cut value for all particle types
  //
  SetCutsWithDefault();
  if (verboseLevel>0) DumpCutValuesTable();
}

