#!/usr/bin/env python

import os, sys
#import sys.argv

from stat import *
from os.path import expandvars
from os import unlink
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
# This script explains how to use I3GulliverFinitePhPnhFactory with gulliver. # 
###############################################################################

filename=expandvars("$I3_TESTDATA/event-viewer/Level3aGCD_IC79_EEData_Run00115990.i3")
outfile="GulliverFinitePhPnh.i3"

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

tray.AddService("I3GulliverMinuitFactory","minuit")(
    ("MaxIterations",50000),
    ("Tolerance",0.1),
    ("Algorithm","SIMPLEX")
    )

tray.AddService("I3SimpleParametrizationFactory","simpar")(
    ("StepX",20.0*I3Units.m),
    ("StepY",20.0*I3Units.m),
    ("StepZ",20.0*I3Units.m),
    ("StepZenith",0.1*I3Units.rad),
    ("StepAzimuth",0.2*I3Units.rad),
    )

tray.AddService("I3BasicSeedServiceFactory","seedserve")(
    ("FirstGuess","linefit"),
    ("InputReadout","OfflinePulsesHLC")
    )

###############################################################################
# Here the service is added                                                   #
# find further explanation at http://web.physik.rwth-aachen.de/~icecubemgr/?n #
#                             =LowEnergy.LLHReconstructionOfFiniteTracks      #
###############################################################################
tray.AddService("I3GulliverFinitePhPnhFactory","finitephpnh")( 
    ("InputReadout","OfflinePulsesHLC"),
    ("NoiseRate",700.0e-9),#GHz
    ("StringLLH","true")
    )

tray.AddModule("I3SimpleFitter")(
    ("OutputName","simplefit_phpnh"),
    ("SeedService","seedserve"),
    ("Parametrization","simpar"),
    ("LogLikelihood","finitephpnh"),
    ("Minimizer","minuit")
    )

tray.AddModule("I3Writer","writer")(
    ("FileName",outfile)
    )



# do 10 events
tray.Execute(10+3)


# this is run as a test, so should clean up after itself
unlink(outfile)
