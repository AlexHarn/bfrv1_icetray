#!/usr/bin/env python
#
#  Monopole Generator
#
#  $Id$
#

import I3Tray
from icecube import icetray, dataclasses, dataio, monopole_generator

nevents = 100;

tray = I3Tray.I3Tray()

# Add the random generator service
tray.AddService("I3GSLRandomServiceFactory","random")

# Make some empty frames
tray.AddModule("I3InfiniteSource", "infinite", Stream=icetray.I3Frame.DAQ)

# Below represents default values for all potential parameters
tray.AddModule("I3MonopoleGenerator","generator")(
# tree name mustn't be changed since other modules expect this name
    ("TreeName","I3MCTree"),
# distance of generation disc to icecube zero coordinates
    ("Disk_dist",1000*icetray.I3Units.m),
# radius of generation disc
    ("Disk_rad",800*icetray.I3Units.m),
# randomize position of monopole track on the generation disk
    ("Rand_pos", True),
# Name of a frame object containing all import simulation parameters
    ("InfoName", "MPInfoDict"),

# Either beta or gamma have to be set (bot not both!)
#   ("Gamma",10),
   ("BetaRange",[0.001]),    
#    ("BetaRange",[0.4,0.995]),

# Relativstic monopoles need the mass to be set
    ("Mass",(1e7)*icetray.I3Units.GeV),

# Those parameters are optional. Here's some choices though
#    ("ZenithRange", [0*icetray.I3Units.degree,180*icetray.I3Units.degree]),
#    ("AzimuthRange", [0*icetray.I3Units.degree,360*icetray.I3Units.degree]),


#   ("Rad_on_disk",0*icetray.I3Units.m),
#   ("Azi_on_disk",0*icetray.I3Units.radian),
#   ("Length",2000*icetray.I3Units.m),

# shift center to DeepCore (useful for IC86-I, the default is [0.,0.,0.] )
#    ("shiftCenter", [46.0*icetray.I3Units.meter, -34.5*icetray.I3Units.meter, -330.0*icetray.I3Units.meter]),

#    ("PowerLawIndex", 5),

    )

#tray.AddModule("I3Writer","writer")(
#    ("filename", "mp-gen.i3")
#    )

#tray.AddModule("Dump","dump")
tray.AddModule("TrashCan", "the can")

tray.Execute(nevents)
tray.Finish()
