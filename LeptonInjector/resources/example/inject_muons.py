#!/bin/sh /cvmfs/icecube.opensciencegrid.org/py3-v4.0.1/icetray-start
#METAPROJECT /data/user/bsmithers/metaprojects/snobo/py3-v4.0.1/

# Ben Smithers
# benjamin.smithers@mavs.uta.edu

# This file creates an icetray with a MultiLeptonInjector object
# The MLI object is built with two distinct injectors:
#   + one with a ranged-mode, muon injector (tracks)
#   + and one with a volume-mode electron injector (cascades)

# It then writes two files: a data file and a configuration file
# the configuration file (a LIC file) can be read by LeptonWeighter 

from I3Tray import *
from icecube import icetray, dataio, dataclasses
from icecube import phys_services
from icecube import LeptonInjector 
from icecube.icetray import I3Units

seed = 15

# create an icetray 
tray = I3Tray()

# Add a random service, an Earth model, and a source of DAQ frames
randomService = phys_services.I3GSLRandomService(seed=seed)
tray.context["I3RandomService"] = randomService
tray.AddService("I3EarthModelServiceFactory", "Earth")
tray.AddModule("I3InfiniteSource", "TheSource", Stream=icetray.I3Frame.DAQ)

# specify where the cross sections are
#   alternatively, you can use the test cross sections included in  the resources directory 
xs_folder = "/cvmfs/icecube.opensciencegrid.org/data/neutrino-generator/cross_section_data/csms_differential_v1.0/"

# we create a list of injector objects
#   each of these injectors can have a unique cross sections used 
injector_list = []
injector_list.append(
    LeptonInjector.injector(
        NEvents         = 100,
        FinalType1      = dataclasses.I3Particle.ParticleType.MuMinus,
        FinalType2      = dataclasses.I3Particle.ParticleType.Hadrons,
        DoublyDifferentialCrossSectionFile  = xs_folder + "/dsdxdy_nu_CC_iso.fits",
        TotalCrossSectionFile               = xs_folder + "/sigma_nu_CC_iso.fits",
        Ranged = True)
    )
injector_list.append(
    LeptonInjector.injector(
        NEvents         = 100,
        FinalType1      = dataclasses.I3Particle.ParticleType.EMinus,
        FinalType2      = dataclasses.I3Particle.ParticleType.Hadrons,
        DoublyDifferentialCrossSectionFile  = xs_folder + "/dsdxdy_nu_CC_iso.fits",
        TotalCrossSectionFile               = xs_folder + "/sigma_nu_CC_iso.fits",
        Ranged = False)
    )

# Create the multileptoninjector object with your list of injectors 
tray.AddModule("MultiLeptonInjector",
    EarthModel      = "Earth",
    Generators      = injector_list,
    MinimumEnergy   = 100. * I3Units.GeV,
    MaximumEnergy   = (1e6) * I3Units.GeV,
    MinimumZenith   = 90 * I3Units.deg,
    MaximumZenith   = 180 * I3Units.deg, 
    PowerLawIndex   = 2.,
    InjectionRadius = 1200 * I3Units.meter,
    EndcapLength    = 1200 * I3Units.meter,
    CylinderRadius  = 800 * I3Units.meter,
    CylinderHeight  = 800 * I3Units.meter,
    MinimumAzimuth  = 0. * I3Units.deg,
    MaximumAzimuth  = 360. * I3Units.deg,
    RandomService   = "I3RandomService")

# a little function to add the event number and seed to each frame 
event_id = 1
def get_header(frame):
    global event_id 
    header          = dataclasses.I3EventHeader()
    header.event_id = event_id
    header.run_id   = seed
    frame["I3EventHeader"] = header

    event_id += 1

tray.AddModule(get_header, streams = [icetray.I3Frame.DAQ])

# write two files: one the configuration file for use with LeptonWeighter
tray.AddModule("InjectionConfigSerializer", OutputPath ="./example_config.lic")
# Write the datafile 
tray.AddModule("I3Writer", Filename="./example_out.i3.zst",
        Streams=[icetray.I3Frame.TrayInfo, icetray.I3Frame.Simulation,
                 icetray.I3Frame.DAQ, icetray.I3Frame.Physics])

tray.Execute()
tray.Finish()
