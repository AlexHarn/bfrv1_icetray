#!/usr/bin/python

## With small showers, one cannot use timing/curvature as part of the likelihood.  
## If ONLY performing a charge LDF reconstruction, then only ONE step of Laputop is necessary.
## Note: the "Trigger" parameter must be set to 3 (since its default is 5).

## The parameters are borrowed from 1st step of the parameters in "laputop_standard_traysegment.py"

from I3Tray import *

from icecube import icetray, gulliver, gulliver_modules, lilliput

@icetray.traysegment
def LaputopSmallShower(tray, name, 
                       pulses='CleanedHLCTankPulses',
                       excluded='ClusterCleaningExcludedStations',
                       snowfactor=2.1,
                       ShowerCOGSeed='ShowerCOG',
                       ShowerPlaneSeed='ShowerPlane',
                       If = lambda f: True,
                       ):

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
        ("InCore", ShowerCOGSeed),
        ("InPlane", ShowerPlaneSeed),
        ("Beta",2.6),                    # first guess for Beta
        ("InputPulses",pulses)  # this'll let it first-guess at S125 automatically
        )
    
    ## Step 1:
    tray.AddService("I3LaputopParametrizationServiceFactory",name+"ToprecParam2")(
        ("FixCore", fixcore),    
        ("FixTrackDir", True),   # Yes, fix the direction!
        ("IsBeta", True),
        ("MinBeta", 1.5),   ## From toprec... 2nd iteration (DLP, using beta)
        ("MaxBeta", 5.0),
        ("LimitCoreBoxSize", 200.0) 
    )

    tray.AddService("I3LaputopLikelihoodServiceFactory",name+"ToprecLike2")(
        ("DataReadout", pulses),
        ("BadTanks", excluded),
        ("DynamicCoreTreatment", 5.0),     # do the 5-meter core cut
        ("Trigger", 3),            ## Reduce min number of stations (the default is 5)
        ("Curvature",""),      # NO timing likelihood (at first; this will be overridden)
        ("SnowServiceName",name+"SimpleSnow"),
        ("OldXYZ", True)  # For backward-compatibility for L3: DOM coordinates
        )
    
    ################# GULLIVERIZED FITTER MODULE #######################
    
    ## This module performs the three steps
    tray.AddModule("I3LaputopFitter",name)(
        ("SeedService",name+"ToprecSeed"),
        ("NSteps",1),            # <--- tells it how many services to look for and perform
        ("Parametrization1",name+"ToprecParam2"),   # the one parametrization
        ("StoragePolicy","OnlyBestFit"),
        ("Minimizer",name+"Minuit"),
        ("LogLikelihoodService",name+"ToprecLike2"),     # the one likelihood
        ("LDFFunctions",["dlp"]),
        ("CurvFunctions",[""]), # NO curvature or timing likelihood
        ("If",If)
        )
    

