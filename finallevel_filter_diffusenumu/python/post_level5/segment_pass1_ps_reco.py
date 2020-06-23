#!/usr/bin/env python

### Infos about SplineMPE with Max settings see here:
### https://wiki.icecube.wisc.edu/index.php/Spline-reco

################## IMPORTS #############################################

from icecube import icetray
icetray.load("gulliver-modules", False)
del icetray

from icecube import icetray
icetray.load("spline-reco", False)
del icetray

from I3Tray import *
from icecube import icetray, dataclasses, photonics_service, gulliver, gulliver_modules, lilliput
from icecube import paraboloid
from icecube.icetray import I3Units
from icecube.photonics_service import I3PhotoSplineService
import string, os


@icetray.traysegment
def pass1_ps_reco_paraboloid(tray, name, 
                                PulsesName          = "",       
                                configuration       = "",            
                                TrackSeedList       = [],
                                EnergyEstimators    = [],
                                BareMuTimingSpline  = '/cvmfs/icecube.opensciencegrid.org/data/photon-tables/splines/InfBareMu_mie_prob_z20a10_V2.fits',
                                BareMuAmplitudeSpline = '/cvmfs/icecube.opensciencegrid.org/data/photon-tables/splines/InfBareMu_mie_abs_z20a10_V2.fits',
                                StochTimingSpline   = '/cvmfs/icecube.opensciencegrid.org/data/photon-tables/splines/InfHighEStoch_mie_prob_z20a10.fits',
                                StochAmplitudeSpline  = '/cvmfs/icecube.opensciencegrid.org/data/photon-tables/splines/InfHighEStoch_mie_abs_z20a10.fits',
                                If                  = lambda f: True ):
    

    # check if spline paths are ok
    if (not os.access(BareMuTimingSpline, os.R_OK)):
        icetray.logging.log_fatal("Cannot access BareMuTimingSpline path: "+BareMuTimingSpline)
    if (not os.access(BareMuAmplitudeSpline, os.R_OK)):
        icetray.logging.log_fatal("Cannot access BareMuAmplitudeSpline path: "+BareMuAmplitudeSpline)
    if configuration == "max":
        if (not os.access(StochTimingSpline, os.R_OK)):
            icetray.logging.log_fatal("Cannot access StochTimingSpline path: "+StochTimingSpline)
        if (not os.access(StochAmplitudeSpline, os.R_OK)):
            icetray.logging.log_fatal("Cannot access StochAmplitudeSpline path: "+StochAmplitudeSpline) 
     
    # configure the likelihood
    NoiseModel = "none"
    PreJitter = 4
    PostJitter = 0
    KSConfidenceLevel = 0
    EnergyDependentJitter = False
    EnergyDependentMPE = False
    
    # enable modifications depending on configuration 
    if not configuration == "default": 
        KSConfidenceLevel = 5 
        EnergyDependentMPE = True
    if configuration == "recommended" or configuration == "max":
        PreJitter = 2
        PostJitter = 2
        EnergyDependentJitter = True
    if configuration == "max":
        NoiseModel = "SRT"
    
    # python2 python3 compatible version of removing all punctuation
    def str_remove(instring, removestr):
        return ''.join(s for s in instring if s not in removestr)

    BareMuSplineName = "BareMuSplineJitter" + str(PreJitter) +\
            str_remove(BareMuAmplitudeSpline, string.punctuation) +\
            str_remove(BareMuTimingSpline, string.punctuation)
    StochSplineName = "StochMuSplineJitter" + str(PreJitter) +\
            str_remove(StochAmplitudeSpline, string.punctuation) +\
            str_remove(StochTimingSpline, string.punctuation)
    NoiseSplineName = "NoiseSpline" + \
            str_remove(BareMuAmplitudeSpline, string.punctuation)+\
            str_remove(BareMuTimingSpline, string.punctuation) 
 
    ####################### functions ##################################
    
    def cleanEmptySeries(frame, Pulses="SRTHVInIcePulses"):
        pulses = dataclasses.I3RecoPulseSeriesMap.from_frame(frame, Pulses)
        for omkey, ps in pulses:
            if len(ps) == 0:
                pulses.pop(omkey)
        del frame[Pulses]
        frame[Pulses] = pulses	    
    
    ####################### services ###################################
    
    # seed service without vertex time shift
    tray.AddService("I3BasicSeedServiceFactory", name+"SeedNoShift",    
              FirstGuesses =                 TrackSeedList,
              ChargeFraction =               0.9,                      # Default
              FixedEnergy =                  float("nan"),             # Default
              MaxMeanTimeResidual =          1000.0 * I3Units.ns,      # Default
              NChEnergyGuessPolynomial =     [],                       # Default
              SpeedPolice =                  True,                     # Default
              AddAlternatives =              "None",                   # Default
              OnlyAlternatives =             False                     # Default
              )

    # the minimizer
    tray.AddService("I3GulliverMinuitFactory", name+"Minuit",
              Algorithm =                    "SIMPLEX",                # Default
              FlatnessCheck =                True,                     # Default
              MaxIterations =                1000,                     # ! Only need 1000 iterations
              MinuitPrintLevel =             -2,                       # Default
              MinuitStrategy =               2,                        # Default
              Tolerance =                    0.01                      # ! Set tolerance to 0.01
              )

    # parameterization:
    tray.AddService("I3SimpleParametrizationFactory", name+"SimpleTrack",
              StepX =                        20 * I3Units.m,           # ! Set to 1/50 the size of the detector
              StepY =                        20 * I3Units.m,           # ! Set to 1/50 the size of the detector
              StepZ =                        20 * I3Units.m,           # ! Set to 1/50 the size of the detector
              StepZenith =                   0.1 * I3Units.radian,     # ! Set to 1/30 the size of the detector
              StepAzimuth =                  0.2 * I3Units.radian,     # ! Set to 1/30 the size of the detector
              StepLinE =                     0,                        # Default
              StepLogE =                     0,                        # Default
              StepT =                        0,                        # Default
              BoundsAzimuth =                [ 0, 0 ],                 # Default
              BoundsZenith =                 [ 0, 0 ],                 # Default
              BoundsT =                      [ 0, 0 ],                 # Default
              BoundsX =                      [ -2000 * I3Units.m,
                                                +2000 * I3Units.m ],    # ! Set bounds to twice the size of the detector
              BoundsY =                      [ -2000 * I3Units.m,
                                                +2000 * I3Units.m ],    # ! Set bounds to twice the size of the detector
              BoundsZ =                      [ -2000 * I3Units.m,
                                                +2000 * I3Units.m ],    # ! Set bounds to twice the size of the detector
              )

    ExistingServices = tray.context.keys()
    # bare muon spline
    if not BareMuSplineName in ExistingServices: 
        tray.context[BareMuSplineName] = I3PhotoSplineService(BareMuAmplitudeSpline,
            BareMuTimingSpline,
            PreJitter)

    if configuration == "max": 
        # stochasics spline
        if not StochSplineName in ExistingServices:
          tray.context[StochSplineName] = I3PhotoSplineService(StochAmplitudeSpline,
            StochTimingSpline,
            PreJitter)

        # noise spline  
        if not NoiseSplineName in ExistingServices:
          tray.context[NoiseSplineName] = I3PhotoSplineService(BareMuAmplitudeSpline,
            BareMuTimingSpline,
            1000)

    # add Likelihood service
    tray.AddService("I3SplineRecoLikelihoodFactory", name+"SplineMPEllh",
           PhotonicsService = BareMuSplineName,
           PhotonicsServiceStochastics = StochSplineName,
           PhotonicsServiceRandomNoise = NoiseSplineName,
           ModelStochastics = False,
           NoiseModel = NoiseModel,
           Pulses = PulsesName,
           E_Estimators=EnergyEstimators,
           Likelihood = "MPE",
           NoiseRate = 10*I3Units.hertz,
           PreJitter = 0,
           PostJitter = PostJitter,
           KSConfidenceLevel = KSConfidenceLevel,
           ChargeCalcStep = 0,
           CutMode = "late",
           EnergyDependentJitter = EnergyDependentJitter,
           EnergyDependentMPE = EnergyDependentMPE)

    # Seed service for Paraboloid
    tray.AddService("I3BasicSeedServiceFactory", name+"ParaboloidSeed",      
            InputReadout=PulsesName,
            FirstGuesses=[name+"MaxSettingsFit"],
            TimeShiftType="TNone",
            PositionShiftType="None")

    ####################################################################
    
    tray.AddModule(cleanEmptySeries, "cleaning2", Pulses=PulsesName)

    # here is the fitter module
    tray.AddModule("I3SimpleFitter", name+"MaxSettingsFit",    
              SeedService       = name+"SeedNoShift",
              Parametrization   = name+"SimpleTrack",
              LogLikelihood     = name+"SplineMPEllh",
              Minimizer         = name+"Minuit",
              NonStdName        = "Params",
              StoragePolicy     = "OnlyBestFit",
              OutputName        = name+"MaxSettingsFit",
              If                = If)    

    # here paraboloid fitter module
    tray.AddModule("I3ParaboloidFitter", name+"Paraboloid",
            SeedService                 = name+"ParaboloidSeed",
            LogLikelihood               = name+"SplineMPEllh",
            MaxMissingGridPoints        = 1,
            VertexStepSize              = 5.*I3Units.m,
            ZenithReach                 = 0.5*I3Units.deg,
            AzimuthReach                = 0.5*I3Units.deg,
            GridPointVertexCorrection   = name+"ParaboloidSeed",
            Minimizer                   = name+"Minuit",
            NumberOfSamplingPoints      = 8,
            NumberOfSteps               = 3,
            OutputName                  = name+"Paraboloid",
            If                          = If)
