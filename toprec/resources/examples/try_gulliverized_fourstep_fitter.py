#!/usr/bin/python

import os
import sys,getopt
#import logging
from os.path import expandvars

from I3Tray import *


load("libicetray")
load("libdataclasses")
load("libphys-services")
load("libdataio")
load("libicepick")
load("libgulliver")
load("libgulliver-modules")
load("liblilliput")
load("libtoprec")

workspace = expandvars("$I3_BUILD")
tools = expandvars("$I3_PORTS")
HOME = expandvars("$HOME")

#### PUT YOUR FAVORITE GCD AND INPUT FILE HERE
GCDfile = "/data/user/kath/gcd/Level2_IC79_data_Run00116097_monthlysnow_GCD.i3.gz"
infile = "/data/user/kath/testdata/Level2a_IC79_data_Run00116081_Part00000000_IT.i3"

#### NAME YOUR OUTPUT FILES HERE
I3_OUTFILE = HOME+"/testout/toprec_testme.i3"
ROOTFILE = HOME+"/testout/toprec_testme.root"


tray = I3Tray()

########## SERVICES FOR GULLIVER ##########

datareadoutName="IceTopVEMPulses_0"

## The "simple lambda" snowservice
tray.AddService("I3SimpleSnowCorrectionServiceFactory","SimpleSnow21")(
    ("Lambda", 2.1)
    )

## This one is the standard one.
tray.AddService("I3GulliverMinuitFactory","Minuit")(
    ("MinuitPrintLevel",-2),  
    ("FlatnessCheck",True),  
    ("Algorithm","SIMPLEX"),  
    ("MaxIterations",1000),
    ("MinuitStrategy",2),
    ("Tolerance",0.01),    
    )

tray.AddService("I3LaputopSeedServiceFactory","ToprecSeed")(
    ("InCore", "ShowerCOG"),
    ("InPlane", "ShowerPlane"),
    #("SnowCorrectionFactor", 1.5),   # Now obsolete
    ("Beta",2.6),                    # first guess for Beta
    ("InputPulses",datareadoutName)  # this'll let it first-guess at S125 automatically
)

# Reminder... here are TopLateralFit's four steps:
# * 1) core only
# * 2) core + snow, with pulses < 11 meters removed
# * 3) core (+/- 3 sigma) + direction, but no snow
# * 4) core again (unbounded) + snow, but no direction + curv ON

fixcore = False

## Do not use... we'll combine steps 1 and 2
#tray.AddService("I3LaputopParametrizationServiceFactory","ToprecParam1")(
#    ("FixCore", fixcore),        
#    ("FixTrackDir", True),
#    ("IsBeta", True),
#    ("MinBeta", 2.9),   ## From toprec... 1st iteration (DLP, using beta)
#    ("MaxBeta", 3.1),
#    ("LimitCoreSigma", False) 
#    )

tray.AddService("I3LaputopParametrizationServiceFactory","ToprecParam2")(
    ("FixCore", fixcore),        
    ("FixTrackDir", True),
    ("IsBeta", True),
    ("MinBeta", 2.9),   ## From toprec... 2nd iteration (DLP, using beta)
    ("MaxBeta", 3.1),
    ("LimitCoreBoxSize", -1) 
    )

tray.AddService("I3LaputopParametrizationServiceFactory","ToprecParam3")(
    ("FixCore", fixcore),        
    ("FixTrackDir", False),      # FREE THE DIRECTION!
    ("IsBeta", True),
    ("MinBeta", 2.0),   ## From toprec... 3rd iteration (DLP, using beta)
    ("MaxBeta", 4.0),
    ("LimitCoreBoxSize", 15.0),
    ## Use these smaller stepsizes instead of the defaults:
    ("VertexStepsize",5.0),      # default is 20
    ("SStepsize", 0.045),        # default is 1
    ("BetaStepsize",0.15)        # default is 0.6    
    )

tray.AddService("I3LaputopParametrizationServiceFactory","ToprecParam4")(
    ("FixCore", fixcore),        
    ("FixTrackDir", True),
    ("IsBeta", True),
    ("MinBeta", 1.5),   ## From toprec... 4th iteration (DLP, using beta)
    ("MaxBeta", 5.0),
    ("LimitCoreBoxSize", -1),
    ## Use these smaller stepsizes instead of the defaults:
    ("VertexStepsize", 4.0),     # default is 20
    ("SStepsize", 0.045),        # default is 1
    ("BetaStepsize",0.15)        # default is 0.6 
    )

tray.AddService("I3LaputopLikelihoodServiceFactory","ToprecLike2")(
    ("datareadout", datareadoutName),
    #("badstations", "IceTopExcludedStations"),   # this option was replaced with BadTanks option
    ("dynamiccoretreatment",11.0),     # do the 11-meter core cut
    ("curvature",""),      # NO timing likelihood
    ("SnowServiceName","SimpleSnow21")
    )


#------------------- LET'S RUN SOME MODULES!  ------------------

#**************************************************
#                 Reader and whatnot
#**************************************************
tray.AddModule("I3Reader","reader")(
    ("FileNameList", [GCDfile, infile])
    )


#**************************************************
#                     icepick
#**************************************************
## Let's look at big events only: 20 pulses minimum 
## (These will survive step 3 in gulliver)
## Feel free to remove this if you like.
#
#tray.AddModule("I3IcePickModule<I3NHitChannelFilter>", "nch")(
#    ("MinThreshold", 20),
#    ("ResponseKey", datareadoutName),
#    ("DiscardEvents", True),
#    ("DecisionName","Nch10")
#    )


################# GULLIVERIZED FITTER MODULE #######################

tray.AddModule("I3LaputopFitter","Gulliverized")(
    ("SeedService","ToprecSeed"),
    ("NSteps",3),                    # <--- tells it how many services to look for and perform
    ("Parametrization1","ToprecParam2"),   # the three parametrizations
    ("Parametrization2","ToprecParam3"),
    ("Parametrization3","ToprecParam4"),
    ("StoragePolicy","OnlyBestFit"),
    ("Minimizer","Minuit"),
    ("LogLikelihoodService","ToprecLike2"),     # the three likelihoods
    ("LDFFunctions",["dlp","dlp","dlp"]),
    ("CurvFunctions",["","gausspar","gausspar"])   # VERY IMPORTANT : use time Llh for step 3, but fix direction!
    )

#tray.AddModule("Dump","dump")()


tray.AddModule("I3Writer","EventWriter")(
    ("DropOrphanStreams", [icetray.I3Frame.DAQ]),
    ("Filename",I3_OUTFILE),
    )
 

   
# Execute the Tray
# a few events
tray.Execute(100)


   
