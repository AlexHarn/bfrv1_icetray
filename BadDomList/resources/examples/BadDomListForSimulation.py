#!/usr/bin/env python                                                                                                              
import os
from os.path import expandvars
from I3Tray import I3Tray
from icecube import dataio, icetray
from icecube.BadDomList.BadDomListTraySegment import BadDomList

# Configs for input/output parameters
runId = 127797
gcd = expandvars("$I3_TESTDATA" + "/GCD/Level2_IC86.2015_data_Run00127797_21_242_GCD.i3.gz")
outFile = os.path.join(os.path.expandvars("$I3_BUILD"), "BDLTEST_%s" % os.path.basename(gcd))

# ---------------------- SIMULATED DATA --------------------------------------------
# Test for simulated data (parameter simulation = False)
# It doesn't matter for this example that the input GCD file is for experimental data
tray = I3Tray()
tray.AddModule('I3Reader', 'reader', 
    FilenameList = [gcd]
)

# Add the bad dom list tray segment
# If the GCD file doesn't have a run number, e.g. for simulated data,
# set the `Simulation` flag to `True`. This is important since the source
# of data is different.
tray.AddSegment(BadDomList, 'BadDomList',
    Simulation = True
)

# Write the updated GCD
tray.AddModule('I3Writer', 'writer',
    Filename = outFile,
    Streams = [icetray.I3Frame.Geometry,
               icetray.I3Frame.Calibration,
               icetray.I3Frame.DetectorStatus
    ]
)

tray.Execute()

