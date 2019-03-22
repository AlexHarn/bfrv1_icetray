#!/usr/bin/python

import os
import sys,getopt
#import logging
from os.path import expandvars

from I3Tray import *
from icecube import icetray, dataclasses, dataio, toprec, recclasses, frame_object_diff
from icecube import gulliver, gulliver_modules, lilliput
from icecube.icetray import I3Module
from icecube.dataclasses import I3EventHeader, I3Particle
from icecube.recclasses import I3LaputopParams

## Are you using tableio?
from icecube.tableio import I3TableWriter
from icecube.rootwriter import I3ROOTTableService

## Set the log level
## Uncomment this in if you're doing code development!
icetray.set_log_level(icetray.I3LogLevel.LOG_INFO)
icetray.set_log_level_for_unit('Laputop', icetray.I3LogLevel.LOG_DEBUG)
icetray.set_log_level_for_unit('Curvature', icetray.I3LogLevel.LOG_DEBUG)

workspace = expandvars("$I3_BUILD")
HOME = expandvars("$HOME")

#### PUT YOUR FAVORITE GCD AND INPUT FILE HERE
# This particular example file lives in Madison.
GCDfile = "/data/sim/IceTop/2012/filtered/CORSIKA-ice-top/12360/level2/0000000-0000999/GeoCalibDetectorStatus_2012.56063_V1_OctSnow.i3.gz"
infile = "/data/user/kath/L3_big/12360/big_Level3_12360.i3"

#### NAME YOUR OUTPUT FILES HERE
# Put this someplace that exists!  (This particular one is for KR)
I3_OUTFILE = HOME+"/testout/toprec_testme.i3"
ROOTFILE = HOME+"/testout/toprec_testme.root"


tray = I3Tray()

########## SERVICES FOR GULLIVER ##########

datareadoutName="IceTopHLCSeedRTPulses"
badtanksName= "IceTopHLCSeedRTExcludedTanks"

## The "simple lambda" snowservice
#tray.AddService("I3SimpleSnowCorrectionServiceFactory","SimpleSnow21")(
#    ("Lambda", 2.1)
#    )

## This one is the standard one.
tray.AddService("I3GulliverMinuitFactory","Minuit")(
    ("MinuitPrintLevel",-2),  
    ("FlatnessCheck",True),  
    ("Algorithm","SIMPLEX"),  
    ("MaxIterations",1000),
    ("MinuitStrategy",2),
    ("Tolerance",0.01),    
    )

tray.AddService("I3CurvatureSeedServiceFactory","CurvSeed")(
    ("SeedTrackName", "Laputop"), # This is also the default
    ("A", 4.823e-4)            # This comes from the original IT-26 gausspar function 
)

tray.AddService("I3CurvatureParametrizationServiceFactory","CurvParam")(
    ("FreeA", True),       # Yeah, fit this one!
    ("MinA", 1.0e-4),      # This is also the default
    ("MaxA", 1.0e-3),      # This is also the default
    ("StepsizeA", 5.0e-6)  # This is also the default
    )

tray.AddService("I3LaputopLikelihoodServiceFactory","ToprecLike2")(
    ("datareadout", datareadoutName),
    ("badtanks", badtanksName),
    ("ldf", ""),      # do NOT do the LDF (charge) likelihood
    ("curvature","gaussparfree")      # yes, do the CURVATURE likelihood
#    ("SnowServiceName","SimpleSnow21")
    )


#------------------- LET'S RUN SOME MODULES!  ------------------

#**************************************************
#                 Reader and whatnot
#**************************************************
tray.AddModule("I3Reader","reader")(
    ("FileNameList", [infile])
    )

## Re-inflate the GCD from the diff's
from icecube.frame_object_diff.segments import uncompress
tray.AddSegment(uncompress, "uncompressme", base_filename=GCDfile)

tray.AddModule("I3LaputopFitter","CurvatureOnly")(
    ("SeedService","CurvSeed"),
    ("NSteps",1),                    # <--- tells it how many services to look for and perform
    ("Parametrization1","CurvParam"),  
    ("StoragePolicy","OnlyBestFit"),
    ("Minimizer","Minuit"),
    ("LogLikelihoodService","ToprecLike2"),     # the three likelihoods
    ("LDFFunctions",[""]),   # do NOT do the LDF (charge) likelihood
    ("CurvFunctions",["gaussparfree"]) # yes, do the CURVATURE likelihood
    )

#tray.AddModule("Dump","dump")()


tray.AddModule("I3Writer","EventWriter")(
    ("DropOrphanStreams", [icetray.I3Frame.DAQ]),
    ("Filename",I3_OUTFILE),
    )
 
## Output root file
root = I3ROOTTableService(ROOTFILE,"aTree")
tray.AddModule(I3TableWriter,'writer')(
    ("tableservice", root),
    ("keys",[ "Laputop", "LaputopParams",
              "CurvatureOnly", "CurvatureOnlyParams" ]),
    ("subeventstreams",["ice_top"])
    )


 
   
# Execute the Tray
# Just to make sure it's working!
tray.Execute(500)


   
