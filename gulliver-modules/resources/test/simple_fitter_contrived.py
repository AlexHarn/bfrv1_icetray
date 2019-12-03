import unittest
from I3Tray import *
import icecube
from icecube import icetray, dataio, dataclasses, lilliput, gulliver
import icecube.gulliver_modules

#These tests use the testing modules in lilliput to test all of the possible combinations
#of outputs and pitfalls

# icetray.i3logging.set_level(icetray.i3logging.I3LogLevel.LOG_DEBUG)

SEEDNAME="TestParticle"
MINIMIZERNAME="TestMinimizer"
LLHNAME="TestLikelihood"
PARAMNAME="TestParametrization"
OUTPUTNAME="SimpleFitterOut"

class PutParticleInFrame(icetray.I3Module, unittest.TestCase):
    
  def __init__(self,ctx):
    icetray.I3Module.__init__(self,ctx)
    self.status = dataclasses.I3Particle.FitStatus.OK
    self.AddParameter("FitStatus", "", self.status)
    self.energy = 1
    self.AddParameter("Energy", "", self.energy)

  def Configure(self):
    self.status = self.GetParameter("FitStatus")
    self.energy = self.GetParameter("Energy")

  def Physics(self, frame):
    particle = dataclasses.I3Particle()
    particle.dir = dataclasses.I3Direction(0.5, 0.5)
    particle.pos = dataclasses.I3Position(0,0,0)
    particle.energy = self.energy
    particle.fit_status=self.status
    frame[SEEDNAME] = particle
    self.assertTrue(SEEDNAME in frame, "Just want to be sure it is in")
    self.PushFrame(frame)

class TestResult(icetray.I3Module, unittest.TestCase):
    
  def __init__(self,ctx):
    icetray.I3Module.__init__(self,ctx)
    self.mode = 0
    self.AddParameter("Mode", "", self.mode)

  def Configure(self):
    self.mode = self.GetParameter("Mode")

  def Physics(self, frame):
    if self.mode == 0:
      status = frame[OUTPUTNAME].fit_status
      self.assertTrue(status == dataclasses.I3Particle.FitStatus.MissingSeed)
    elif self.mode == 1:
      miniDiagnostic = frame[OUTPUTNAME+"_"+MINIMIZERNAME]
      self.assertTrue(miniDiagnostic == 42)
      paramDiagnostic = frame[OUTPUTNAME+"_"+PARAMNAME]
      self.assertTrue(paramDiagnostic == 42)
      llhDiagnostic = frame[OUTPUTNAME+"_"+LLHNAME]
      self.assertTrue(llhDiagnostic == 42)
      nonstd = frame[OUTPUTNAME + "Params"]
      self.assertTrue(nonstd == 42)

#This is a little helper function to make the tray for each test
def MakeTray(seedName=SEEDNAME, diagnostic=False):
  tray = I3Tray()
  tray.AddService("I3TestMinimizerFactory", MINIMIZERNAME,
                  SetDiagnostics=diagnostic)
  tray.AddService("I3TestParametrizationFactory", PARAMNAME,
                  SetDiagnostics=diagnostic,
                  SetNonStd=diagnostic)
  tray.AddService("I3TestLikelihoodFactory", LLHNAME,
                  Multiplicity=10,
                  SetDiagnostics=diagnostic)
  tray.AddService("I3BasicSeedServiceFactory", "linefitseed",
                  FirstGuesses=[seedName])
  #Make a dummy PFrame
  tray.AddModule("BottomlessSource", "BS", Stream=icetray.I3Frame.Physics)
  tray.AddModule(PutParticleInFrame, "PutParticleInFrame")
  tray.AddModule("I3SimpleFitter", "SimpleFitter",
                 SeedService="linefitseed",
                 Minimizer=MINIMIZERNAME,
                 Parametrization=PARAMNAME,
                 LogLikelihood=LLHNAME,
                 OutputName=OUTPUTNAME
                 )  
  return tray

class TestSimpleFitterContrived(unittest.TestCase):

  #Test when there are no seeds (particles in the frame)
  def test_nonexistant_seed(self):
    tray = MakeTray(seedName = "")
    tray.AddModule(TestResult, "TestResult",
                   Mode=0
                  )
    tray.Execute(1)

  #The llh, parametrization, and minimizer can output diagnostic tools
  #Check to make sure they are correctly in the frame 
  def test_diagnostics(self):
    tray = MakeTray(diagnostic=True)
    tray.AddModule(TestResult, "TestResult",
                   Mode=1
                  )
    tray.Execute(1)

unittest.main()