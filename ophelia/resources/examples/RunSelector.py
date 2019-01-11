#!/usr/bin/env python

from I3Tray import *
from I3Units import *
from os.path import expandvars


import os

#gcdfile = "GCD.i3.gz"
physfile  = "Juliet.i3.gz"
outputfile  = "SelectedJuliet.i3"

if len(sys.argv) > 1:
	physfile = sys.argv[1]
if len(sys.argv) > 2:
	outputfile = sys.argv[2]

load("libicetray")
load("libdataclasses")
load("libphys-services")
load("libdataio")
load("libjuliet-interface")
load("libromeo-interface")
load("libportia")
load("libophelia")


tray = I3Tray()


tray.AddModule("I3Reader","reader")(
#    ("FilenameList", [gcdfile, physfile])
    ("Filename", physfile)
    )

tray.AddModule("I3Portia","pulse")(
  ("DataReadoutName",          "InIceRawData"),

  ("ATWDPulseSeriesName",      "ATWDPulseSeries"),
  ("ATWDPortiaPulseName",      "ATWDPortiaPulse"),
  ("ATWDWaveformName",         "CalibratedATWD"),
  ("ATWDBaseLineOption",       "zerobaseline"),
  ("ATWDThresholdCharge",      1.*I3Units.pC),
  ("ATWDLEThresholdAmplitude", 3.*I3Units.mV),

  ("UseFADC", True),
  ("FADCPulseSeriesName",      "FADCPulseSeries"),
  ("FADCPortiaPulseName",      "FADCPortiaPulse"),
  ("FADCWaveformName",         "CalibratedFADC"),
  ("FADCBaseLineOption",       "zerobaseline"),
  ("FADCThresholdCharge",      1.*I3Units.pC),
  ("FADCLEThresholdAmplitude", 1.*I3Units.mV),

  ("MakeBestPulseSeries",      False),
  ("BestPortiaPulseName",      "BestPortiaPulse")
	
)

tray.AddModule("I3EHEFirstGuess","reco")(

  ("MinimumNumberPulseDom",  4),
  ("OutputFirstguessName",   "OpheliaFirstGuess"),
  ("InputPulseName1",        "ATWDPortiaPulse"),
  ("InputPulseName2",        "FADCPortiaPulse"),
  ("InputLaunchName",        "InIceRawData"),
  ("ChargeOption",           0),#0-either larger charge, 1-use input puse 1, 2-use input pulse 2
  ("LCOption",               False)
)

tray.AddModule("I3EHEEventSelector","selector")(
    ("setCriteriaOnJulietParticle", True),
    ("energyMin", 1.0E5*I3Units.GeV),
    ("energyMax", 1.0E12*I3Units.GeV),
    ("particleType", 6), # I3Particle::MuMinus
    ("cosZenithMin", -1.0),
    ("cosZenithMax", 1.0),
    ("AzimuthMin", 0.0*I3Units.deg),
    ("AzimuthMax", 360.0*I3Units.deg),
    ("distanceOfCascade",10.0*I3Units.kilometer),

    ("setCriteriaOnInIceDOMLaunch", True),
    ("inInIceDOMLaunchName","InIceRawData"), 
#    ("numberOfDOMsWithLaunch", 80),
    ("numberOfDOMsWithLaunch", 3),

    ("setCriteriaOnPortiaPulse", True),
    ("inAtwdPortiaName","ATWDPortiaPulse"),  
    ("inFadcPortiaName","FADCPortiaPulse"),
    ("lowestNPEs", 1.0e3),
    ("highestNPEs", 1.0e4),
#    ("numberOfDOMs", 80),
    ("numberOfDOMs", 3),

    ("setCriteriaOnEHEFirstGuess", True),
    ("inFirstguessName","OpheliaFirstGuess"),
    ("firstGuessCosZenithMin", -0.2),
    ("firstGuessCosZenithMax", 0.2),
    ("firstGuessAzimuthMin", 0.0*I3Units.deg),
    ("firstGuessAzimuthMax", 360.0*I3Units.deg),
    ("distanceOfCOB",0.5*I3Units.kilometer),
    ("firstGuessVelocity",0.01*(I3Units.meter/I3Units.ns))
)


tray.AddModule("I3Writer","writer")(
        ("filename", outputfile),
)

tray.AddModule("Dump","dump")


tray.Execute(100);


