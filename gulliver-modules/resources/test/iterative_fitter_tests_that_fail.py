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
RANDOMNAME=""
COSRANGE=[0,1]

#This is a little helper function to make the tray for each test
def MakeTray(useMini=True, useParam=True, useLLH=True, useSeedServ=True,\
             seedName=SEEDNAME, outName=OUTPUTNAME, randName=RANDOMNAME,\
             cosRange=COSRANGE):
  tray = I3Tray()
  if useMini:
    tray.AddService("I3TestMinimizerFactory", MINIMIZERNAME)
  if useParam:
    tray.AddService("I3TestParametrizationFactory", PARAMNAME)
  if useLLH:
    tray.AddService("I3TestLikelihoodFactory", LLHNAME)

  if useSeedServ:
    tray.AddService("I3BasicSeedServiceFactory", "linefitseed",
                    FirstGuesses=[seedName])

  #Make a dummy PFrame
  tray.AddModule("BottomlessSource", "BS", Stream=icetray.I3Frame.Physics)
  tray.AddModule("I3IterativeFitter", "SimpleFitter",
                 SeedService="linefitseed",
                 Minimizer=MINIMIZERNAME,
                 Parametrization=PARAMNAME,
                 LogLikelihood=LLHNAME,
                 OutputName=outName,
                 RandomService=randName,
                 CosZenithRange=cosRange
                 )  
  return tray

class TestSimpleFitterContrived(unittest.TestCase):

  #Should throw because there is no minimizer in context
  @unittest.expectedFailure
  def test_nonexistant_minimizer(self):
    tray = MakeTray(useMini=False)
    tray.Execute(1)

  #Should throw because there is no paramitrization in context
  @unittest.expectedFailure
  def test_nonexistant_paramitrization(self):
    tray = MakeTray(useParam=False)
    tray.Execute(1)

  #Should throw because there is no likelihood in context
  @unittest.expectedFailure
  def test_nonexistant_likelihood(self):
    tray = MakeTray(useLLH=False)
    tray.Execute(1)

  #Should throw because there is no seed service in context
  @unittest.expectedFailure
  def test_nonexistant_seed_service(self):
    tray = MakeTray(useSeedServ=False)
    tray.Execute(1)

  #Should throw because there is no output name is given
  @unittest.expectedFailure
  def test_non_output_name_given(self):
    tray = MakeTray(outName="")
    tray.Execute(1)

  #Should throw because there is no random service in context
  @unittest.expectedFailure
  def test_nonexistant_random_service(self):
    tray = MakeTray(randName="BigTed")
    tray.Execute(1)

  #Should throw because there are three numbers for the cosine range
  @unittest.expectedFailure
  def test_too_many_range_limits(self):
    tray = MakeTray(cosRange=[0,0.5,1])
    tray.Execute(1)

  #Should throw because the range values are out of order
  @unittest.expectedFailure
  def test_bad_order_range_limits(self):
    tray = MakeTray(cosRange=[1,0])
    tray.Execute(1)

  #Should throw because the range values are out of bounds
  @unittest.expectedFailure
  def test_range_limits_out_of_bounds(self):
    tray = MakeTray(cosRange=[-1.1,1])
    tray.Execute(1)

unittest.main()