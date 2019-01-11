#!/usr/bin/env python

from I3Tray import *
from I3Units import *
from glob import glob
from os.path import expandvars


import os

physfile  = "MCEventsPhysics.i3.gz"
outputfile  = "SelectedMCEventsPhysics.i3.gz"

month = "0602"

if len(sys.argv) > 1:
#	physfile = sys.argv[1]
	month = sys.argv[1]
if len(sys.argv) > 2:
	outputfile = sys.argv[2]
level1dir = "/disk1/data/IceCube/RealData/2006/level1-ehe/" + month

file_list = glob(level1dir + "/Run*.i3.gz")


load("libicetray")
load("libdataclasses")
load("libphys-services")
load("libdataio")
load("libpfclasses")
load("libjuliet-interface")
load("libromeo-interface")
load("libDOMcalibrator")
load("libportia")
load("libophelia")


tray = I3Tray()


tray.AddModule("I3Reader","reader")(
#    ("SkipKeys", ["I3Calibration","I3DetectorStatus"]),
    ("SkipKeys", ["CalibratedATWD","CalibratedFADC","IceTopCalibratedATWD","IceTopCalibratedFADC"]),
#    ("OmitGeometry",True),
#    ("OmitCalibration",True),
#    ("OmitStatus",True),
#    ("Filename", physfile))
    ("FilenameList", file_list))

tray.AddModule("I3DOMcalibrator","calibrateandlisten")(
	("InputRawDataName","CleanInIceRawData"),
	("CalibrationMode", 0)
	)

tray.AddModule("I3Portia","pulse")(
  ("DataReadoutName",          "CleanInIceRawData"),

  ("ATWDPulseSeriesName",      "ATWDPulseSeries"),
  ("ATWDPortiaPulseName",      "ATWDPortiaPulse"),
#  ("ATWDWaveformName",         "CalibratedATWD"),
  ("ATWDWaveformName",         "CalibratedATWD"),
  ("ATWDBaseLineOption",       "eheoptimized"),
  ("ATWDThresholdCharge",      1.*I3Units.pC),
  ("ATWDLEThresholdAmplitude", 3.*I3Units.mV),

  ("UseFADC", True),
  ("FADCPulseSeriesName",      "FADCPulseSeries"),
  ("FADCPortiaPulseName",      "FADCPortiaPulse"),
  ("FADCWaveformName",         "CalibratedFADC"),
  ("FADCBaseLineOption",       "eheoptimized"),
  ("FADCThresholdCharge",      1.*I3Units.pC),
  ("FADCLEThresholdAmplitude", 3.*I3Units.mV),

  ("MakeBestPulseSeries",      False),
  ("BestPortiaPulseName",      "BestPortiaPulse")
	
)

tray.AddModule("I3EHEFirstGuess","reco")(

  ("MinimumNumberPulseDom",  4),
  ("OutputFirstguessName",   "OpheliaFirstGuess"),
  ("InputPulseName1",        "ATWDPortiaPulse"),
  ("InputPulseName2",        "FADCPortiaPulse"),
  ("InputLaunchName",        "CleanInIceRawData"),
  ("ChargeOption",           0),#0-either larger charge, 1-use input puse 1, 2-use input pulse 2
  ("LCOption",               False)
)

tray.AddModule("I3EHEEventSelector","selector")(
    ("setCriteriaOnJulietParticle", False),
    ("energyMin", 1.0E5*I3Units.GeV),
    ("energyMax", 1.0E12*I3Units.GeV),
    ("particleType", 6), # I3Particle::MuMinus
    ("cosZenithMin", -1.0),
    ("cosZenithMax", 1.0),
    ("AzimuthMin", 0.0*I3Units.deg),
    ("AzimuthMax", 360.0*I3Units.deg),
    ("distanceOfCascade",10.0*I3Units.kilometer),

    ("setCriteriaOnInIceDOMLaunch", True),
    ("inInIceDOMLaunchName","CleanInIceRawData"),  
    ("numberOfDOMsWithLaunch", 80),

    ("setCriteriaOnPortiaPulse", True),
    ("inAtwdPortiaName","ATWDPortiaPulse"),  
    ("inFadcPortiaName","FADCPortiaPulse"),
    ("lowestNPEs", 5.0e3),
    ("highestNPEs", 1.0e8),
    ("numberOfDOMs", 70),

    ("setCriteriaOnEHEFirstGuess", False),
    ("inFirstguessName","OpheliaFirstGuess"),
    ("firstGuessCosZenithMin", -0.2),
    ("firstGuessCosZenithMax", 0.2),
    ("firstGuessAzimuthMin", 0.0*I3Units.deg),
    ("firstGuessAzimuthMax", 360.0*I3Units.deg),
    ("distanceOfCOB",0.5*I3Units.kilometer),
    ("firstGuessVelocity",0.01*(I3Units.meter/I3Units.ns))
)


tray.AddModule("I3Writer","writer")(
#	("SkipKeys", ["I3Calibration","I3DetectorStatus"]),
        ("filename", outputfile)
)

tray.AddModule("Dump","dump")


tray.Execute();


