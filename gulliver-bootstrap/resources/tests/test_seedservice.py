import unittest
import math
import numpy as np

from I3Tray import *
from icecube import icetray, dataio, dataclasses, lilliput, gulliver
from icecube import gulliver_bootstrap
from icecube.icetray.i3logging import *
import icecube.lilliput.segments

#############################
###There are two tests to perform. One in which the seed service is used to
###wrap another seed service and one in which this is the central seed
###service. Two testing modules are written below, one for each of these cases
#############################

# icetray.I3Logger.global_logger.set_level(icetray.I3LogLevel.LOG_DEBUG)

#############################
#Finding the location of test files and such
#############################

testpath = os.environ.get('I3_TESTDATA')
if (testpath is None) or (not os.path.exists(testpath)):
  print("$I3_TESTDATA does not point to a proper folder.")
  print("Please set up your environment correctly.")
  sys.exit(1)

gcd_file = os.path.join(testpath, 'GCD/GeoCalibDetectorStatus_IC86.55697_corrected_V2.i3.gz')
i3_file  = os.path.join(testpath, 'sim/Level3_nugen_numu_IC86.2012.011069.000000_20events.i3.bz2')

pulses   = "TWSRTHVInIcePulses"


#First we define a function that will be used to set up the basic tray structure
#Each test will call this function once and then we will add a testing module at
#the end to ensure valid functionality
def SetUpTray(tray, wrapped = ""):
  llhname  = lilliput.segments.add_pandel_likelihood_service(tray, pulses, "MPE")
  
  tray.AddService("I3GSLRandomServiceFactory", "I3RandomService")

  # configure bootstrapping (6 iterations here)
  tray.AddService("BootstrappingLikelihoodServiceFactory", "BSLikelihood",
          Pulses            = pulses,
          Bootstrapping     = gulliver_bootstrap.BootstrapOption.Poisson,
          Iterations        = 6,
          WrappedLikelihood = llhname,
          RandomService     = "I3RandomService"
          )

  # seed with previous MPE fit
  tray.AddService("I3BasicSeedServiceFactory", "TestMPESeed",
          FirstGuesses =                 [ "TestMPEFit" ],
          ChargeFraction =               0.9,                      # Default
          FixedEnergy =                  777,                      # Default
          MaxMeanTimeResidual =          1000.0*I3Units.ns,        # Default
          NChEnergyGuessPolynomial =     [],                       # Default
          SpeedPolice =                  True,                     # Default
          AddAlternatives =              "None",                   # Default
          OnlyAlternatives =             False                     # Default
          )

  # Set up seed service for the wrapped case
  tray.AddService("BootStrappingSeedServiceFactory", "TheBSSeed",
          WrappedSeed             = wrapped,
          BootstrappingLikelihood = "BSLikelihood"
          )

  # configure file reader
  tray.AddModule('I3Reader', 'reader', FilenameList=[gcd_file,i3_file])




