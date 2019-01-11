#!/usr/bin/env python

from I3Tray import *
from icecube.icetray import I3Units
from math import *
from os.path import expandvars
import glob
import os

eve_num = 10

gcd_file = "GeoCalibDetectorStatus_IC86.55040_official.i3.gz"
physFileList  = "juliet_mu_slope1_ic86.000000.*.i3.gz"
outrootfile = "julietWithEHECriteria.root"

if len(sys.argv) > 1:
        physFileList = sys.argv[1]


load("libicetray")
load("libdataclasses")
load("libphys-services")
load("libdataio")
load("libpfclasses")
#load("libjuliet-interface")
load("libportia")
load("libophelia")
load("libtree-maker")

gcddir = "/media/logitec/icecube/data/MCData/86strings/trunk_new/"
gcd_file = gcddir + gcd_file

datadir = "/media/logitec/icecube/data/MCData/86strings/trunk_new/juliet/muon/slope-1/"
file_list =  glob.glob(datadir + physFileList)
file_list.sort()
file_list.insert(0,gcd_file)


tray = I3Tray()

tray.AddService("I3ReaderServiceFactory","reader")(
	("FilenameList", file_list)
)

tray.AddModule("I3Muxer","muxer")

tray.AddModule("I3EHEEventSelector","selector")(
    ("setCriteriaOnJulietParticle", False),
    ("setCriteriaOnInIceDOMLaunch", False),
    ("setCriteriaOnPortiaPulse", False),
    ("setCriteriaOnEHEFirstGuess", False),

    ("setEHECriteria", True),  # EHE cut defined by Ophelia's EHECriteria class
#   ("EHECriteriaFileName", "IC86Criteria.txt"), # Criteria file 
                                                # No need if you stay with the default EHE criteria

    ("InputPortiaEventName","EHESummaryPulseInfo"),  # PortiaEvent dataclass
    ("inAtwdPortiaName","ATWDPortiaPulse"),          # PortiaPulse ATWD
    ("inFadcPortiaName","FADCPortiaPulse"),          # PortiaPulse FADC
    ("inFirstguessName","OpheliaFirstGuessBaseTimeWindow"), #Firstguess with the BTW cleaning
)


tray.AddModule("I3EHEEventDigest","digest")(

    ("digestJulietParticle", False), # Digest MC truth concerning JULIeT
    ("digestMMCTrack", False),       # Digest MC truth concerning MMC

    ("digestPortiaPulse", True),    # Digest NPE etc calculated by Portia
    ("InputPortiaEventName","EHESummaryPulseInfo"),  # PortiaEvent dataclass
    ("inAtwdPortiaName","ATWDPortiaPulse"),          # PortiaPulse ATWD
    ("inFadcPortiaName","FADCPortiaPulse"),          # PortiaPulse FADC

    ("digestEHEFirstGuess", True),  # Digest reco results by Ophelia
    ("inFirstguessName","OpheliaFirstGuessBaseTimeWindow"), #Firstguess with the BTW cleaning

    ("digestExecutiveSummary", True),# Digest Executive Summary of an event
    ("baseTimeWindowCleaning", True) # the window cleaing - default is "True"
)

#***************************************************************
# Root Tree Maker
#***************************************************************
tray.AddModule("I3TreeMakerModule","tree-maker")(
    ("outTreeName",            "JulietTree"),
    ("outFileName",            outrootfile),

    ("doJulietTree",         True),
    ("inMCHitSeriesName",      "MCHitSeriesMap"),
    ("inMCTreeName",           "I3MCTree"),
    ("frameMCWeightName",      "JulietWeightDict"),
    ("frameCosmicRayEnergyWeightName", "CosmicRayEnergyDist"),
    ("framePropagationMatrixVectorName","PropagationMatrix"),
    ("inAtmosphericMuonFluxName","ElbertModelIC22"),
    
    ("doDetectorTree",         True),
    ("inDOMLaunchName",        "InIceRawData"),
    #("doPulseChannelTree",     True),
    ("inAtwdPortiaName",       "ATWDPortiaPulse"),
    ("inFadcPortiaName",       "FADCPortiaPulse"),
    ("inPortiaEventName",      "EHESummaryPulseInfo"),
    #("doWaveformDraw",         True),

    ("doFirstguessTree",       True),
    ("inFirstguessName",       "OpheliaFirstGuess"),
    ("inFirstguessNameBtw",       "OpheliaFirstGuessBaseTimeWindow"),
    
    )

#tray.AddModule("I3EventCounter", "counter")(
#    ("NEvents", eve_num)
#    )

tray.AddModule("Dump","dump")


tray.Execute();


