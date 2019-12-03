import unittest
from I3Tray import *
import icecube
from icecube import icetray, dataio, dataclasses, lilliput, gulliver
import icecube.gulliver_modules

#The loglikelihood calculator requires a likelihood service, a
#parameterization, and a particle. It these do not exist, the
#code should fail or at least have predictable behavior.
#These are tested below
class PutStuffInFrame(icetray.I3Module, unittest.TestCase):
    
  def __init__(self,ctx):
    icetray.I3Module.__init__(self,ctx)
    self.energy = 1
    self.AddParameter("Energy", "Particle energy", self.energy)
    self.setTime = True
    self.AddParameter("SetTime", "Set the particle time?", self.setTime)
    self.setDir = True
    self.AddParameter("SetDir", "Set the particle direction?", self.setDir)

  def Configure(self):
    self.energy = self.GetParameter("Energy")
    self.setTime = self.GetParameter("SetTime")
    self.setDir = self.GetParameter("SetDir")

  def Physics(self, frame):
    particle = dataclasses.I3Particle()
    if self.setDir:
      particle.dir = dataclasses.I3Direction(0.5, 0.5)
    particle.pos = dataclasses.I3Position(0,0,0)
    if self.setTime:
      particle.time = 1.
    particle.energy = self.energy
    particle.shape = dataclasses.I3Particle.StartingTrack
    particle.fit_status=dataclasses.I3Particle.FitStatus.OK
    frame["TestParticle"] = particle
    self.assertTrue("TestParticle" in frame, "Just want to be sure it is in")
    self.PushFrame(frame)

def MakeTray(useLikelihood, useParam, placeParticle,\
             setDir=True, energy=1, setTime=True, llhName="theLLH", particleName="TestParticle"):
  tray = I3Tray()

  if useLikelihood:
    tray.AddService("I3GulliverIPDFPandelFactory", "theLLH",
                      InputReadout="Dummy",
                      Likelihood="SPE1st",
                      PEProb="GaussConvoluted",
                      IceModel=2,
                      NoiseProbability=1000.*icecube.icetray.I3Units.hertz,
                      JitterTime=15.*icecube.icetray.I3Units.ns)

  if useParam:
    tray.AddService("I3SimpleParametrizationFactory", "theParam")

  #Make a dummy PFrame
  tray.AddModule("BottomlessSource", "BS", Stream=icetray.I3Frame.Physics)

  if placeParticle:  
    tray.AddModule(PutStuffInFrame, "PutStuffInFrame",
                   Energy = energy,
                   SetTime = setTime,
                   SetDir = setDir)

  tray.AddModule("I3LogLikelihoodCalculator", "llhCalc",
                 LogLikelihoodService=llhName,
                 FitName=particleName,
                 NFreeParameters=0
                 )

  tray.Execute(1)


class TestLikelihoodCalculatorFails(unittest.TestCase):

  #This test will throw because no name is supplied
  @unittest.expectedFailure
  def test_no_llh_name_given(self):
    MakeTray(False,False,False,llhName="")

  #This test will throw because the llh does not exist in the context
  @unittest.expectedFailure
  def test_nonexistant_llh(self):
    MakeTray(False,False,False)

  #This test will throw because no particle name is supplied
  @unittest.expectedFailure
  def test_no_particle_name_given(self):
    MakeTray(True,False,False,particleName="")

  #The module allows for particles to not exist in the frame, but should warn
  def test_nonexistant_particle(self):
    MakeTray(True,False,False)

  #Test the various ways that the particle can be incorrectly initialized
  def test_bad_particle_energy(self):
    MakeTray(True,True,True,energy=-1)
  def test_bad_particle_time(self):
    MakeTray(True,True,True,setTime=False)
  def test_bad_particle_direction(self):
    MakeTray(True,True,True,setDir=False)

unittest.main() 