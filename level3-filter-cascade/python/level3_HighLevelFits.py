from icecube.icetray import I3Units, load, traysegment
from icecube import dataclasses
from I3Tray import *
load("libphys-services")            # xml, event counter
load("libstatic-twc")               # static twc
load("libSTTools")                  # srt
load("libDomTools")                 # rt
load("libgulliver")                 # Gulliver
load("libgulliver-modules")         # Gulliver Module
load("liblilliput")                 # 
load("libcscd-llh")
load("libphotonics-service")
load("libbayesian-priors")
########################################
@traysegment
def HighLevelFits(tray, name, pulses, InIceCscd = lambda frame: True):

    ## SPE 32 reconstruction
    ##############################################################################################
    # Services to do Gulliver reconstruction
    tray.AddService("I3SimpleParametrizationFactory","SimpleTrack",
                    BoundsAzimuth                              = [0.0, 0.0], # Default                          
                    BoundsLinE                                 = [0.0, 0.0], # Default                          
                    BoundsLinL                                 = [0.0, 0.0], # Default                          
                    BoundsLogE                                 = [0.0, 0.0], # Default                          
                    BoundsLogL                                 = [0.0, 0.0], # Default                          
                    BoundsT                                    = [0.0, 0.0], # Default                          
                    BoundsX    =      [-2000.0*I3Units.m,+2000.0*I3Units.m],  # ! Change 
                    BoundsY    =      [-2000.0*I3Units.m,+2000.0*I3Units.m],  # ! Change 
                    BoundsZ    =      [-2000.0*I3Units.m,+2000.0*I3Units.m],  # ! Change 
                    BoundsZenith                               = [0.0, 0.0], # Default                            
                    ParticleTrace                                   = False, # Default                            
                    RelativeBoundsAzimuth                      = [0.0, 0.0], # Default                  
                    RelativeBoundsLinE                         = [0.0, 0.0], # Default                  
                    RelativeBoundsLinL                         = [0.0, 0.0], # Default                  
                    RelativeBoundsLogE                         = [0.0, 0.0], # Default                  
                    RelativeBoundsLogL                         = [0.0, 0.0], # Default                  
                    RelativeBoundsT                            = [0.0, 0.0], # Default                  
                    RelativeBoundsX                            = [0.0, 0.0], # Default                  
                    RelativeBoundsY                            = [0.0, 0.0], # Default                  
                    RelativeBoundsZ                            = [0.0, 0.0], # Default                  
                    RelativeBoundsZenith                       = [0.0, 0.0], # Default                  
                    StepAzimuth                      = 0.2 * I3Units.radian,  # ! Change                 
                    StepLinE                                          = 0.0, # Default                                
                    StepLinL                                          = 0.0, # Default                                
                    StepLogE                                          = 0.0, # Default
                    StepLogL                                          = 0.0, # Default                                
                    StepT                                             = 0.0, # Default                                
                    StepX                                = 20.0 * I3Units.m,  # ! Change                             
                    StepY                                = 20.0 * I3Units.m,  # ! Change                              
                    StepZ                                = 20.0 * I3Units.m,  # ! Change                             
                    StepZenith                       = 0.1 * I3Units.radian,  # ! Change                        
                    VertexMode                                         = '', # Default                                       
                    )

    # Define the gulliver minimization sevice to use
    tray.AddService( "I3GulliverMinuitFactory", "Minuit",
                     Algorithm= "SIMPLEX",                                  # Default
                     Tolerance= 0.01,                                       # ! change to 0.01
                     MaxIterations= 10000,                                  # Default
                     MinuitPrintLevel= -2,                                  # Default
                     MinuitStrategy= 2,                                     # Default
                     FlatnessCheck= True,                                   # Default
                     )

    # Use convoluted pandel as the PDF for the likelihood
    tray.AddService("I3GulliverIPDFPandelFactory","Pandel",
                    InputReadout= pulses,                               # ! Name of pulses to use
                    Likelihood= "SPE1st",                                  # Default
                    PEProb= "GaussConvoluted",                                  # Default
                    IceModel= 2,                                           # Default
                    IceFile= "",                                           # Default
                    AbsorptionLength= 98.0 * I3Units.m,                    # Default
                    JitterTime= 15.0 * I3Units.ns,                         # Default
                    NoiseProbability= 1.0*I3Units.hertz * 10.0*I3Units.ns,  # ! Added a little noise term
                    )

    # linefit seed service
    tray.AddService( "I3BasicSeedServiceFactory", "SPE2Seed",
                     FirstGuesses= ["SPEFit2","LineFit"],                   # ! Use spe2 and LF (in case spe2 fails) as seeds
                     InputReadout= pulses,                                  # ! Use pulses for vertex correction
                     TimeShiftType= "TFirst",                               # ! Use TFirst for vertex correction
                     SpeedPolice= True,                                     # Default
                     MaxMeanTimeResidual= 1000.0 * I3Units.ns,              # Default
                     )

    # track fit
    tray.AddModule( "I3IterativeFitter", "CscdL3_SPEFit16",
                    RandomService= "SOBOL",                                # Default
                    NIterations= 16,                                       # ! Nunmber of iterations
                    SeedService= "SPE2Seed",                               # ! Name of seed service
                    Parametrization= "SimpleTrack",                        # ! Name of track parametrization service
                    LogLikelihood= "Pandel",                               # ! Name of likelihood service
                    CosZenithRange= [ -1, 1 ],                             # Default
                    Minimizer= "Minuit",                                   # ! Name of minimizer service
                    If = InIceCscd,
                    )

    ## Cascade LLH reconstruction
    ##############################################################################################
    # cscd 
    tray.AddModule( "I3CscdLlhModule", "CscdL3_CascadeLlhVertexFit",
                    InputType=                   "RecoPulse",              # ! Use reco pulses
                    RecoSeries=                  pulses,                    # ! Name of pulse series
                    FirstLE=                     True,                     # Default
                    SeedWithOrigin=              False,                    # Default
                    SeedKey=                     "CascadeLlhVertexFit_IC",    # ! Seed fit with L2 CascadeLLlh
                    MinHits=                     8,                        # ! Require 8 hits
                    AmpWeightPower=              0.0,                      # Default
                    ResultName=                  "CscdL3_CascadeLlhVertexFit",    # ! Name of fit result
                    Minimizer=                   "Powell",                 # ! Set the minimizer to use
                    PDF=                         "UPandel",                # ! Set the pdf to use
                    ParamT=                      "1.0, 0.0, 0.0, false",   # ! Setup parameters
                    ParamX=                      "1.0, 0.0, 0.0, false",   # ! Setup parameters
                    ParamY=                      "1.0, 0.0, 0.0, false",   # ! Setup parameters
                    ParamZ=                      "1.0, 0.0, 0.0, false",   # ! Setup parameters
                    If = InIceCscd,
                    )
    
    ## Bayesian Track LLH reconstruction
    ##############################################################################################
    tray.AddService( "I3BasicSeedServiceFactory", "SPEFit16Seed",
                     InputReadout=                                 pulses, # ! Use timewindow cleaned pulses
                     FirstGuesses=                  [ "CscdL3_SPEFit16" ],        # ! Use SPE single fit as seed
                     TimeShiftType=               "TFirst",                 # ! Use TFirst for vertex correction
                     ChargeFraction=              0.9,                      # Default
                     FixedEnergy=                 float( "nan" ),           # Default
                     MaxMeanTimeResidual=         1000.0 * I3Units.ns,      # Default
                     NChEnergyGuessPolynomial=    [ 0.9789139, 
                                                    1.173308,
                                                    0.3895591 ],            # ! Energy polynomial
                     SpeedPolice=                 True,                     # Default
                     AddAlternatives=             "None",                   # Default
                     AltTimeShiftType=            "TFirst",                 # Default
                     OnlyAlternatives=            False,                     # Default
                     )

    tray.AddService( "I3PowExpZenithWeightServiceFactory", "ZenithWeight",
                     Amplitude=                   2.49655e-07,              # Default
                     CosZenithRange=              [ -1, 1 ],                # Default
                     DefaultWeight=               1.383896526736738e-87,    # Default
                     ExponentFactor=              0.778393,                 # Default
                     FlipTrack=                   False,                    # Default
                     PenaltySlope=               -1000,                    # ! Add penalty for being in the wrong region
                     PenaltyValue=               -200,                     # Default
                     Power=                      1.67721,                   # Default
                     )

    tray.AddService( "I3EventLogLikelihoodCombinerFactory", "ZenithWeightPandel",
                     InputLogLikelihoods=         [ "Pandel","ZenithWeight" ],              # ! Names of Likelihood functions to combine
                     Multiplicity=                "Max",                    # Default
                     RelativeWeights=             [],                        # Default
                     )

    tray.AddModule( "I3IterativeFitter", "CscdL3_Bayesian16",
                    RandomService=               "SOBOL",                  # ! Name of randomizer service
                    NIterations=                 16,                       # ! Nunmber of iterations
                    SeedService=                 "SPEFit16Seed",           # ! Name of seed service
                    Parametrization=             "SimpleTrack",            # ! Name of track parametrization service
                    LogLikelihood=               "ZenithWeightPandel",     # ! Name of likelihood service
                    CosZenithRange=              [ 0, 1 ],                 # ! This is a downgoing hypothesis
                    Minimizer=                   "Minuit",                 # ! Name of minimizer service
                    If = InIceCscd,
                    )



