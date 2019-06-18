#!/usr/bin/env python                                                                                                              
import os
from os.path import expandvars
from I3Tray import I3Tray
from icecube import dataio, icetray
from icecube.BadDomList.BadDomListTraySegment import BadDomList

# Configs for input/output parameters
runId = 127797
gcd = expandvars('$I3_TESTDATA/GCD/Level2_IC86.2015_data_Run00127797_21_242_GCD.i3.gz')
outFile = os.path.join(os.path.expandvars("$I3_BUILD"), "BDLTEST_%s" % os.path.basename(gcd))

# The tray...
# ---------------------- EXPERIMENTAL DATA -----------------------------------------
# Test for experimental data (parameter simulation = False)
tray = I3Tray()
tray.AddModule('I3Reader', 'reader', 
    FilenameList = [gcd]
)

# Add the bad dom list tray segment
# Since it is a GCD file for experimental data, it has
# a run number. Let the `Simulation` flag be `False` (default)
# and pass teh run number. Now it gets the information
# form I3Live.
tray.AddSegment(BadDomList, 'BadDomList',
    RunId = runId
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


