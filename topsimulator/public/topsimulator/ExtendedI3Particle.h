/**
 * Copyright (C) 2015
 * The IceCube collaboration
 * ID: $Id: $
 *
 * @file ExtendedI3Particle.h
 * @version $Rev: $
 * @date $Date: $
 * @author Javier Gonzalez
 */


#ifndef _EXTENDEDI3PARTICLE_H_
#define _EXTENDEDI3PARTICLE_H_


#include <dataclasses/physics/I3Particle.h>

/**
 * This enum lists all air shower components that could be used in simulation.
 */
enum AirShowerComponent {
  Undefined,
  Background,
  Gamma,
  Electron,
  Muon,
  ElectronFromChargedMesons,
  ElectronFromLocalChargedMesons,
  GammaFromChargedMesons,
  GammaFromLocalChargedMesons,
  Hadron
};

/**
 * \brief ExtendedI3Particle extends the functionality of I3Particle
 * by adding a component enumeration type that corresponds to the air
 * shower component.
 *
 * NOTE: I3Particle provides no virtual method (except the destructor,
 * fortunately). That means one should NOT reimplement ANY of
 * I3Particle methods as this could cause trouble and/or confusion
 * when sliced. As it is, zero intersection with I3Particle, it works fine.
 */

class ExtendedI3Particle : public I3Particle
{
public:

  // I do not see a reason to have other constructors. This could change.
  ExtendedI3Particle();
  /// Explicit copy constructor to avoid implicit conversion.
  explicit ExtendedI3Particle(const I3Particle& p, AirShowerComponent c=Undefined);

  ~ExtendedI3Particle(){}

  // Return a copy of the instance
  ExtendedI3Particle Clone() const
  {
    return ExtendedI3Particle(I3Particle::Clone(), component_);
  }

  AirShowerComponent GetAirShowerComponent() const
  { return component_; }

  void SetAirShowerComponent(AirShowerComponent c)
  { component_ = c; }

private:
  AirShowerComponent component_;

  SET_LOGGER("ExtendedI3Particle");
};

#endif
