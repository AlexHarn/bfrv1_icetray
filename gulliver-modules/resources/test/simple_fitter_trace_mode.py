import unittest
from I3Tray import *
import icecube
from icecube import icetray, dataio, dataclasses, lilliput, gulliver, linefit

if "I3_TESTDATA" in os.environ:
    testdata = os.environ["I3_TESTDATA"]
else:
    raise IOError("Cannot find test data file, please define I3_TESTDATA")
                  
filename = os.path.join(
        testdata, "event-viewer", "Level3aGCD_IC79_EEData_Run00115990.i3")

pulses = "SRTOfflinePulses"

#This is module to interact with the fitter/frame and perform various tests
class TestModule(icetray.I3Module, unittest.TestCase):
    
  def __init__(self,ctx):
    icetray.I3Module.__init__(self,ctx)
        
  def Physics(self,frame):

    #First we do a check that the trace setting does not affect the reconstruction
    #Three recos run with the three trace settings, check outputs against each other
    for iname in ["TestMe1", "TestMe2", "TestMe3"]:
      #These should be in the frame
      self.assertTrue(iname in frame)
      iparticle = frame[iname]
      self.assertTrue(type(iparticle) == dataclasses.I3Particle)

      self.assertTrue((iname+"FitParams") in frame)
      iparams = frame[(iname+"FitParams")]
      self.assertTrue(type(iparams) == gulliver.I3LogLikelihoodFitParams)

      for jname in ["TestMe1", "TestMe2", "TestMe3"]:
        if iname == jname:
          continue

        #These should be in the frame
        self.assertTrue(jname in frame)
        jparticle = frame[jname]
        self.assertTrue(type(jparticle) == dataclasses.I3Particle)

        self.assertTrue((jname+"FitParams") in frame)
        jparams = frame[(jname+"FitParams")]
        self.assertTrue(type(jparams) == gulliver.I3LogLikelihoodFitParams)

        self.assertTrue(iparticle == jparticle, "The trace mode setting affects the reconstruction!")
        self.assertTrue(iparams == jparams, "The trace mode setting affects the reconstruction!")

    #Assert that all the things that are supposed to be in the frame are there and nothing else is
    self.assertFalse("TestMe1_TRACE" in frame, "Setting \"None\" still puts trace in frame!")
    self.assertFalse("TestMe1_TRACE0000" in frame, "Setting \"None\" still puts trace in frame!")

    self.assertTrue("TestMe2_TRACE" in frame, "No trace found in frame for setting \"Single\"")
    self.assertFalse("TestMe2_TRACE0000" in frame, "Setting \"Single\" still puts numbered trace in frame!")

    self.assertFalse("TestMe3_TRACE" in frame, "Setting \"All\" still puts single trace in frame!")
    self.assertTrue("TestMe3_TRACE0000" in frame, "Did not find the numbered trace in the frame for \"All\"")

    self.PushFrame(frame)


#This is a little helper script to set up the tray over and over
#for the various tests
def MakeTray(tray, storage):
  tray.AddService("I3SimpleParametrizationFactory", "xyzza",
                  StepX=20.*icecube.icetray.I3Units.m,
                  StepY=20.*icecube.icetray.I3Units.m,
                  StepZ=20.*icecube.icetray.I3Units.m,
                  StepZenith=0.1*icecube.icetray.I3Units.radian,
                  StepAzimuth=0.2*icecube.icetray.I3Units.radian,
                  BoundsX=[-2000.*icecube.icetray.I3Units.m,
                           2000.*icecube.icetray.I3Units.m],
                  BoundsY=[-2000.*icecube.icetray.I3Units.m,
                           2000.*icecube.icetray.I3Units.m],
                  BoundsZ=[-2000.*icecube.icetray.I3Units.m,
                           2000.*icecube.icetray.I3Units.m])

  tray.AddService("I3GulliverMinuitFactory", "minuit",
                  Algorithm="SIMPLEX")

  tray.AddService("I3GulliverIPDFPandelFactory", "spe1st",
                  InputReadout=pulses,
                  Likelihood="SPE1st",
                  PEProb="GaussConvoluted",
                  IceModel=2,
                  NoiseProbability=1000.*icecube.icetray.I3Units.hertz,
                  JitterTime=15.*icecube.icetray.I3Units.ns)

  tray.AddService("I3BasicSeedServiceFactory", "linefitseed",
                  InputReadout=pulses,
                  TimeShiftType="TFirst",
                  FirstGuesses=["linefit"])

  # ----Modules------------------------------------------------------------------
  tray.AddModule("I3Reader", "reader",
                 FilenameList=[filename])

  tray.AddModule("I3LineFit", "linefit",
                 Name="linefit",
                 InputRecoPulses=pulses,
                 AmpWeightPower=1.)

  tray.AddModule("I3SimpleFitter", "SimpleFitter1",
                 TraceMode="None",
                 StoragePolicy="OnlyBestFit",
                 OutputName="TestMe1",
                 SeedService="linefitseed",
                 Parametrization="xyzza",
                 LogLikelihood="spe1st",
                 Minimizer="minuit")

  tray.AddModule("I3SimpleFitter", "SimpleFitter2",
                 TraceMode="Single",
                 StoragePolicy="OnlyBestFit",
                 OutputName="TestMe2",
                 SeedService="linefitseed",
                 Parametrization="xyzza",
                 LogLikelihood="spe1st",
                 Minimizer="minuit")

  tray.AddModule("I3SimpleFitter", "SimpleFitter3",
                 TraceMode="All",
                 StoragePolicy="OnlyBestFit",
                 OutputName="TestMe3",
                 SeedService="linefitseed",
                 Parametrization="xyzza",
                 LogLikelihood="spe1st",
                 Minimizer="minuit")

  tray.AddModule(TestModule, "TestModule")

class TestTraceMode(unittest.TestCase):
  def test_trace_mode(self):
    tray = I3Tray()
    MakeTray(tray, "OnlyBestFit")
    tray.Execute(5)


unittest.main()