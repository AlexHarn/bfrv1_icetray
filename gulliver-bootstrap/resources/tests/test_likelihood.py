import unittest

from I3Tray import *
from icecube import icetray, dataio, dataclasses, lilliput, gulliver
from icecube import gulliver_bootstrap
from icecube.icetray.i3logging import *
import icecube.lilliput.segments


#This is the overall testing module which can directly call specific functions
#from the services. Different tests are performed using the "setting" argument
class TestModule(icetray.I3Module, unittest.TestCase):
    
  def __init__(self,ctx):
    icetray.I3Module.__init__(self,ctx)
    self.setting = 0
    self.AddParameter("Setting", "Switch for the testing module", self.setting)
    self.likelihood = 0

  def Configure(self):
    self.setting = self.GetParameter("Setting")
    if "BSLikelihood" in self.context:
      self.likelihood = self.context["BSLikelihood"]
        
  def Physics(self,frame):

    if self.setting == 0:
      particle = dataclasses.I3Particle()
      eventHypothesis = gulliver.I3EventHypothesis(particle)
      print(self.likelihood.GetName())
      self.assertTrue(self.likelihood.GetName() == "BSLikelihood")

    if self.setting == 1:
      particle = dataclasses.I3Particle()
      eventHypothesis = gulliver.I3EventHypothesis(particle)
      self.likelihood.GetLogLikelihoodWithGradient(eventHypothesis, eventHypothesis, 1)

    if self.setting == 2:
      self.assertFalse(self.likelihood.HasGradient())

    if self.setting == 3:
      self.likelihood.SetEvent(frame)

    elif self.setting == 4:
      pulseMap = dataclasses.I3RecoPulseSeriesMap()
      frame["ContrivedPulseMap"] = pulseMap
      self.likelihood.SetEvent(frame)


    self.PushFrame(frame)

  def Finish(self):
    log_info("Finished the test module")

class TestLikelihood(unittest.TestCase):

  def test_get_name(self):
    tray = I3Tray()
    llhname  = lilliput.segments.add_pandel_likelihood_service(tray, "NoPulses", "MPE")
  
    tray.AddService("I3GSLRandomServiceFactory", "I3RandomService")
    
    tray.AddService("BootstrappingLikelihoodServiceFactory", "BSLikelihood",
            Pulses            = "ContrivedPulseMap",
            Bootstrapping     = gulliver_bootstrap.BootstrapOption.Multinomial,
            WrappedLikelihood = llhname,
            RandomService     = "I3RandomService"
            )
    tray.AddModule("BottomlessSource", "BS", Stream=icetray.I3Frame.Physics)
    tray.AddModule(TestModule, "TestModule", Setting=0)
    tray.Execute(1)  
    
  #This test will throw because there is no defauly likelihood with gradient
  @unittest.expectedFailure
  def test_likelihood_with_gradient(self):
    tray = I3Tray()
    llhname  = lilliput.segments.add_pandel_likelihood_service(tray, "NoPulses", "MPE")
  
    tray.AddService("I3GSLRandomServiceFactory", "I3RandomService")
    
    tray.AddService("BootstrappingLikelihoodServiceFactory", "BSLikelihood",
            Pulses            = "ContrivedPulseMap",
            Bootstrapping     = gulliver_bootstrap.BootstrapOption.Multinomial,
            WrappedLikelihood = llhname,
            RandomService     = "I3RandomService"
            )
    tray.AddModule("BottomlessSource", "BS", Stream=icetray.I3Frame.Physics)
    tray.AddModule(TestModule, "TestModule", Setting=1)
    tray.Execute(1)

  def test_has_gradient(self):
    tray = I3Tray()
    llhname  = lilliput.segments.add_pandel_likelihood_service(tray, "NoPulses", "MPE")
  
    tray.AddService("I3GSLRandomServiceFactory", "I3RandomService")
    
    tray.AddService("BootstrappingLikelihoodServiceFactory", "BSLikelihood",
            Pulses            = "ContrivedPulseMap",
            Bootstrapping     = gulliver_bootstrap.BootstrapOption.Multinomial,
            WrappedLikelihood = llhname,
            RandomService     = "I3RandomService"
            )
    tray.AddModule("BottomlessSource", "BS", Stream=icetray.I3Frame.Physics)
    tray.AddModule(TestModule, "TestModule", Setting=2)
    tray.Execute(1)

  #This test will throw because there is no pulse series in the frame
  @unittest.expectedFailure
  def test_nonexistant_pulses(self):
    tray = I3Tray()
    llhname  = lilliput.segments.add_pandel_likelihood_service(tray, "NoPulses", "MPE")
  
    tray.AddService("I3GSLRandomServiceFactory", "I3RandomService")
    
    tray.AddService("BootstrappingLikelihoodServiceFactory", "BSLikelihood",
            Pulses            = "THIS DOES NOT EXIST",
            Bootstrapping     = gulliver_bootstrap.BootstrapOption.Poisson,
            WrappedLikelihood = llhname,
            RandomService     = "I3RandomService"
            )
    tray.AddModule("BottomlessSource", "BS", Stream=icetray.I3Frame.Physics)
    tray.AddModule(TestModule, "TestModule", Setting=3)
    tray.Execute(1)

  #This will not throw, but will warn that the pulse series is empty
  def test_empty_pulse_map(self):
    tray = I3Tray()
    llhname  = lilliput.segments.add_pandel_likelihood_service(tray, "NoPulses", "MPE")
  
    tray.AddService("I3GSLRandomServiceFactory", "I3RandomService")
    
    tray.AddService("BootstrappingLikelihoodServiceFactory", "BSLikelihood",
            Pulses            = "ContrivedPulseMap",
            Bootstrapping     = gulliver_bootstrap.BootstrapOption.Multinomial,
            WrappedLikelihood = llhname,
            RandomService     = "I3RandomService"
            )
    tray.AddModule("BottomlessSource", "BS", Stream=icetray.I3Frame.Physics)
    tray.AddModule(TestModule, "TestModule", Setting=4)
    tray.Execute(1)

  #This test will throw because there is no random service in the context
  @unittest.expectedFailure
  def test_nonexistant_random_service(self):
    tray = I3Tray()
    tray.AddService("BootstrappingLikelihoodServiceFactory", "BSLikelihood",
            Bootstrapping     = gulliver_bootstrap.BootstrapOption.Poisson,
            WrappedLikelihood = "",
            RandomService     = "THIS DOES NOT EXIST"
            )
    tray.AddModule(TestModule, "TestModule", Setting=999)
    tray.Execute()

  #This will throw because there is no likelihood to wrap in the context
  @unittest.expectedFailure
  def test_nonexistant_wrapped_likelihood(self):
    tray = I3Tray()
    tray.AddService("I3GSLRandomServiceFactory", "I3RandomService")
    
    tray.AddService("BootstrappingLikelihoodServiceFactory", "BSLikelihood",
            Bootstrapping     = gulliver_bootstrap.BootstrapOption.Poisson,
            WrappedLikelihood = "THIS DOES NOT EXIST",
            RandomService     = "I3RandomService"
            )
    tray.AddModule(TestModule, "TestModule", Setting=999)
    tray.Execute()

unittest.main()