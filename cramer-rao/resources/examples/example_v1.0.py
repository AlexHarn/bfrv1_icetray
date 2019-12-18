#!/usr/bin/env python

"""
Cramer-Rao is an estimator based information thoery 

To use this example, either change those five input
parameters or use sys.argv provided.
After running this example script, output estimate parameters
exist in frame["CramerRaoParams"] in Physics frames in output i3 file.

Input:
gcdfile <.i3/.i3.bz2/.i3.gz>: /path/to/input/gcd_i3file
inputfile <.i3/.i3.bz2/.i3.gz>: /path/to/input/i3file
outfile <.i3/.i3.bz2/.i3.gz>: /path/to/output/i3file
InitialHitSeriesReco <String>: pulses map <I3RecoPulseSeriesMapMask>
TrackLlhFit <String>: track info (Zenith, Azimuth, etc.) <I3Particle>

Return:
CramerRaoParams <CramerRaoParams>: variances info

Output:
outfile <.i3/.i3.bz2/.i3.gz>: /path/to/output/i3file
"""

from I3Tray import I3Tray
from icecube import icetray,dataclasses,dataio,cramer_rao,paraboloid
from os.path import expandvars

import os
import sys

__author__ = "Chujie Chen"
__version__ = "1.0"
__date__ = "12/18/2019"

#### change input parameters yourself ####
# comment out below if you need to use argv input instead
gcdfile = "/path/to/input/gcd_i3file"
inputfile = "/path/to/input/i3file"
outfile = "/path/to/output/i3file"
InitialHitSeriesReco = "frameKeyOfPulsesMap"
TrackLlhFit = "frameKeyOfTrackInfo"
# comment out above if you need to use argv input instead

#### sys.argv ####
# uncomment below if you need to use argv input
"""
# Retrieve arguments
if len(sys.argv) <= 5:
    print('\nERROR:\nRequired arguments: gcdfile inputfile outputfile pulsesKey trackKey\n')
    raise ValueError("ERROR: arguments not passed")

gcdfile = sys.argv[1]
inputfile = sys.argv[2]
outfile = sys.argv[3]
InitialHitSeriesReco = sys.argv[4]
TrackLlhFit = sys.argv[5]
# abandoned parameters: see deprecated example file for more details
## datatype = sys.argv[4]
## outrootfile = outfile + ".root"
"""
# uncomment above if you need to use argv input

if os.access(outfile,os.R_OK) == True:
    raise RuntimeError("ERROR: output file already exists!")

tray = I3Tray()
tray.Add("I3Reader","reader",
        FilenameList = [gcdfile, inputfile]
    )
tray.Add("CramerRao","cr",
        InputResponse = InitialHitSeriesReco,
        InputTrack = TrackLlhFit,
        OutputResult = "CramerRao",
        AllHits = True
    )
tray.Add("I3Writer","writer",
         filename= outfile,
        Streams=[icetray.I3Frame.TrayInfo, icetray.I3Frame.DAQ, icetray.I3Frame.Physics])
tray.Execute()


