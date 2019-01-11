/**
 * Copyright (C) 2009
 * The IceCube collaboration
 * ID: $Id$
 *
 * @file G4IceTopDetectorConstruction.h
 * @version $Rev$
 * @date $Date$
 * @author Tilo Waldenmaier, Thomas Melzig
 */


#ifndef _TOPSIMULATOR_G4ICETOPDETECTORCONSTRUCTION_H_
#define _TOPSIMULATOR_G4ICETOPDETECTORCONSTRUCTION_H_

#include <G4VUserDetectorConstruction.hh>
#include <G4ThreeVector.hh>

class G4IceTopTank;

/**
 * The implementation of Geant4's G4VUserDetectorConstruction. It creates all materials and top-level volumes. It uses G4IceTopTank and G4Delaunay to build the basic elements in the geometry.
 *
 * The snow surface is a Delaunay triangulation defined by taking three points around each tank. The points are defined by G4IceTopTank.
 */

class G4IceTopDetectorConstruction: public G4VUserDetectorConstruction
{
 public:
  G4IceTopDetectorConstruction();

  ~G4IceTopDetectorConstruction();

  G4VPhysicalVolume* Construct();

  void SetVerboseLevel(G4int level) {verboseLevel_=level;}

  void InstallTank(G4IceTopTank* tank) {tankList_.push_back(tank);}

  const G4ThreeVector& GetWorldOrigin() const {return origin_;}

 private:

  void CreateMaterials();

  void CreateAir();
  void CreateIce();
  void CreateSnow();
  void CreateWater();
  void CreatePlastic();
  void CreateTyvek();
  void CreatePerlite();
  void CreateGlassSphere();
  void CreateEffectiveDOMMaterial();

  G4ThreeVector origin_;

  G4int verboseLevel_;

  std::vector<G4IceTopTank*> tankList_;
};

#endif
