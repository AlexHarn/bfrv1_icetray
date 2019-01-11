/**
 * Copyright (C) 2009
 * The IceCube collaboration
 * ID: $Id$
 *
 * @file G4IceTopPhysicsList.h
 * @version $Rev$
 * @date $Date$
 * @author Tilo Waldenmaier, Thomas Melzig, Fabian Kislat
 */

#ifndef _TOPSIMULATOR_G4ICETOPPHYSICSLIST_H
#define _TOPSIMULATOR_G4ICETOPPHYSICSLIST_H

#include <G4VModularPhysicsList.hh>

/**
 * Implementation of G4VModularPhysicsList. The top-level physics list. It combines all the other physics lists:
 *
 *  \li G4IceTopGeneralPhysics
 *  \li G4IceTopHadronPhysics
 *  \li G4IceTopIonPhysics
 *  \li G4IceTopMuonPhysics
 *  \li G4IceTopEMPhysics
 */
class G4IceTopPhysicsList: public G4VModularPhysicsList
{
public:
  G4IceTopPhysicsList();
  ~G4IceTopPhysicsList();
private:
  void SetCuts();
};

#endif
