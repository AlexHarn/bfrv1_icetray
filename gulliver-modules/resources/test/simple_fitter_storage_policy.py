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

nevents = 10
pulses = "SRTOfflinePulses"

#This is module to interact with the fitter/frame and perform various tests
class TestModule(icetray.I3Module, unittest.TestCase):
    
  def __init__(self,ctx):
    icetray.I3Module.__init__(self,ctx)
    self.StoragePolicy = 0
    self.AddParameter("StoragePolicy",\
     "This mirrors the storage policy of the simple fitter", self.StoragePolicy)
        
  def Configure(self):
    self.StoragePolicy = self.GetParameter("StoragePolicy")
        
  def Physics(self,frame):

    if self.StoragePolicy == "OnlyBestFit":
      #These things should not be in the frame
      self.assertFalse("TestNstd" in frame)
      self.assertFalse("TestMeVect" in frame)
      self.assertFalse("TestMeFitParamsVect" in frame)

      #These should be in the frame
      self.assertTrue("TestMe" in frame)
      particle = frame["TestMe"]
      self.assertTrue(type(particle) == dataclasses.I3Particle)

      self.assertTrue("TestMeFitParams" in frame)
      params = frame["TestMeFitParams"]
      self.assertTrue(type(params) == gulliver.I3LogLikelihoodFitParams)

    elif self.StoragePolicy == "AllFitsAndFitParams":
      #These things should not be in the frame
      self.assertFalse("TestNstd" in frame)

      #These should be in the frame
      self.assertTrue("TestMe" in frame)
      particle = frame["TestMe"]
      self.assertTrue(type(particle) == dataclasses.I3Particle)

      self.assertTrue("TestMeVect" in frame)
      particleVec = frame["TestMeVect"]
      self.assertTrue(type(particleVec) == dataclasses.I3VectorI3Particle)

      self.assertTrue("TestMeFitParams" in frame)
      params = frame["TestMeFitParams"]
      self.assertTrue(type(params) == gulliver.I3LogLikelihoodFitParams)

      self.assertTrue("TestMeFitParamsVect" in frame)
      paramsVec = frame["TestMeFitParamsVect"]
      self.assertTrue(type(paramsVec) == gulliver.I3VectorI3LogLikelihoodFitParams)

    elif self.StoragePolicy == "AllFitsAndFitParamsNotInVectors":
      #These things should not be in the frame
      self.assertFalse("TestNstd" in frame)
      self.assertFalse("TestMeVect" in frame)
      self.assertFalse("TestMeFitParamsVect" in frame)

      #These should be in the frame
      for namepart in ["", "0", "1"]:
        name = "TestMe" + namepart
        self.assertTrue(("TestMe" + namepart) in frame)
        particle = frame[name]
        self.assertTrue(type(particle) == dataclasses.I3Particle)

        self.assertTrue(("TestMe" + namepart + "FitParams") in frame)
        params = frame["TestMe" + namepart + "FitParams"]
        self.assertTrue(type(params) == gulliver.I3LogLikelihoodFitParams)

    elif self.StoragePolicy == "AllResults":
      #These things should not be in the frame
      self.assertFalse("TestNstd" in frame)

      #These should be in the frame
      self.assertTrue("TestMe" in frame)
      particle = frame["TestMe"]
      self.assertTrue(type(particle) == dataclasses.I3Particle)

      self.assertTrue("TestMeVect" in frame)
      particleVec = frame["TestMeVect"]
      self.assertTrue(type(particleVec) == dataclasses.I3VectorI3Particle)

      self.assertTrue("TestMeFitParams" in frame)
      params = frame["TestMeFitParams"]
      self.assertTrue(type(params) == gulliver.I3LogLikelihoodFitParams)

      self.assertTrue("TestMeFitParamsVect" in frame)
      paramsVec = frame["TestMeFitParamsVect"]
      self.assertTrue(type(paramsVec) == gulliver.I3VectorI3LogLikelihoodFitParams)


    elif self.StoragePolicy == "AllResultsNotInVectors":
      #These things should not be in the frame
      self.assertFalse("TestNstd" in frame)
      self.assertFalse("TestMeVect" in frame)
      self.assertFalse("TestMeFitParamsVect" in frame)

      #These should be in the frame
      for namepart in ["", "0", "1"]:
        name = "TestMe" + namepart
        self.assertTrue(("TestMe" + namepart) in frame)
        particle = frame[name]
        self.assertTrue(type(particle) == dataclasses.I3Particle)

        self.assertTrue(("TestMe" + namepart + "FitParams") in frame)
        params = frame["TestMe" + namepart + "FitParams"]
        self.assertTrue(type(params) == gulliver.I3LogLikelihoodFitParams)

    else:
      self.assertTrue(False, "This setting is not being tested!")

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
                  FirstGuesses=["linefit", "linefit2"])

  # ----Modules------------------------------------------------------------------
  tray.AddModule("I3Reader", "reader",
                 FilenameList=[filename])

  tray.AddModule("I3LineFit", "linefit",
                 Name="linefit",
                 InputRecoPulses=pulses,
                 AmpWeightPower=1.)

  #Here we add a second line-fit so that we can have a "best" reco
  tray.AddModule("I3LineFit", "linefit2",
                 Name="linefit2",
                 InputRecoPulses=pulses,
                 AmpWeightPower=0.5)

  tray.AddModule("I3SimpleFitter",
                 StoragePolicy=storage,
                 OutputName="TestMe",
                 NonStdName="TestNstd",
                 SeedService="linefitseed",
                 Parametrization="xyzza",
                 LogLikelihood="spe1st",
                 Minimizer="minuit")

  tray.AddModule(TestModule, "TestModule", StoragePolicy=storage)

class TestFitterStoragePolicy(unittest.TestCase):
  def test_only_best_fit(self):
    tray = I3Tray()
    MakeTray(tray, "OnlyBestFit")
    tray.Execute(5)

  def test_all_fits_and_params(self):
    tray = I3Tray()
    MakeTray(tray, "AllFitsAndFitParams")
    tray.Execute(5)

  def test_all_fits_and_params_not_in_vec(self):
    tray = I3Tray()
    MakeTray(tray, "AllFitsAndFitParamsNotInVectors")
    tray.Execute(5)

  def test_all_results(self):
    tray = I3Tray()
    MakeTray(tray, "AllResults")
    tray.Execute(5)

  def test_all_results_not_in_vec(self):
    tray = I3Tray()
    MakeTray(tray, "AllResultsNotInVectors")
    tray.Execute(5)


unittest.main()