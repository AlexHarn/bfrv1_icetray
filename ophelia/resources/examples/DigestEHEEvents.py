#!/usr/bin/env python

from I3Tray import *
from os.path import expandvars

gcdfile = "GCD.i3.gz"
physfile  = "juliet.i3.gz"

if len(sys.argv) > 1:
	gcdfile = sys.argv[1]
if len(sys.argv) > 2:
	physfile = sys.argv[2]

load("libicetray")
load("libdataclasses")
load("libphys-services")
load("libdataio")
load("libpfclasses")
load("libjuliet-interface")
load("libromeo-interface")
load("libportia")
load("libophelia")

tray = I3Tray()

tray.AddModule("I3Reader","reader")(
    ("FilenameList", [gcdfile, physfile]))

tray.AddModule("I3EHEEventDigest","digest")(
    ("GeometryShiftX", 0.0),
    ("GeometryShiftY", 0.0),
    ("GeometryShiftZ", 0.0),

    ("digestJulietParticle", False),

    ("digestPortiaPulse", True),
    ("inInIceDOMLaunchName","CleanInIceRawData"),
    ("inAtwdPortiaName","ATWDPortiaPulse"),  
    ("inFadcPortiaName","FADCPortiaPulse"),

    ("digestEHEFirstGuess", True),
    ("inFirstguessName","OpheliaFirstGuess")
)

tray.AddModule("Dump","dump")


tray.Execute(10);

