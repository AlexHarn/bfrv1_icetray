#!/usr/bin/env python

import unittest
import numpy as np

from I3Tray import *
import icecube
from icecube import icetray, dataio, dataclasses, lilliput, gulliver
from icecube.icetray.i3logging import *

class TestModule(icetray.I3Module, unittest.TestCase):
    
  def __init__(self,ctx):
    icetray.I3Module.__init__(self,ctx)
    self.setting = 0
    self.AddParameter("Setting", "Switch for the testing module", self.setting)
    self.combiner = 0
    self.mults = []
    self.AddParameter("Multiplicity", "List of multiplicities", self.setting)
    self.weights = []
    self.AddParameter("Weights", "List of weights", self.setting)
    self.llhs = []
    self.AddParameter("LLHs", "List of llhs", self.setting)
 
  def Configure(self):
    self.setting = self.GetParameter("Setting")
    self.assertTrue("Combiner" in self.context, "The combiner is not properly installed")
    self.combiner = self.context["Combiner"]
    self.mults = self.GetParameter("Multiplicity")
    self.weights = self.GetParameter("Weights")
    self.llhs = self.GetParameter("LLHs")

  def Geometry(self, frame):
    self.PushFrame(frame)
        
  def Physics(self,frame):
    if self.setting == 0:
      self.assertTrue(self.combiner)
      geom = dataclasses.I3Geometry()
      self.combiner.SetGeometry(geom)
    elif self.setting == 1:
      self.assertTrue(self.combiner)
      self.combiner.SetEvent(frame)
    elif self.setting == 2:
      self.assertTrue(self.combiner)
      self.assertTrue(self.combiner.HasGradient())
    elif self.setting == 3:
      self.assertTrue(self.combiner)
      self.assertFalse(self.combiner.HasGradient())
    elif self.setting == 4:
      self.assertTrue(self.combiner)
      particle = dataclasses.I3Particle()
      hypothesis = gulliver.I3EventHypothesis(particle)
      llh = self.combiner.GetLogLikelihood(hypothesis)
      expected = (np.array(self.weights) * np.array(self.llhs)).sum()
      self.assertEqual(llh, expected)
    elif self.setting == 5:
      self.assertTrue(self.combiner)
      particle = dataclasses.I3Particle()
      hypothesis = gulliver.I3EventHypothesis(particle)
      llh = self.combiner.GetLogLikelihoodWithGradient(hypothesis, hypothesis, 1)
      expected = (np.array(self.weights) * np.array(self.llhs)).sum()
      self.assertEqual(llh, expected)
    elif self.setting == 6:
      self.assertTrue(self.combiner)
      particle = dataclasses.I3Particle()
      hypothesis = gulliver.I3EventHypothesis(particle)
      diag = self.combiner.GetDiagnostics(hypothesis)

    self.PushFrame(frame)

def SetUpTray(setting=0, inputs=["LLH1","LLH2","LLH3"],
              weights=[1,1,1], mode="Sum",
              mults=[1,2,3],
              grads=[True,True,True],
              llhs=[1,2,3]):

    tray = I3Tray()
    tray.AddService("I3TestLikelihoodFactory", "LLH1",
                    LikelihoodValue=llhs[0],
                    Multiplicity=mults[0],
                    HasGradient=grads[0])
    tray.AddService("I3TestLikelihoodFactory", "LLH2",
                    LikelihoodValue=llhs[1],
                    Multiplicity=mults[1],
                    HasGradient=grads[1])
    tray.AddService("I3TestLikelihoodFactory", "LLH3",
                    LikelihoodValue=llhs[2],
                    Multiplicity=mults[2],
                    HasGradient=grads[2])
    tray.AddService("I3TestMinimizerFactory", "DummyMini")
    tray.AddService("I3EventLogLikelihoodCombinerFactory", "Combiner",
                    InputLogLikelihoods = inputs,
                    RelativeWeights=weights,
                    Multiplicity=mode)

    #Make a dummy PFrame
    tray.AddModule("BottomlessSource", "BS", Stream=icetray.I3Frame.Physics)

    return tray

