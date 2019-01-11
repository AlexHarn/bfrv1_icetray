#!/usr/bin/python

## Want to compare the old code (I3TopLateralFit) to the new code (Laputop)?  There are sufficient
## differences between the two that they will never match exactly, but the purpose of this traysegment
## is to run Laputop in a way that mimics what I3TopLateralFit did as closely as possible.
## It runs four steps, performs a "static" 11-meter core cut between steps 1 and 2, etc.

from I3Tray import *


load("libgulliver")
load("liblilliput")
load("libtoprec")

@icetray.traysegment
def LaputopMimicShowerCombined(tray, name, 
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
        ("MaxIterations",1000),
        ("MinuitStrategy",2),
        ("Tolerance",0.01),    
        )
    
    ## The Seed service
    tray.AddService("I3LaputopSeedServiceFactory",name+"ToprecSeed")(
        ("InCore", "ShowerCOG"),
        ("InPlane", "ShowerPlane"),
#        ("SnowCorrectionFactor", snowfactor),   # <--- Now obsolete
        ("Beta",2.6),                    # first guess for Beta
        ("InputPulses",pulses)  # this'll let it first-guess at S125 automatically
        )
    
    ## Step 1 AND 2 (they are the same):
    tray.AddService("I3LaputopParametrizationServiceFactory",name+"ToprecParam1")(
        ("FixCore", fixcore),        
        ("FixTrackDir", True),
        ("IsBeta", True),
        ("MinBeta", 2.9),   ## From toprec... 2nd iteration (DLP, using beta)
        ("MaxBeta", 3.1),
        ("LimitCoreBoxSize", -1) 
    )
    tray.AddService("I3LaputopParametrizationServiceFactory",name+"ToprecParam2")(
        ("FixCore", fixcore),        
        ("FixTrackDir", True),
        ("IsBeta", True),
        ("MinBeta", 2.9),   ## From toprec... 2nd iteration (DLP, using beta)
        ("MaxBeta", 3.1),
        ("LimitCoreBoxSize", -1) 
    )

    ## Step 3:
    tray.AddService("I3LaputopParametrizationServiceFactory",name+"ToprecParam3")(
        ("FixCore", fixcore),        
        ("FixTrackDir", False),      # FREE THE DIRECTION!
        ("IsBeta", True),
        ("MinBeta", 2.0),   ## From toprec... 3rd iteration (DLP, using beta)
        ("MaxBeta", 4.0),
        ("LimitCoreBoxSize", 15.0),  # these two together mimic the "+/- 3 sigma thing
        ## Use these smaller stepsizes instead of the defaults:
        ("VertexStepsize",5.0),      # default is 20
        ("SStepsize", 0.045),        # default is 1
        ("BetaStepsize",0.15)        # default is 0.6    
        )
        
    ## Step 4:
    tray.AddService("I3LaputopParametrizationServiceFactory",name+"ToprecParam4")(
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
    
    tray.AddService("I3LaputopLikelihoodServiceFactory",name+"ToprecLike2")(
        ("DataReadout", pulses),
        ("BadStations", excluded),
        ("DynamicCoreTreatment", 0.0),     # don't do dynamic core things
        ("SaturationLikelihood", False),   # don't do saturation likelihood
        ("Curvature",""),      # NO timing likelihood (at first; this will be overridden)
        ("SnowServiceName",name+"SimpleSnow")
       )
    
    
    ################# GULLIVERIZED FITTER MODULE #######################
    
    ## This module performs the three steps
    tray.AddModule("I3LaputopFitter",name)(
        ("SeedService",name+"ToprecSeed"),
        ("NSteps",4),            # <--- tells it how many services to look for and perform
        ("Parametrization1",name+"ToprecParam1"),   # the four parametrizations
        ("Parametrization2",name+"ToprecParam2"),
        ("Parametrization3",name+"ToprecParam3"),
        ("Parametrization4",name+"ToprecParam4"),
        ("StoragePolicy","OnlyBestFit"),
        ("Minimizer",name+"Minuit"),
        ("LogLikelihoodService",name+"ToprecLike2"),     # the four likelihoods
        ("LDFFunctions",["dlp","dlp","dlp","dlp"]),
        ("CurvFunctions",["","","gausspar","gausspar"]),   # VERY IMPORTANT : use time Llh for step 3, but fix direction!
        ("StaticCoreCut",[0, 11.0, 0, 0])    ## perform the static 11-meter core cut between steps 1 and 2
        )
    

