#!/usr/bin/python

## Have you ALREADY performed a fit using Laputop (or some other fitter), and would just like to make
## one small change to the procedure and run it again?  
## (A classic case of this is: trying a different snow attenuation length.)

## If so, then there's no need to run all three steps from the beginning... just run the "last" step.
## In this traysegment, give it the name of the last fit you ran (the "seed"), the name of the
## pulses, and the name of the NEW snow attenuation length, which will override whatever the attenuation
## length was from the seed fit.

## The parameters are borrowed from the March 2012 parameters in "laputop_standard_traysegment.py".

from I3Tray import *


load("libgulliver")
load("liblilliput")
load("libtoprec")

@icetray.traysegment
def LaputopLastStepSnowOnly(tray, name, seed,
                             pulses='CleanedHLCTankPulses',
                             excluded='ClusterCleaningExcludedStations',
                             snowfactor=2.1):

    ## Some more defaults
    fixcore = False     # do NOT keep core fixed
    

    ########## SERVICES FOR GULLIVER ##########

    ## The "simple lambda" snowservice
    tray.AddService("I3SimpleSnowCorrectionServiceFactory",name+"SimpleSnow")(
        ("Lambda", snowfactor)
        )

    ## This one is the standard one.
    tray.AddService("I3GulliverMinuitFactory",name+"Minuit")(
        ("MinuitPrintLevel",-2),  
        ("FlatnessCheck",True),  
        ("Algorithm","SIMPLEX"),  
        ("MaxIterations",2500),
        ("MinuitStrategy",2),
        ("Tolerance",0.01),    
        )
    
    ## The Seed service
    tray.AddService("I3LaputopSeedServiceFactory",name+"ToprecSeed")(
        ("InCore", seed),
        ("InPlane", seed),
        ("InParams", seed+"Params")
#        ("SnowCorrectionFactor", snowfactor)   # <--- Now obsolete   
                            # All other seed stuff (like S125) will come from the Params.
        )
    
    ## Step 3 repeated:
    tray.AddService("I3LaputopParametrizationServiceFactory",name+"ToprecParam4")(
        ("FixCore", fixcore),        
        ("FixTrackDir", True),
        ("IsBeta", True),
        ("MinBeta", 0.0),   ## From toprec... 4th iteration (DLP, using beta)
        ("MaxBeta", 10.0),
        ("LimitCoreBoxSize", 45.0),
        ("maxLogS125",8.0),        # Default is 6., be a bit safer, although should never happen to be this large
        ## Use these smaller stepsizes instead of the defaults:
        ("VertexStepsize", 4.0),     # default is 20
        ("SStepsize", 0.045),        # default is 1
        ("BetaStepsize",0.15)        # default is 0.6 
        )
    
    tray.AddService("I3LaputopLikelihoodServiceFactory",name+"ToprecLike2")(
        ("DataReadout", pulses),
        ("BadStations", excluded),
        ("DynamicCoreTreatment", 11.0),     # do the 11-meter core cut
        ("SaturationLikelihood", True),
        ("MaxIntraStationTimeDiff",80.0),    # Don't use time fluctuating tanks for timing fits, could really mess up the hard work
        ("Curvature","gausspar"),    
        ("SnowServiceName",name+"SimpleSnow")
        )
    
    
    ################# GULLIVERIZED FITTER MODULE #######################
    
    ## This module performs the three steps
    tray.AddModule("I3LaputopFitter",name)(
        ("SeedService",name+"ToprecSeed"),
        ("NSteps",1),            # <--- tells it how many services to look for and perform
        ("Parametrization1",name+"ToprecParam4"), 
        ("StoragePolicy","OnlyBestFit"),
        ("Minimizer",name+"Minuit"),
        ("LogLikelihoodService",name+"ToprecLike2"), 
        ("LDFFunctions",["dlp"]),
        ("CurvFunctions",["gausspar"])
        )
    