class Test_Likelihood_Combiner_Failures(unittest.TestCase):

    #This test will throw because there are no LLHs supplied to the combiner
    @unittest.expectedFailure
    def test_nothing_supplied(self):
        tray = SetUpTray(inputs=[])
        tray.Execute(1)

    #This test will throw because I did not pass in something parseable
    @unittest.expectedFailure
    def test_not_a_string(self):
        tray = SetUpTray(inputs=[1])
        tray.Execute(1)

    #This test will throw because the given name is not in the context
    @unittest.expectedFailure
    def test_nonexistant_likelihood(self):
        tray = SetUpTray(inputs=["THIS_DOES_NOT_EXIST"])
        tray.Execute(1)

    #This test will throw because we gave something that was not a likelihood
    @unittest.expectedFailure
    def test_not_a_likelihood(self):
        tray = SetUpTray(inputs=["DummyMini"])
        tray.Execute(1)

    #This test will throw because #LLH != #weights
    @unittest.expectedFailure
    def test_not_equal_lists(self):
        tray = SetUpTray(inputs=["LLH1"], weights=[1,2])
        tray.Execute(1)

    #This test will throw because invalid mode was given
    @unittest.expectedFailure
    def test_bad_mode_name(self):
        tray = SetUpTray(mode="EDNA_MODE")
        tray.Execute(1)

    #This test will throw because only some LLHs have gradients
    @unittest.expectedFailure
    def test_bad_gradient(self):
        tray = SetUpTray(grads=[True, False, False])
        tray.AddModule(TestModule, "TestModule", Setting=2)
        tray.Execute(1)


class Test_Likelihood_Combiner(unittest.TestCase):
    def test_set_geometry(self):
        tray = SetUpTray()
        tray.Execute(1)
        combiner = tray.context["Combiner"]
        geom = dataclasses.I3Geometry()
        combiner.SetGeometry(geom)

    def test_set_event(self):
        tray = SetUpTray()
        tray.Execute(1)
        combiner = tray.context["Combiner"]
        frame = icetray.I3Frame()
        combiner.SetEvent(frame)

    def test_has_gradient(self):
        tray = SetUpTray()
        tray.Execute(1)
        combiner = tray.context["Combiner"]
        self.assertTrue(combiner.HasGradient())

    def test_has_no_gradient(self):
        tray = SetUpTray(grads=[False,False,False])
        tray.Execute(1)
        combiner = tray.context["Combiner"]
        self.assertFalse(combiner.HasGradient())

    def test_llh_correctly_calculated(self):
        particle = dataclasses.I3Particle()
        hypothesis = gulliver.I3EventHypothesis(particle)

        tray = SetUpTray(weights=[1,1,1], llhs=[1,2,3])
        tray.Execute(1)
        combiner = tray.context["Combiner"]
        self.assertTrue(6 == combiner.GetLogLikelihood(hypothesis))

        tray = SetUpTray(weights=[1,1,3], llhs=[1,2,3])
        tray.Execute(1)
        combiner = tray.context["Combiner"]
        self.assertTrue(12 == combiner.GetLogLikelihood(hypothesis))

        tray = SetUpTray(weights=[1,1,1], llhs=[10,2,3])
        tray.Execute(1)
        combiner = tray.context["Combiner"]
        self.assertTrue(15 == combiner.GetLogLikelihood(hypothesis))

    def test_llh_with_grad_correctly_calculated(self):
        particle = dataclasses.I3Particle()
        hypothesis = gulliver.I3EventHypothesis(particle)

        tray = SetUpTray(weights=[1,1,1], llhs=[1,2,3])
        tray.Execute(1)
        combiner = tray.context["Combiner"]
        self.assertTrue(6 == combiner.GetLogLikelihoodWithGradient(hypothesis, hypothesis, 1))

        tray = SetUpTray(weights=[1,1,3], llhs=[1,2,3])
        tray.Execute(1)
        combiner = tray.context["Combiner"]
        self.assertTrue(12 == combiner.GetLogLikelihoodWithGradient(hypothesis, hypothesis, 1))

    def test_get_diagnostic(self):
        particle = dataclasses.I3Particle()
        hypothesis = gulliver.I3EventHypothesis(particle)

        tray = SetUpTray()
        tray.Execute(1)
        combiner = tray.context["Combiner"]
        self.assertTrue(combiner.GetDiagnostics(hypothesis) is None)

    def test_get_multiplicity(self):
        tray = SetUpTray(mults=[1,2,3], mode="Sum")
        tray.Execute(1)
        combiner = tray.context["Combiner"]
        self.assertAlmostEqual(6, combiner.GetMultiplicity())

        tray = SetUpTray(mults=[1,2,3], mode="Max")
        tray.Execute(1)
        combiner = tray.context["Combiner"]
        self.assertAlmostEqual(3, combiner.GetMultiplicity())

        tray = SetUpTray(mults=[0,1,2], mode="Min")
        tray.Execute(1)
        combiner = tray.context["Combiner"]
        self.assertAlmostEqual(1, combiner.GetMultiplicity())

        tray = SetUpTray(mults=[99,7,7], mode="LLH1")
        tray.Execute(1)
        combiner = tray.context["Combiner"]
        self.assertAlmostEqual(99, combiner.GetMultiplicity())

unittest.main()