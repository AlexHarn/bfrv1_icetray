/**
 * Copyright (C) 2009
 * The IceCube collaboration
 * ID: $Id: $
 *
 * @file I3InjectorService.h
 * @version $Rev: $
 * @date $Date: $
 * @author Tilo Waldenmaier
 */


#ifndef _TOPSIMULATOR_I3INJECTORSERVICE_H_
#define _TOPSIMULATOR_I3INJECTORSERVICE_H_


#include <icetray/I3ServiceBase.h>
#include <icetray/I3Context.h>
#include <icetray/I3FrameObject.h>
#include <icetray/I3DefaultName.h>
#include <dataclasses/physics/I3Particle.h>
#include <topsimulator/interface/I3IceTopResponseService.h>
#include <topsimulator/ExtendedI3Particle.h>

#include <map>
#include <string>

/**
 * This virtual base class specifies the interface for all IceTop particle injectors.
 *
 * The main method that any particle injector needs to implement is NextParticle, wich
 * takes a non-const reference to a particle. This method gets called until there are no more particles.
 *
 * Other methods that need to be implemented:
 *  - NextEvent
 *  - GetAirShowerComponentNameMap
 */

class I3InjectorService: public I3ServiceBase
{
public:
  I3InjectorService(const I3Context& context): I3ServiceBase(context) {}

  virtual ~I3InjectorService() {};

  /**
   * This method should be used to read in optional tank parameters
   * which can be defined in the constructor (as in I3Module)
   */
  virtual void Configure() {};

  /**
     Reset the injector before proceeding to the next event. Derived
     classes are responsible for setting the first three arguments
     (run ID, event ID and primary particle). It is called at
     the beginning of I3TopSimulator::DAQ.
   */
  virtual bool NextEvent(int& runID, int& evtID, I3Particle& primary, I3FrameConstPtr frame) = 0;

  /**
     Get the next particle in line. If there are no more particles,
     return false. Otherwise return true.

     Note that the parameter is of type
     ExtendedI3Particle. ExtendedI3Particle extends the functionality
     of I3Particle by adding an enumeration type that
     corresponds to the air shower component.  It is the
     responsibility of the derived classes to specify the shower
     component of each particle. It is "Undefined" by default.

     One can assign particles to a categories. How this category is
     chosen is implementation specific as it depends on the code used
     to simulate the air shower. For example, one could split
     particles into EM component or muon component (where the
     electrons from muon decay are categorized in the muon component),
     or we could categorize muons as "prompt" and "non prompt".
   */
  virtual bool NextParticle(ExtendedI3Particle& particle) = 0;

  /**
     Return additional event info. This is implementation dependent,
     as each service can define its own frame object type to hold the
     event info.

     This method is called at the end of I3TopSimulator::DAQ to add this map to the frame.
   */
  virtual I3FrameObjectConstPtr GetEventInfo() {return I3FrameObjectConstPtr();};

  /**
     Set pointer to the tank response service.

     This is a bit of a hack. It is done because CORSIKA files can be
     very large and iterating over all particles and all tanks can be
     very costly. A solution is to index tanks according to positions
     once and then iterate over a subset of tanks for each
     particle. This indexing happens inside I3IceTopResponseService.
     This method essentially breaks the encapsulation of
     responses and injectors.

     This method is called in I3TopSimulator::DetectorStatus.
   */
  void SetResponseService(I3IceTopResponseServicePtr response) {responseService_ = response;};

  /**
     Get a map of shower component name to enum.

     This method provides a mapping between the
     ExtendedI3Particle::AirShowerComponent enumeration value and the
     meaning of the category in human readable form. This helps to
     make i3 files self-documenting.

     This method is called at the end of I3TopSimulator::DAQ to add this map to the frame.
   */
  virtual std::map<std::string, int> GetAirShowerComponentNameMap() const = 0;

protected:

  template <class ParamType>
  void AddParameter(const std::string& name,
                    const std::string& description,
                    const ParamType& defaultValue)
  { I3ServiceBase::AddParameter<ParamType>(name, description, defaultValue); }

  template <class ParamType>
  void GetParameter(const std::string& name, ParamType& value) const
  { I3ServiceBase::GetParameter<ParamType>(name, value); }

  const I3Context& GetContext()
  { return I3ServiceBase::GetContext(); }

  I3IceTopResponseServicePtr responseService_;

  SET_LOGGER("I3InjectorService");
};

I3_DEFAULT_NAME(I3InjectorService);
I3_POINTER_TYPEDEFS(I3InjectorService);


#endif
