#!/usr/bin/env python

from I3Tray import *
from icecube.icetray import I3Units
from math import *
from os.path import expandvars
import glob
import os

gcd_file = "Level1_IC59_data_Run00115514_GCD.i3.gz"
physFileList  = "Run00115514.i3"

if len(sys.argv) > 1:
        physFileList = sys.argv[1]


load("libicetray")
load("libdataclasses")
load("libphys-services")
load("libdataio")
load("libophelia")

if len(sys.argv) > 1:
        physFileList = sys.argv[1]
if len(sys.argv) > 2:
        gcd_file = sys.argv[2]

file_list =  glob.glob(physFileList)
file_list.sort()
file_list.insert(0,gcd_file)


tray = I3Tray()

tray.AddModule('I3Reader', 'reader', FilenameList=file_list)

tray.AddModule("I3EHEEventDigest","digest")(

    ("digestJulietParticle", False), # Digest MC truth concerning JULIeT
    ("digestMMCTrack", False),       # Digest MC truth concerning MMC

    ("digestPortiaPulse", True),    # Digest NPE etc calculated by Portia
    ("InputPortiaEventName","EHESummaryPulseInfoEnergyReco"),  # PortiaEvent dataclass
    ("inAtwdPortiaName","ATWDPortiaPulseEnergyReco"),          # PortiaPulse ATWD
    ("inFadcPortiaName","FADCPortiaPulseEnergyReco"),          # PortiaPulse FADC

    ("digestEHEFirstGuess", False),  # Digest reco results by Ophelia
    ("inFirstguessName","OpheliaFirstGuessBaseTimeWindow"), #Firstguess with the BTW cleaning

    ("digestRecoTrack", True),  # Digest reco results by Ophelia
    ("inRecoTrackName","SPEFit8"),

    ("digestMillipede", True),  # Digest results by Millipede
#    ("inMillipedeParticlesName","MillipedeAmpsPortiaOphelia"),
    ("inMillipedeParticlesName","MillipedeAmpsPortia"),

    ("digestExecutiveSummary", False),# Digest Executive Summary of an event
    ("baseTimeWindowCleaning", True) # the window cleaing - default is "True"
)

tray.AddModule("Dump","dump")


tray.Execute();

