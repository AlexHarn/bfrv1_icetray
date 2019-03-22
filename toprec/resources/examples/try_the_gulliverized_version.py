#!/usr/bin/python

#---------------------------------------------------------
#  This script dates back to early development, and early tests of the Laputop
#  Services (the SeedService, ParametrizationService, and LikelihoodService)
#  This test used "SimpleFitter" to see whether these services were doing
#  what they were supposed to... it performs only one minimization step.
#  
#  I'll try to keep this in a runnable state, but... for analysis,
#  you should probably be using "LaputopFitter" instead, which can 
#  perform up to four steps.  
#---------------------------------------------------------

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
load("libflat-ntuple")


workspace = expandvars("$I3_BUILD")

#### Put your favorite input file here:
#### (Easiest if it's already a level2 file with COG and Planefit and whatnot...)
HOME = expandvars("$HOME")
GCDfile = "/basil/exp_it73/gcd/Level2_IC79_data_Run00116097_monthlysnow_GCD.i3.gz"
#infile = "/basil/exp_it73/Level2_IC79_data_Run00116097_Part00000004_IT.i3.gz"
infile = "/basil/exp_it73/snow15reco/snowcorr_116097_monthlysnow.i3.gz"
I3_OUTFILE = HOME+"/testout/gulliverized_toprec_2nd.i3"
ROOTFILE = HOME+"/testout/gulliverized_toprec_2nd.root"

tray = I3Tray()


#**************************************************
#             SERVICES FOR GULLIVER 
#**************************************************
datareadoutName="IceTopVEMPulses_0"

## The "simple lambda" snowservice
tray.AddService("I3SimpleSnowCorrectionServiceFactory","SimpleSnow21")(
    ("Lambda", 2.1)
    )

## This one is a standard one.  It lives in the lilliput project.
tray.AddService("I3GulliverMinuitFactory","Minuit")(
    ("MinuitPrintLevel",-2),  
    ("FlatnessCheck",True),  
    ("Algorithm","SIMPLEX"),  
    ("MaxIterations",1000),
    ("MinuitStrategy",2),
    ("Tolerance",0.01),    
    )

## This one initializes a seed track, and also seeds for the 
## three "extras" (S125, beta, and snow)
tray.AddService("I3LaputopSeedServiceFactory","ToprecSeed")(
    ("InCore", "ShowerCOG"),
    ("InPlane", "ShowerPlane"),
    #("SnowCorrectionFactor", 1.5),  # <-- Now obsolete
    ("Beta",2.6),                  # seed beta
    ("InputPulses",datareadoutName)   # this'll let it guess at seed S125 for you
)

## Free parameters and bounds: here, we'll mimic the 1st of 
## the lateralfit's four minimizations.
tray.AddService("I3LaputopParametrizationServiceFactory","ToprecParam1")(
    ("FixCore", True),        ## Float the core position
    ("FixSize", True),        ## Float the core position
    ("FixTrackDir", False),     ## ... but not the track direction
    ("IsBeta", True),    ## Not used for now... (for NKG, it's "age")
    ("MinBeta", 2.9),   ## Toprec's initial bounds on beta for the 1st iteration
    ("MaxBeta", 3.1),   ##   which are quite restrictive
    ("LimitCoreBoxSize", -1)  
    )

## The real "meat": the likelihood function!
## For now, only DLP is implemented.
tray.AddService("I3LaputopLikelihoodServiceFactory","ToprecLike")(
    ("DataReadout", datareadoutName),
    ("BadStations", "IceTopExcludedStations"),
    ("DynamicCoreTreatment",11.0),            # Let's try this, to emulate step2 (combined with step1)
    ("Curvature", "gausspar"),   # For now, turn curvature off
#    ("CutBad", 0),               # doesn't do anything yet
#    ("CutBadThreshold", 3),      # doesn't do anything yet
    ("LDF", "dlp"),          # only one available for now
    ("SoftwareThreshold", -1),   # default
    ("Trigger", 5),               # should become obsolete in future, as gulliver decides
                                   # for itself whether a fit is possible
    ("SnowServiceName","SimpleSnow21")
    )

#**************************************************
#                 Reader & Muxer 
#**************************************************
tray.AddModule("I3Reader","reader")(
    ("FileNameList", [GCDfile, infile])
    )


##### Add this module if you need to convert your input file
##### (say, a level2 file with no Q-frames) into an environment
##### that DOES contain Q-frames (such as icerec-trunk)
tray.AddModule("QConverter","qconvert")(  
    )



#**************************************************
#     first guess reconstruction - plane & cog
#        let's assume these are already done.
#**************************************************
#
#tray.AddModule("I3TopRecoCore", "core")(
#    ("datareadout", datareadoutName),
#    )
#
#tray.AddModule("I3TopRecoPlane","plane")(
#    ("datareadout", datareadoutName),
#    )



#**************************************************
#                     lateralfit
#   the regular kind, for comparison
#   remember, this module performs FOUR minimizations
#   (Unless you hack into it for testing purposes, like
#    I have!)
#   
#   Our gulliverized version is only going to mimic the
#   first one.
#**************************************************

#tray.AddModule("I3TopLateralFit", "latfit")(
#    ("datareadout", datareadoutName),
#    ("badstations", "IceTopExcludedStations"),
#    ("OutShower", "ShowerCombined2")
#    )



#**************************************************
#           GULLIVERIZED FITTER MODULE 
#**************************************************

## A simple one that does just ONE iteration.
## This lives in gulliver-modules.
tray.AddModule("I3SimpleFitter","GulliverizedToprec")(
    ("SeedService","ToprecSeed"),
    ("Parametrization","ToprecParam1"),
    ("StoragePolicy","OnlyBestFit"),    # default; irrelevant since we have only 1 seed
    ("Minimizer","Minuit"),
    ("LogLikelihood","ToprecLike"),
    )

tray.AddModule("Dump","dump")()


tray.AddModule("I3Writer","EventWriter")(
    ("DropOrphanStreams", [icetray.I3Frame.DAQ]),
    ("Filename",I3_OUTFILE),
    )
 
## Flat-ntuple it if you feel like it.
## 
## Be aware: flat-ntuple cannot properly find the fitparameters
## yet.  It will find Gulliver's internal FitParams (like rlogl, etc.)
## But it cannot find the "extras" (S125, Beta, etc.) unless 
## you hack into flat-ntuple too.  Fixing this is in progress.
# 
#tray.AddModule("I3FlatNtupleModule","flatntuple")(
#    ("FavoriteFit","GulliverizedToprec"),   
#    ("FavoritePulses",datareadoutName),
#    ("TreeName","aTree"),
#    ("OutFile",ROOTFILE),
##    ("MCWeightsName","CorsikaWeightMap"),
#    ("BookOMs",False),
#    ("EventHeader","I3EventHeader"),
#    ("BookTracks",True),
##    ("MCTruthName","I3MCTree"),
#    ("BookMuons",False),
#    ("BookNDirect",False),
#    ("BookSkyCoords",False),
#    )
 

 
   
# Execute the Tray
# one event
tray.Execute(20)
#tray.Execute(100)


   
