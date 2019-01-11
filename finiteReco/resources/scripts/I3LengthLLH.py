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
load("libgulliver")
load("libgulliver-modules")
load("liblilliput")
load("libphys-services")
load("libfiniteReco")

###############################################################################
# This script explains how to use I3LengthLLH.                                # 
###############################################################################

filename=expandvars("$I3_TESTDATA/event-viewer/Level3aGCD_IC79_EEData_Run00115990.i3")
outfile="I3LengthLLH.i3"

if len(sys.argv)>1:
    filename = sys.argv[1];
if len(sys.argv)>2:
    outfile=sys.argv[2]

if os.access(filename,os.R_OK) == False:
    raise IOError("cannot find input file!")

tray = I3Tray()

tray.AddModule("I3Reader", "reader")(
   ("filename", filename ),
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
# We need a likelihood the length calculation uses. It must be sensitive to the position of the interaction vertex and the stop point. In general I3LengthLLH works with all likelihood services for gulliver
tray.AddService("I3GulliverFinitePhPnhFactory","finitephpnh")(
    ("InputReadout","OfflinePulsesHLC"),
    ("NoiseRate",700.0e-9),#GHz
    ("StringLLH","true")
    )
###############################################################################
# Here the module is added                                                    #
# find further explanation at                                                 #
#        https://wiki.icecube.wisc.edu/index.php/FiniteReco.I3LengthLLH       #
###############################################################################
tray.AddModule("I3LengthLLH","lengthllh")(
    ("InputName","linefit_Finite"),
    ("StepSize",50*I3Units.m),
    ("ServiceName","finitephpnh")
    )

tray.AddModule("I3Writer","writer")(
    ("FileName",outfile)
    )



# do 10 events
tray.Execute(10+3)


import os
os.unlink(outfile)

