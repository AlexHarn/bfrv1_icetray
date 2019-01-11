#!/usr/bin/env python

import os, sys
#import sys.argv

from stat import *
from os.path import expandvars
from icecube import dataio

from I3Tray import *

load("libdataclasses")
load("libdataio")
load("liblinefit")
load("libphys-services")
load("libfiniteReco")
###############################################################################
# This script explains how to use I3StartStopLProb.                           # 
###############################################################################

filename=expandvars("$I3_TESTDATA/event-viewer/Level3aGCD_IC79_EEData_Run00115990.i3")
outfile="StartStopLProb.i3"

if len(sys.argv)>1:
    filename = sys.argv[1];
if len(sys.argv)>2:
    geofile=sys.argv[2]
if len(sys.argv)>3:
    outfile=sys.argv[3]

if os.access(filename,os.R_OK) == False:
    raise "cannot find input file %s !" % filename

tray = I3Tray()

tray.AddModule("I3Reader", "reader")(
    ("filenamelist", [filename] ),
    )

tray.AddModule("I3LineFit","linefit")(
    ("InputRecoPulses","OfflinePulsesHLC"),
    ("Name","linefit")
    )

# We need a interaction vertex and a stop point. See: I3StartStopPoint.py
tray.AddModule("I3StartStopPoint","getstartStop")(
    ("Name","linefit"),
    ("InputRecoPulses","OfflinePulsesHLC"),
    ("CylinderRadius",300*I3Units.m),
    ("ExpectedShape",70)
    )

# We need a likelihood for the calculation. It must be sensitive to the position of the interaction vertex and the stop point.
tray.AddService("I3GulliverFinitePhPnhFactory","finitephpnh")(
    ("InputReadout","OfflinePulsesHLC"),
    ("NoiseRate",700.0e-9),#GHz
    ("ProbName","PhPnhParam"),
    ("RCylinder",250*I3Units.m),
    ("StringLLH","true")
    )

###############################################################################
# Here the module is added                                                    #
# find further explanation at                                                 #
#     https://wiki.icecube.wisc.edu/index.php/FiniteReco.I3StartStopLProb     #
###############################################################################
tray.AddModule("I3StartStopLProb","StartStopLLHPhotorec")(
    ("Name","linefit_Finite"),
    ("ServiceName","finitephpnh")
    )

tray.AddModule("I3Writer","writer")(
    ("FileName",outfile))




# do 10 events
tray.Execute(10+3)


import os
os.unlink(outfile)

