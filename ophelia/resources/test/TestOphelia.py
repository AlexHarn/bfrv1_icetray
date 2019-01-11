#!/usr/bin/env python
import os
import sys
from os.path import expandvars
	       
from I3Tray import *
from icecube import ophelia
from icecube import icetray

workspace = expandvars("$I3_BUILD")
i3testdata = expandvars("$I3_TESTDATA")

infile = i3testdata + "/2007data/2007_I3Only_Run109732_Nch20.i3.gz"
outphysfile  = workspace + "/TestOphelia.i3.gz"

tray = I3Tray()
 	
tray.AddModule("I3Reader","readerfactory")(
    ("Filename", infile)
    )

#**************************************************
# Build pulses
#**************************************************
tray.AddModule("I3Portia","pulse")(
    ("DataReadoutName", "InIceRawData"),
    ("ATWDWaveformName", "CalibratedATWD"),
    ("FADCWaveformName", "CalibratedFADC")
    )
 	
#**************************************************
# First guess by line fit
#**************************************************
tray.AddModule("I3EHEFirstGuess", "reco")(
    ("MinimumNumberPulseDom",  2),
    ("OutputFirstguessName",   "OpheliaFirstGuess"),
    ("InputPulseName1",        "ATWDPortiaPulse"),
    ("InputPulseName2",        "FADCPortiaPulse"),
    ("InputLaunchName",        "InIceRawData"),
    ("ChargeOption",           0),
    ("LCOption",               True)
    )

#**************************************************
# Convert I3OpheliaFirstGuessTrack to I3Particle
#**************************************************
tray.AddModule("I3OpheliaConvertFirstGuessTrack","convert")(
    ("InputOpheliaFGTrackName", "OpheliaFirstGuess"),
    ("OutputParticleName", "OpheliaParticle"),
    )

tray.AddModule("I3Writer","writer")(
    ("filename", outphysfile),
    ("streams",[icetray.I3Frame.Physics])
    )



tray.Execute(10)

