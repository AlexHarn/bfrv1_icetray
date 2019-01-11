#!/usr/bin/env python

from I3Tray import *

from os.path import expandvars

import os
import sys

load("libdataclasses")
load("libdataio")
load("libtriggerUtil-C++")
load("libphys-services")
load("libDOMcalibrator")
load("libFeatureExtractor")
load("liblinefit")
load("libparalysis")
load("libDomTools")
true = 1
false = 0
workspace = expandvars("$I3_SRC")
tools = expandvars("$I3_PORTS")

datadir = "/data/rodin/data/sim/IceSim/IceCube/2010/dataset_140/"
datadir = "/data/exp/sim/IceCube/2006/generated/dataset_10/"

runTag = "simplemuon.1.i3.gz"

datafile = datadir + runTag

mbids = workspace + "/phys-services/resources/doms.txt"
amageofile = workspace + "/phys-services/resources/amanda.geo"
icegeofile = workspace + "/phys-services/resources/icecube.geo"

tray = I3Tray()
#....
if 1 :
	tray.AddService("I3FileOMKey2MBIDFactory","omkey2mbid")(
		("Infile",mbids)
		)

if 1 :
	tray.AddModule("I3Reader","reader")(
		("Filename",datafile),
		("SkipUnregistered",True),
		)

if  1 :
	tray.AddModule("I3DOMLaunchCleaning","launchcleaning")(
		("InIceInput","InIceRawData"),
		("IceTopInput","IceTopRawData"),
		("InIceOutput","CleanInIceRawData"),
		("IceTopOutput","CleanIceTopRawData"),
		("FirstLaunchCleaning",True) #,
		#("CleanedKeys",[OMKey(21,30),
		#	OMKey(29,59),
		#	OMKey(29,60),
		#	OMKey(30,23),
		#	OMKey(38,59),
		#	OMKey(39,8),
		#	OMKey(40,51),
		#	OMKey(40,52),
		#	OMKey(40,54), # pika, tmp
		#	OMKey(50,36),
		#	OMKey(50,58),
		#	OMKey(59,51),
		#	OMKey(59,52)])
		)

	tray.AddModule("I3DOMcalibrator","calibrateandlisten")(
		("InIceLaunches","CleanInIceRawData"),
		("IceTopLaunches","CleanIceTopRawData"),
		)

	tray.AddModule("I3FeatureExtractor","features")(
		("InitialHitSeriesReco","FE-Hit-Series"),
		("RawReadoutName","CleanInIceRawData"),
		("MaxNumHits",0),
		("FastFirstPeak",3),
		("FastPeakUnfolding", 0 ),
		)

if 0 :	
	
	tray.AddModule("I3FeatureExtractor","feature")(
		("InitialHitSeriesReco","FE-Hit-Series"),
		("InitialPulseSeriesReco","InitialPulseSeriesReco"),
		("RawReadoutName","InIceRawData"),
		# ("CalibratedFADCWaveforms","CalibratedFADC"),
		("CalibratedATWDWaveforms","CalibratedATWD"),
		("MaxNumHits", 0),            
		("FastFirstPeak", 2),         
		("FastPeakUnfolding", 0),
		("ADCThreshold", 1)
		)

if 1 :
	tray.AddModule("I3LineFit","linefit")(
		("RecoHitsName","FE-Hit-Series")
		)

if 1 :
	tray.AddModule("I3Panalysis", "panalysis")(
		("RootFilename","LineFit_timing.root"),
    		("MCName", "MCList" ),
    		("AsciiFilename", "LineFit_timing.txt" ),
      		("RecoName", "LineFit" ),
		("ParticleType", "MuMinus:MuPlus")
		)


#tray.AddModule("I3Writer","writer")(
#    ("filename", "cvertex_flasher1.i3")
#    )


tray.Execute(1000)

	
 
