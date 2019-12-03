import unittest

from I3Tray import *
from icecube import icetray, dataio, dataclasses
from icecube import gulliver_bootstrap

class TestSeedTweak(unittest.TestCase):

  def test_not_found_in_frame(self):
    #Make sure nothing ran
    def NotFoundInFrame(frame):
      assert(not frame.Has("AngleError"))
      assert(not frame.Has("AngleErrorParams"))
      assert(not frame.Has("AngleErrorParams"))


    # build tray
    tray = I3Tray()

    tray.AddModule("BottomlessSource", "BS", Stream=icetray.I3Frame.Physics)

    # # print em
    # tray.AddModule("Dump")

    tray.AddModule("BootstrapSeedTweak", "BootstrapSeedTweak",
            BootstrappedRecos = "BSRecos",
            ContainmentLevel  = 0.5,
            AngularError      = "AngleError"
            )
    tray.AddModule(NotFoundInFrame, "NotFoundInFrame")

    # run tray
    tray.Execute(1)

  def test_empty_particle_vector(self):

    #Puts an empty vector of particles into the frame
    def LoadIntoFrame(frame):
      frame["BSRecos"] = dataclasses.I3VectorI3Particle()

    #Make sure the module puts in the correct frame objects
    def CheckOutput(frame):
      assert(frame.Has("AngleError"))
      assert(frame.Has("AngleErrorParams"))
      params = frame["AngleErrorParams"]
      assert(params.status == gulliver_bootstrap.BootstrapParams.NoValidFits)
      assert(params.successfulFits == 0)
      assert(params.totalFits == 0)

    # build tray
    tray = I3Tray()

    #Make a dummy PFrame
    tray.AddModule("BottomlessSource", "BS", Stream=icetray.I3Frame.Physics)

    tray.AddModule(LoadIntoFrame, "LoadIntoFrame")

    tray.AddModule("BootstrapSeedTweak", "BootstrapSeedTweak",
            BootstrappedRecos = "BSRecos",
            ContainmentLevel  = 0.5,
            AngularError      = "AngleError"
            )
    tray.AddModule(CheckOutput, "CheckOutput")

    # run tray
    tray.Execute(1)


  def test_bad_containment_part1(self):
    print("Running containment test")

    def GetParticle(status, dir):
      particle = dataclasses.I3Particle()
      particle.pos = dataclasses.I3Position(1,2,3)
      particle.dir = dir
      particle.fit_status = status

      return particle

    #Puts an empty vector of particles into the frame
    def LoadIntoFrame(frame):
      particleVec = dataclasses.I3VectorI3Particle()

      particle = GetParticle(dataclasses.I3Particle.FitStatus.OK, dataclasses.I3Direction(0,0))
      particleVec.append(particle)

      particle2 = GetParticle(dataclasses.I3Particle.FitStatus.GeneralFailure, dataclasses.I3Direction(0,0))
      particleVec.append(particle2)

      frame["BSRecos"] = particleVec

    #Make sure the module puts in the correct frame objects
    def CheckOutput(frame):
      ####Check the underflow settings####
      assert(frame.Has("AngleError"))
      assert(frame.Has("AngleErrorParams"))

      angEstimate = frame["AngleError"]
      self.assertEqual(angEstimate, 0)

      params = frame["AngleErrorParams"]
      assert(params.status == gulliver_bootstrap.BootstrapParams.Underflow)
      assert(params.successfulFits == 1)
      assert(params.totalFits == 2)

      ####Check the overflow output####
      assert(frame.Has("AngleError2"))
      assert(frame.Has("AngleError2Params"))

      angEstimate = frame["AngleError2"]
      self.assertEqual(angEstimate, 0)

      params = frame["AngleError2Params"]
      assert(params.status == gulliver_bootstrap.BootstrapParams.Overflow)

    # build tray
    tray = I3Tray()

    #Make a dummy PFrame
    tray.AddModule("BottomlessSource", "BS", Stream=icetray.I3Frame.Physics)

    tray.AddModule(LoadIntoFrame, "LoadIntoFrame")

    tray.AddModule("BootstrapSeedTweak", "BootstrapSeedTweak",
            BootstrappedRecos = "BSRecos",
            ContainmentLevel  = -10,
            AngularError      = "AngleError"
            )

    tray.AddModule("BootstrapSeedTweak", "BootstrapSeedTweak2",
            BootstrappedRecos = "BSRecos",
            ContainmentLevel  = 10,
            AngularError      = "AngleError2"
            )

    tray.AddModule(CheckOutput, "CheckOutput")

    # run tray
    tray.Execute(1)

    print("Done with containment test")

unittest.main()