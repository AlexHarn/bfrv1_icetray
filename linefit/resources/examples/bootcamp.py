#!/usr/bin/env python
#
iftree    = True
ifwritei3 = False
# the first file has a lot of trays first
ncount=100
#
outfile = "my.i3"
rootfile = "my.root"
#
from I3Tray import *
from os.path import expandvars
from glob import glob

load("libicetray")
load("libdataclasses")
load("libphys-services")
load("libdataio")
load("libamanda-core")
load("libDOMcalibrator")
load("libFeatureExtractor")
load("libDomTools")
load("libanalysis-tree")
load("liblinefit")

tray = I3Tray()

#---Set variables......
ir = expandvars("$IR")
workspace = ir + "/trunk"
tools = expandvars("$I3_PORTS")

#-------                   icecube 80 string anis-mu (V1-9-4)
nstr    = 40
data    = "/Users/schlenst/prog/ice/data/icesim/2011/"
cdsfile = glob(data + "GeoCalibDetectorStatus_000159.i3.gz")
infiles = glob(data + "nugen_numu.000159.*.i3.gz")
infiles.sort()
#
#---Services......

tray.AddModule("I3Reader", "i3reader")(
    ("filenamelist", cdsfile + infiles),
    ("SkipUnregistered",True))

tray.AddService("I3RootTreeServiceFactory","treeservice")(
    ("TreeFileName", rootfile ))

#---Modules......

tray.AddModule("I3EventCounter", "counter")(
    ("CounterStep", ncount ))

#-- bad DOM cleaning
tray.AddModule("I3DOMLaunchCleaning","launchcleaning")(
    ("InIceInput","InIceRawData"),
#        ("IceTopInput","IceTopRawData"),
    ("InIceOutput","CleanInIceRawData"),
    ("FirstLaunchCleaning",True),
    ("CleanedKeys",[OMKey(21,30),  OMKey(29,59), OMKey(29,60),
                    OMKey(30,23),  OMKey(38,59), OMKey(39,8)])
    )

tray.AddModule("I3DOMcalibrator","calibrate")(
    ("InputRawDataName","CleanInIceRawData"))

tray.AddModule("I3FeatureExtractor","features")(
    ("RawReadoutName","CleanInIceRawData"),
    ("InitialPulseSeriesReco","HitsExtracted"),
    ("FastFirstPeak",1+2+8),
    ("FastPeakUnfolding",0),
    ("ADCThreshold",1),
    )

if nstr == 16:
    hitsForHitC = "HitsCutGeom"
    EraseStrings = [5,6,11,12,13,15,16]
    tray.AddModule("I3OMSelection<I3RecoPulseSeries>","badstrings")(
	("OmittedStrings", EraseStrings ),
#    ("OutputOMSelection", "ListOfBadDOMs"), 
        ("InputResponse", "HitsExtracted"),
	("OutputResponse", hitsForHitC ))
else:
    hitsForHitC = "HitsExtracted"

#--- Hit cleaning: distance (AMANDA) or Coincify (IceCube)
tray.AddModule("I3Coincify<I3RecoPulse>","coincify")(
    ("InputName", hitsForHitC),
    ("OutputName", "HitsPhysics"),
    ("CoincidenceNeighbors", 2),
    ("CoincidenceWindow", 1000))

#---LF
tray.AddModule("I3LineFit","LF")(
    ("Name", "LineFit"),
    ("InputRecoPulses","HitsPhysics"),
    ("MinHits", 8))
#---phys-services
tray.AddModule("I3CutsModule","LFcuts")(
    ("PulsesName", "HitsPhysics"),
    ("ParticleNames", "LineFit"))

#---analysis-tree
if iftree:
    tray.AddModule("I3MCTreeModule","mctru")(
#        ("MCName", "MMCI3ParticleList"),
        ("ParticleTypes", "MuPlus,MuMinus"))

    tray.AddModule("I3RootTreeModule","tracktree")(
        ("ClassNames", "I3Particle,I3CutValues"),
        ("KeyNames","*Params"))
#        ("KeyNames","DirectWalkParams")

#---end
tray.AddModule("Dump","dumpe")

if ifwritei3:
   tray.AddModule("I3Writer","writer")(
	 ("filename", outfile ))


tray.Execute(ncount)


print()
print("Input file   ", infiles)
if iftree:
   print("Root file    ", rootfile)

print()