#This is module to interact with the seed service and perform various tests
#This class is used to test cases when the BootstrappingSeedService is a
#wrapper for another seed service
class TestWrappedSeed(icetray.I3Module, unittest.TestCase):
    
  def __init__(self,ctx):
    icetray.I3Module.__init__(self,ctx)
    self.setting = 0
    self.AddParameter("Setting", "Switch for the testing module", self.setting)
    self.seedService = 0
        
  def Configure(self):
    self.setting = self.GetParameter("Setting")
    self.seedService = self.context["TheBSSeed"]
        
  def Physics(self,frame):

    if self.setting == 0:
      dummyEventHypothesis = self.seedService.GetDummy()
      self.assertTrue(dummyEventHypothesis)
      dummyParticle = dummyEventHypothesis.particle
      self.assertTrue(dummyParticle)
      self.assertTrue(math.isnan(dummyParticle.pos.x))
      self.assertTrue(math.isnan(dummyParticle.pos.y))
      self.assertTrue(math.isnan(dummyParticle.pos.z))
      self.assertTrue(not dummyEventHypothesis.nonstd)

    elif self.setting == 1:
      particle = dataclasses.I3Particle()
      particle.dir = dataclasses.I3Direction(0.5, 0.5)
      eventHypothesis = gulliver.I3EventHypothesis(particle)
      self.seedService.Tweak(eventHypothesis)
      #This should not actually do anythin unless you are wrapping
      #...nothing to test

    elif self.setting == 2:
      particle = dataclasses.I3Particle()
      particle.dir = dataclasses.I3Direction(0.5, 0.5)
      eventHypothesis = gulliver.I3EventHypothesis(particle)
      self.seedService.FillInTheBlanks(eventHypothesis)
      self.assertTrue(eventHypothesis.particle.energy == 777)
      self.assertTrue(eventHypothesis.particle.dir.zenith == 0.5)
      self.assertTrue(eventHypothesis.particle.dir.azimuth == 0.5)

    self.PushFrame(frame)

  def Finish(self):
    log_info("Finished the test module")


#This is module to interact with the seed service and perform various tests
#These tests will be for when BooststrappingSeedService is the main
#seed service in the frame
class TestSolo(icetray.I3Module, unittest.TestCase):
    
  def __init__(self,ctx):
    icetray.I3Module.__init__(self,ctx)
    self.setting = 0
    self.AddParameter("Setting", "Switch for the testing module", self.setting)
    self.likelihood = 0
    self.seedService = 0
 
  def Configure(self):
    self.setting = self.GetParameter("Setting")
    self.seedService = self.context["TheBSSeed"]
    self.likelihood = self.context["BSLikelihood"]

  def Geometry(self, frame):
    assert("I3Geometry" in frame)
    i3geo = frame["I3Geometry"]
    self.likelihood.SetGeometry(i3geo)
    self.PushFrame(frame)
        
  def Physics(self,frame):
    if self.setting == 0:
      dummyEventHypothesis = self.seedService.GetDummy()
      self.assertTrue(dummyEventHypothesis)
      dummyParticle = dummyEventHypothesis.particle
      self.assertTrue(dummyParticle)
      self.assertTrue(dummyParticle.pos == dataclasses.I3Position(0,0,0))
      self.assertTrue(dummyParticle.fit_status == dataclasses.I3Particle.FitStatus.OK)
      self.assertTrue(dummyParticle.dir.phi == 0)
      self.assertTrue(dummyParticle.dir.theta == 0)
      self.assertTrue(not dummyEventHypothesis.nonstd)

    if self.setting == 1:
      dummyEventHypothesis = self.seedService.GetDummy()
      dummyEventHypothesis.particle.pos == dataclasses.I3Position(1,2,3)
      newEventHypothesis = self.seedService.GetCopy(dummyEventHypothesis)
      self.assertTrue(dummyEventHypothesis.particle == newEventHypothesis.particle)

    if self.setting == 2:
      self.likelihood.SetEvent(frame)
      self.seedService.SetEvent(frame)
      eventHypothesis = self.seedService.GetSeed(0)
      particle = eventHypothesis.particle
      self.assertAlmostEqual(particle.dir.zenith, 0.245967, places=5)
      self.assertAlmostEqual(particle.dir.azimuth, 0.85141, places=5)
      self.assertAlmostEqual(particle.pos.x, -133.877, places=2)
      self.assertAlmostEqual(particle.pos.y,  461.72, places=2)
      self.assertAlmostEqual(particle.pos.z,  329.199, places=2)

    self.PushFrame(frame)

  def Finish(self):
    log_info("Finished the test module")




