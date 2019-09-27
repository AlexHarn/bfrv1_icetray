from icecube import icetray
import string

icetray.load("spline-reco", False)


# clean up the local dictionary
del icetray

from icecube import icetray, dataclasses, dataio, phys_services
from icecube import gulliver, gulliver_modules, lilliput
from icecube.icetray import I3Units

from icecube.load_pybindings import load_pybindings
load_pybindings(__name__,__path__)

def str_remove(instring, removestr):
    return ''.join(s for s in instring if s not in removestr)

@icetray.traysegment
def SplineMPE(tray, name,
              fitname = "",
              configuration="default",
              PulsesName = "",
              TrackSeedList = [],
              EnergyEstimators = [],
              BareMuTimingSpline = '/cvmfs/icecube.opensciencegrid.org/data/photon-tables/splines/InfBareMu_mie_prob_z20a10_V2.fits',
              BareMuAmplitudeSpline = '/cvmfs/icecube.opensciencegrid.org/data/photon-tables/splines/InfBareMu_mie_abs_z20a10_V2.fits',
              StochTimingSpline = '/cvmfs/icecube.opensciencegrid.org/data/photon-tables/splines/InfHighEStoch_mie_prob_z20a10.fits',
              StochAmplitudeSpline = '/cvmfs/icecube.opensciencegrid.org/data/photon-tables/splines/InfHighEStoch_mie_abs_z20a10.fits',
              If = lambda f: True):

  """
  Perform a SplineMPE-reconstruction 
  Configuration=='default' will run a default MPE reco giving the worst resolution.
  The options 'fast', 'recommended' and 'max' will activate modifications
  with rising accuracy and decreasing execution speed, where "fast" is faster than "default" because of quicker convergence.
  See https://wiki.icecube.wisc.edu/index.php/Spline-reco for documentation as well as speed and resolution comparisons. 
  """
  
  # check input args
  if (PulsesName == ""):
    icetray.logging.log_fatal("Specify PulsesName")
  if (len(TrackSeedList)==0):
    icetray.logging.log_fatal("Specify Tracks used as a seed")
  if not (configuration == "default" or configuration == "fast" or configuration == "recommended" or configuration == "max"):
    icetray.logging.log_fatal("Configuration must be one of 'default', 'fast', 'recommended' or 'max'")  
  if len(EnergyEstimators)==0 and not configuration == "default":
    icetray.logging.log_fatal("If configuration is not 'default' at least one EnergyEstimator has to be set.")

  # check if spline paths are ok
  import os
  if (not os.access(BareMuTimingSpline, os.R_OK)):
    icetray.logging.log_fatal("Cannot access BareMuTimingSpline path: "+BareMuTimingSpline)
  if (not os.access(BareMuAmplitudeSpline, os.R_OK)):
    icetray.logging.log_fatal("Cannot access BareMuAmplitudeSpline path: "+BareMuAmplitudeSpline)
  if configuration == "max":
    if (not os.access(StochTimingSpline, os.R_OK)):
      icetray.logging.log_fatal("Cannot access StochTimingSpline path: "+StochTimingSpline)
    if (not os.access(StochAmplitudeSpline, os.R_OK)):
      icetray.logging.log_fatal("Cannot access StochAmplitudeSpline path: "+StochAmplitudeSpline) 
 

####### Add Services ##########

  # seed service without vertex time shift
  tray.AddService("I3BasicSeedServiceFactory", name+"SeedNoShift",
          FirstGuesses =                 TrackSeedList,
          ChargeFraction =               0.9,                      # Default
          FixedEnergy =                  float("nan"),           # Default
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
   
  ######### Configure the likelihood ###########
  
  # disable all modifications
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


  ####### Load Splines ##############

  # If we want to use multiple SplineMPE segments with varying configuration,
  # each would load it's own spline services. This would be a huge waste of memory.
  # Therefore this creates a SplineService name from the spline paths
  # and the jitter value and checks if this SplineService already exists.
  # If not the SplineService will be created

  #SplineService names
  BareMuSplineName = "BareMuSplineJitter" + str(PreJitter) +\
          str_remove(BareMuAmplitudeSpline, string.punctuation) +\
          str_remove(BareMuTimingSpline, string.punctuation)
  StochSplineName = "StochMuSplineJitter" + str(PreJitter) +\
          str_remove(StochAmplitudeSpline, string.punctuation) +\
          str_remove(StochTimingSpline, string.punctuation)
  NoiseSplineName = "NoiseSpline" + \
          str_remove(BareMuAmplitudeSpline, string.punctuation)+\
          str_remove(BareMuTimingSpline, string.punctuation) 
    
  #the old p2 version:
  #BareMuSplineName = "BareMuSplineJitter" + str(PreJitter) + BareMuAmplitudeSpline.translate(str.maketrans("","", string.punctuation)) + BareMuTimingSpline.translate(str.maketrans("","", string.punctuation))
  #StochSplineName = "StochMuSplineJitter" + str(PreJitter) + StochAmplitudeSpline.translate(str.maketrans("","", string.punctuation)) + StochTimingSpline.translate(str.maketrans("","", string.punctuation))
  #NoiseSplineName = "NoiseSpline" + BareMuAmplitudeSpline.translate(str.maketrans("","", string.punctuation)) + BareMuTimingSpline.translate(str.maketrans("","", string.punctuation))

  ExistingServices = tray.context.keys()

  from icecube.photonics_service import I3PhotoSplineService
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

  ############ Add Likelihood service ################

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

  #here is the fitter module
  tray.AddModule("I3SimpleFitter", name,
          SeedService = name+"SeedNoShift",
          Parametrization = name+"SimpleTrack",
          LogLikelihood = name+"SplineMPEllh",
          Minimizer = name+"Minuit",
          NonStdName = "Params",
          StoragePolicy = "OnlyBestFit",
          OutputName = fitname,
          If = If)
