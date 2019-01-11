#!/usr/bin/env python

from I3Tray import *
from I3Units import *
from glob import glob
from os.path import expandvars

mcdir = "/home/syoshida/icecube/i3files/IC22Corsika/"

#physfileList = "juliet_mu_e1_AllWeight.*.i3.gz"
physfileList = "juliet_mu_e2_AllWeight.*.i3.gz"

if len(sys.argv) > 1:
       physfileList = sys.argv[1]

file_list = glob(mcdir + physfileList)
file_list.sort()


gcd_file = "/home/syoshida/icecube/i3files/IC22Corsika/GeoCalibDetectorStatus_IC40_TWR.54649.i3.gz"

file_list.insert(0,gcd_file)


load("libicetray")
load("libdataclasses")
load("libsimclasses")
load("libphys-services")
load("libsim-services")
load("libdataio")
load("libpfclasses")
load("libjuliet-interface")
load("libromeo-interface")
load("libportia")
load("libophelia")

tray = I3Tray()

tray.AddModule("I3Reader","reader")(
    ("FilenameList", file_list))

stringsToUse = "21,29,30,38,39,40,49,50,59,58,67,66,74,73,65,72,78,48,57,47,46,56"
stationsToUse = "21,29,30,38,39,40,47,48,49,50,57,58,59,66,67,74,46,56,65,73,72,78,77,71,64,55"

tray.AddService("I3GeometrySelectorServiceFactory","geo-selector")(
       ("StringsToUse",stringsToUse),
       ("StationsToUse",stationsToUse),
       ("ShiftX",-310.),
       ("ShiftY",-189.),
       ("GeoSelectorName","IC22-IT26-Geo"))



tray.AddModule("I3MetaSynth", "multiplex")(
       ("GeometryService","IC22-IT26-Geo"))

tray.AddModule("I3EHEEventSelector","selector")(
       ("setCriteriaOnJulietParticle", False),
       ("setCriteriaOnInIceDOMLaunch", True),
       ("inInIceDOMLaunchName","CleanInIceRawData"),
       ("numberOfDOMsWithLaunch", 20),

       ("setCriteriaOnPortiaPulse", False),
       ("setCriteriaOnEHEFirstGuess", False))

tray.AddModule("I3EHEEventDigest","digest")(
    ("digestMMCTrack", True),
    ("inPrimaryCRTreeName","I3MCTree"),
    ("inMMCTrackListName","MMCTrackList"),
    
    ("digestPortiaPulse", True),
    ("inInIceDOMLaunchName","CleanInIceRawData"),
    ("inAtwdPortiaName","ATWDPortiaPulse"),  
    ("inFadcPortiaName","FADCPortiaPulse"),

    ("digestEHEFirstGuess", True),
    ("inFirstguessName","OpheliaFirstGuess")
)

tray.AddModule("Dump","dump")


tray.Execute();

