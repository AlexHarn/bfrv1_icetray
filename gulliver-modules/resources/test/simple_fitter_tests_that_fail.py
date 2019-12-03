import unittest
from I3Tray import *
import icecube
from icecube import icetray, dataio, dataclasses, lilliput, gulliver
import icecube.gulliver_modules

#The simple fitter requires a minimizer, parameterization,
#loglikelihood, and seed service. If any of these do not exist
#The script should fail. Here we test many cases where we give
#bad inputs or no inputs at all to the various modules
#Each of the following tests should fail

class TestSimpleFitterFails(unittest.TestCase):

  #This test will throw because there is no minimizer in the context
  @unittest.expectedFailure
  def test_nonexistant_minimizer(self):
    tray = I3Tray()
    tray.AddModule("I3SimpleFitter")
    tray.Execute(1)

  #This test will throw because there is no parameterization in the context
  @unittest.expectedFailure
  def test_nonexistant_parameterization(self):
    tray = I3Tray()
    tray.AddService("I3GulliverMinuitFactory", "minuit",
                    Algorithm="SIMPLEX")
    tray.AddModule("I3SimpleFitter",
                   Minimizer="minuit")
    tray.Execute(1)

  #This test will throw because there is no likelihood in the context
  @unittest.expectedFailure
  def test_nonexistant_likelihood(self):
    tray = I3Tray()
    tray.AddService("I3SimpleParametrizationFactory", "xyzza")
    tray.AddService("I3GulliverMinuitFactory", "minuit",
                    Algorithm="SIMPLEX")
    tray.AddModule("I3SimpleFitter",
                   Parametrization="xyzza",
                   Minimizer="minuit")
    tray.Execute(1)

  #This test will throw because there is no seedservice in the context
  @unittest.expectedFailure
  def test_nonexistant_seedservice(self):
    tray = I3Tray()
    tray.AddService("I3SimpleParametrizationFactory", "xyzza")
    tray.AddService("I3GulliverMinuitFactory", "minuit",
                    Algorithm="SIMPLEX")
    tray.AddService("I3GulliverIPDFPandelFactory", "spe1st",
                    InputReadout="Dummy",
                    Likelihood="SPE1st",
                    PEProb="GaussConvoluted",
                    IceModel=2,
                    NoiseProbability=1000.*icecube.icetray.I3Units.hertz,
                    JitterTime=15.*icecube.icetray.I3Units.ns)
    tray.AddModule("I3SimpleFitter",
                   Parametrization="xyzza",
                   LogLikelihood="spe1st",
                   Minimizer="minuit")
    tray.Execute(1)

  #This test will throw because of a bad storage policy as input
  @unittest.expectedFailure
  def test_nonexistant_storage_policy(self):
    tray = I3Tray()
    tray.AddService("I3SimpleParametrizationFactory", "xyzza",
                    StepX=20.*icecube.icetray.I3Units.m,
                    StepY=20.*icecube.icetray.I3Units.m,
                    StepZ=20.*icecube.icetray.I3Units.m)
    tray.AddService("I3GulliverMinuitFactory", "minuit",
                    Algorithm="SIMPLEX")
    tray.AddService("I3GulliverIPDFPandelFactory", "spe1st",
                    InputReadout="Dummy",
                    Likelihood="SPE1st",
                    PEProb="GaussConvoluted",
                    IceModel=2,
                    NoiseProbability=1000.*icecube.icetray.I3Units.hertz,
                    JitterTime=15.*icecube.icetray.I3Units.ns)

    tray.AddService("I3BasicSeedServiceFactory", "linefitseed",
                    InputReadout="Dummy",
                    TimeShiftType="TFirst",
                    FirstGuesses=["linefit"])
    tray.AddModule("I3SimpleFitter",
                   SeedService="linefitseed",
                   Parametrization="xyzza",
                   LogLikelihood="spe1st",
                   StoragePolicy="THIS POLICY IS FOR TESTING",
                   Minimizer="minuit")
    tray.Execute(1)

  #This test will throw because of a trace as input
  @unittest.expectedFailure
  def test_nonexistant_trace_mode(self):
    tray = I3Tray()
    tray.AddService("I3SimpleParametrizationFactory", "xyzza",
                    StepX=20.*icecube.icetray.I3Units.m,
                    StepY=20.*icecube.icetray.I3Units.m,
                    StepZ=20.*icecube.icetray.I3Units.m)
    tray.AddService("I3GulliverMinuitFactory", "minuit",
                    Algorithm="SIMPLEX")
    tray.AddService("I3GulliverIPDFPandelFactory", "spe1st",
                    InputReadout="Dummy",
                    Likelihood="SPE1st",
                    PEProb="GaussConvoluted",
                    IceModel=2,
                    NoiseProbability=1000.*icecube.icetray.I3Units.hertz,
                    JitterTime=15.*icecube.icetray.I3Units.ns)

    tray.AddService("I3BasicSeedServiceFactory", "linefitseed",
                    InputReadout="Dummy",
                    TimeShiftType="TFirst",
                    FirstGuesses=["linefit"])
    tray.AddModule("I3SimpleFitter",
                   SeedService="linefitseed",
                   Parametrization="xyzza",
                   LogLikelihood="spe1st",
                   TraceMode = "TRACE MODE FOR TESTING",
                   Minimizer="minuit")
    tray.Execute(1)

  #This test will throw because the ouput has not been set
  @unittest.expectedFailure
  def test_nonexistant_trace_mode(self):
    tray = I3Tray()
    tray.AddService("I3SimpleParametrizationFactory", "xyzza",
                    StepX=20.*icecube.icetray.I3Units.m,
                    StepY=20.*icecube.icetray.I3Units.m,
                    StepZ=20.*icecube.icetray.I3Units.m)
    tray.AddService("I3GulliverMinuitFactory", "minuit",
                    Algorithm="SIMPLEX")
    tray.AddService("I3GulliverIPDFPandelFactory", "spe1st",
                    InputReadout="Dummy",
                    Likelihood="SPE1st",
                    PEProb="GaussConvoluted",
                    IceModel=2,
                    NoiseProbability=1000.*icecube.icetray.I3Units.hertz,
                    JitterTime=15.*icecube.icetray.I3Units.ns)

    tray.AddService("I3BasicSeedServiceFactory", "linefitseed",
                    InputReadout="Dummy",
                    TimeShiftType="TFirst",
                    FirstGuesses=["linefit"])
    tray.AddModule("I3SimpleFitter",
                   SeedService="linefitseed",
                   Parametrization="xyzza",
                   LogLikelihood="spe1st",
                   TraceMode = "All",
                   Minimizer="minuit")
    tray.Execute(1)

unittest.main()