#This is module to interact with the seed service and perform various tests
#These tests will be for when BooststrappingSeedService is the main
#seed service in the frame
class TestFailures(icetray.I3Module, unittest.TestCase):
    
  def __init__(self,ctx):
    icetray.I3Module.__init__(self,ctx)
    self.setting = 0
    self.AddParameter("Setting", "Switch for the testing module", self.setting)
    self.likelihood = 0
    self.seedService = 0
 
  def Configure(self):
    self.setting = self.GetParameter("Setting")
    if "TheBSSeed" in self.context:
      self.seedService = self.context["TheBSSeed"]
    if "BSLikelihood" in self.context:
      self.likelihood = self.context["BSLikelihood"]

  def Geometry(self, frame):
    i3geo = frame["I3Geometry"]
    if self.likelihood:
      self.likelihood.SetGeometry(i3geo)
    self.PushFrame(frame)
        
  def Physics(self,frame):
    if self.setting == 0:
      frame.Delete("I3Geometry")
      self.seedService.SetEvent(frame)

    elif self.setting == 1:
      self.likelihood.SetEvent(frame)
      self.seedService.SetEvent(frame)
      eventHypothesis = self.seedService.GetSeed(1000000)

    self.PushFrame(frame)

  def Finish(self):
    log_info("Finished the test module")




class TestSeedService(unittest.TestCase):

  def test_get_dummy_wrapped(self):
    tray = I3Tray()
    SetUpTray(tray,"TestMPESeed")
    tray.AddModule(TestWrappedSeed, "TestDummyWrapped", Setting=0)
    tray.Execute(5)

  def test_tweak_wrapped(self):
    tray = I3Tray()
    SetUpTray(tray,"TestMPESeed")
    tray.AddModule(TestWrappedSeed, "TestTweakWrapped", Setting=1)
    tray.Execute(5)

  def test_fill_in_the_blanks_wrapped(self):
    tray = I3Tray()
    SetUpTray(tray,"TestMPESeed")
    tray.AddModule(TestWrappedSeed, "TestFillInTheBlanksWrapped", Setting=2)
    tray.Execute(5)

  def test_get_dummy_solo(self):
    tray = I3Tray()
    SetUpTray(tray)
    tray.AddModule(TestSolo, "TestDummySolo", Setting=0)
    tray.Execute(5)

  def test_get_copy_solo(self):
    tray = I3Tray()
    SetUpTray(tray)
    tray.AddModule(TestSolo, "TestGetCopySolo", Setting=1)
    tray.Execute(5)

  def test_get_seed_solo(self):
    tray = I3Tray()
    SetUpTray(tray)
    tray.AddModule(TestSolo, "TestGetSeedSolo", Setting=2)
    tray.Execute(5)

  ##################################################
  ##The following tests are meant to hit the edge cases\
  ##These should all fail
  ##################################################

  @unittest.expectedFailure
  def test_nonexistant_seedservice(self):
    tray = I3Tray()
    tray.AddService("BootStrappingSeedServiceFactory", "TheBSSeed",
            WrappedSeed             = "TestMPESeed",
            BootstrappingLikelihood = "BSLikelihood"
            )
    tray.AddModule(TestSolo, "TestGetSeedSolo", Setting=2)
    tray.Execute()

  @unittest.expectedFailure
  def test_nonexistant_likelihood(self):
    tray = I3Tray()
    tray.AddService("I3BasicSeedServiceFactory", "TestMPESeed")
    tray.AddService("BootStrappingSeedServiceFactory", "TheBSSeed",
            WrappedSeed             = "TestMPESeed",
            BootstrappingLikelihood = "BSLikelihood"
            )
    tray.AddModule(TestSolo, "TestGetSeedSolo", Setting=2)
    tray.Execute()

  @unittest.expectedFailure
  def test_nonexistant_geometry(self):
    tray = I3Tray()
    SetUpTray(tray)
    tray.AddModule(TestFailures, "TestFailure", Setting=0)
    tray.Execute()

  @unittest.expectedFailure
  def test_large_seed(self):
    tray = I3Tray()
    SetUpTray(tray)
    tray.AddModule(TestFailures, "TestFailure", Setting=1)
    tray.Execute()

unittest.main()