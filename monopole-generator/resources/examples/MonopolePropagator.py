#!/usr/bin/env python
#
#  Monopole Propagator example script
#
#  $Id: MonopolePropagator.py 124987 2014-10-23 15:09:51Z jacobi $
#

import I3Tray
from icecube import icetray, dataclasses, dataio, monopole_generator, monopole_propagator

nevents = 100;
fastMonopole=False # set to false for slow monopole example


if fastMonopole:
    beta=0.70
else:
    beta=0.001

tray = I3Tray.I3Tray()

# Add the random generator service
tray.AddService("I3GSLRandomServiceFactory","random")

# Make some empty frames
tray.AddModule("I3InfiniteSource", "infinite", Stream=icetray.I3Frame.DAQ)

# Fill them with monopoles
tray.AddModule("I3MonopoleGenerator","generator",
    BetaRange=[beta],
    Mass=(1e7)*icetray.I3Units.GeV,
)

# Below represents default values for all potential parameters
tray.AddModule("I3MonopolePropagator","propagator")(

# Defaults:
#    ("InputTreeName","I3MCTree"),
# change the following to keep the info in the first tree
#    ("OutputTreeName","I3MCTree"),
# very important: needs to be the same name as in generator, else you get wrong results!!
#    ("InfoName", "MPInfoDict"),
# dont change the following unless you know what you do (defaults are shown here)

#   ("IncludeErrorChecking", true),

# for fast monopoles
#   ("CalculateEnergy", True),
#   ("CalculateDensityCorrection", True),
#   ("StepSize", NAN),
#   ("SpeedMin", 0.2*icetray.I3Constants.C),
#   ("MaxLength", 10*icetray.I3Units.m),
#   ("MinLength", 0.001*icetray.I3Units.m),
#   ("MaxDistanceFromCenter", 1000*icetray.I3Units.m),
#   ("Profiling", False),

# for slow monopoles
    ("MeanFreePath", 0.1*icetray.I3Units.meter),  # default is NaN, must be always configured if dealing with SLOPs
    ("UseCorrectDecay", False),
    ("ScaleEnergy", False),
    ("EnergyScaleFactor", 1.0),

)

#tray.AddModule("I3Writer","writer")(
#    ("filename", "mp-prop.i3")
#    )

#tray.AddModule("Dump","dump")
tray.AddModule("TrashCan", "the can")

tray.Execute(nevents)
tray.Finish()
