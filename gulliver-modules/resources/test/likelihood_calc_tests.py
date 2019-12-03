import unittest
from I3Tray import *
import icecube
from icecube import icetray, dataio, dataclasses, lilliput, gulliver
import icecube.gulliver_modules

#The loglikelihood calculator requires a likelihood service, a
#parameterization, and a particle. It these do not exist, the
#code should fail or at least have predictable behavior.
#These are tested below

class PutParticleInFrame(icetray.I3Module):
    
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
    frame["TestParticle"] = particle
    self.assertTrue("TestParticle" in frame, "Just want to be sure it is in")
    self.PushFrame(frame)


def MakeTray(useLikelihood, useParam, placeParticle, llhName="theLLH", particleName="TestParticle"):
  tray = I3Tray()
  
  tray.AddService("I3GulliverIPDFPandelFactory", "theLLH",
                    InputReadout="Dummy",
                    Likelihood="SPE1st",
                    PEProb="GaussConvoluted",
                    IceModel=2,
                    NoiseProbability=1000.*icecube.icetray.I3Units.hertz,
                    JitterTime=15.*icecube.icetray.I3Units.ns)

  tray.AddModule(PutParticleInFrame, "Putter",
                 FitStatus = dataclasses.I3Particle.FitStatus.OK,
                 Energy = 1)

  #Make a dummy PFrame
  tray.AddModule("BottomlessSource", "BS", Stream=icetray.I3Frame.Physics)

  tray.Execute(1)


class TestLikelihoodCalculatorFails(unittest.TestCase):

  def test_no_llh_name_given(self):
    MakeTray(False,False,False,llhName="")

unittest.main()