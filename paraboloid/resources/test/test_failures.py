#!/usr/bin/env python

import unittest

from icecube import icetray, dataclasses, dataio, lilliput, paraboloid
from I3Tray import I3Tray


class TestStringMethods(unittest.TestCase):

    def test_no_minimizer(self):
        tray = I3Tray()

        tray.Add("I3GulliverMinuitFactory", "Minuit")
        tray.Add("I3BasicSeedServiceFactory", "ParaboloidSeed")
        tray.Add("I3GulliverIPDFPandelFactory", "MPEParaboloidPandel")

        tray.Add("BottomlessSource")

        tray.Add("I3ParaboloidFitter",
                 OutputName="MPEFitParaboloid2",
                 Minimizer="NotMinuit",
                 LogLikelihood="MPEParaboloidPandel",
                 SeedService="ParaboloidSeed",
                 GridpointVertexCorrection="ParaboloidSeed")

        tray.Add("I3EventCounter", NEvents=1)

        self.assertRaises(RuntimeError, tray.Execute)

    def test_no_liklihood(self):
        tray = I3Tray()

        tray.Add("I3GulliverMinuitFactory", "Minuit")
        tray.Add("I3BasicSeedServiceFactory", "ParaboloidSeed")
        tray.Add("I3GulliverIPDFPandelFactory", "MPEParaboloidPandel")

        tray.Add("BottomlessSource")

        tray.Add("I3ParaboloidFitter",
                 OutputName="MPEFitParaboloid2",
                 Minimizer="Minuit",
                 LogLikelihood="fasl;dkfjasdkl",
                 SeedService="ParaboloidSeed",
                 GridpointVertexCorrection="ParaboloidSeed")

        tray.Add("I3EventCounter", NEvents=1)

        self.assertRaises(RuntimeError, tray.Execute)

    def test_no_seed(self):
        tray = I3Tray()

        tray.Add("I3GulliverMinuitFactory", "Minuit")
        tray.Add("I3BasicSeedServiceFactory", "ParaboloidSeed")
        tray.Add("I3GulliverIPDFPandelFactory", "MPEParaboloidPandel")

        tray.Add("BottomlessSource")

        tray.Add("I3ParaboloidFitter",
                 OutputName="MPEFitParaboloid2",
                 Minimizer="Minuit",
                 LogLikelihood="MPEParaboloidPandel",
                 SeedService="NoSeed",
                 GridpointVertexCorrection="ParaboloidSeed")

        tray.Add("I3EventCounter", NEvents=1)

        self.assertRaises(RuntimeError, tray.Execute)

    def test_no_vertex(self):
        tray = I3Tray()

        tray.Add("I3GulliverMinuitFactory", "Minuit")
        tray.Add("I3BasicSeedServiceFactory", "ParaboloidSeed")
        tray.Add("I3GulliverIPDFPandelFactory", "MPEParaboloidPandel")

        tray.Add("BottomlessSource")

        tray.Add("I3ParaboloidFitter",
                 OutputName="MPEFitParaboloid2",
                 Minimizer="Minuit",
                 LogLikelihood="MPEParaboloidPandel",
                 SeedService="ParaboloidSeed",
                 GridpointVertexCorrection="NoSeed")

        tray.Add("I3EventCounter", NEvents=1)

        self.assertRaises(RuntimeError, tray.Execute)

    def test_zenith_reach_fail(self):
        tray = I3Tray()

        tray.Add("I3GulliverMinuitFactory", "Minuit")
        tray.Add("I3BasicSeedServiceFactory", "ParaboloidSeed")
        tray.Add("I3GulliverIPDFPandelFactory", "MPEParaboloidPandel")

        tray.Add("BottomlessSource")

        tray.Add("I3ParaboloidFitter",
                 OutputName="MPEFitParaboloid2",
                 Minimizer="Minuit",
                 LogLikelihood="MPEParaboloidPandel",
                 SeedService="ParaboloidSeed",
                 GridpointVertexCorrection="ParaboloidSeed",
                 ZenithReach=-1000)

        tray.Add("I3EventCounter", NEvents=1)

        self.assertRaises(RuntimeError, tray.Execute)

    def test_sample_points_fail(self):
        tray = I3Tray()

        tray.Add("I3GulliverMinuitFactory", "Minuit")
        tray.Add("I3BasicSeedServiceFactory", "ParaboloidSeed")
        tray.Add("I3GulliverIPDFPandelFactory", "MPEParaboloidPandel")

        tray.Add("BottomlessSource")

        tray.Add("I3ParaboloidFitter",
                 OutputName="MPEFitParaboloid2",
                 Minimizer="Minuit",
                 LogLikelihood="MPEParaboloidPandel",
                 SeedService="ParaboloidSeed",
                 GridpointVertexCorrection="ParaboloidSeed",
                 NumberOfSamplingPoints=7)

        tray.Add("I3EventCounter", NEvents=1)

        self.assertRaises(RuntimeError, tray.Execute)

    def test_steps_fail(self):
        tray = I3Tray()

        tray.Add("I3GulliverMinuitFactory", "Minuit")
        tray.Add("I3BasicSeedServiceFactory", "ParaboloidSeed")
        tray.Add("I3GulliverIPDFPandelFactory", "MPEParaboloidPandel")

        tray.Add("BottomlessSource")

        tray.Add("I3ParaboloidFitter",
                 OutputName="MPEFitParaboloid2",
                 Minimizer="Minuit",
                 LogLikelihood="MPEParaboloidPandel",
                 SeedService="ParaboloidSeed",
                 GridpointVertexCorrection="ParaboloidSeed",
                 NumberOfSteps=71)

        tray.Add("I3EventCounter", NEvents=1)

        self.assertRaises(RuntimeError, tray.Execute)

    def test_vertex_step_fail(self):
        tray = I3Tray()

        tray.Add("I3BasicSeedServiceFactory", "ParaboloidSeed")
        tray.Add("I3GulliverIPDFPandelFactory", "MPEParaboloidPandel")

        tray.Add("BottomlessSource")

        tray.Add("I3ParaboloidFitter",
                 OutputName="MPEFitParaboloid2",
                 Minimizer="",
                 LogLikelihood="MPEParaboloidPandel",
                 SeedService="ParaboloidSeed",
                 GridpointVertexCorrection="ParaboloidSeed",
                 VertexStepSize=10.0)

        tray.Add("I3EventCounter", NEvents=1)

        self.assertRaises(RuntimeError, tray.Execute)


if __name__ == "__main__":
    unittest.main()
