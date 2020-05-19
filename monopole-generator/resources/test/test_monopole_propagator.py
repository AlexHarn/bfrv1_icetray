#!/usr/bin/env python
import unittest
from icecube import icetray, dataclasses, dataio
from icecube import monopole_generator
from icecube import icetray
from icecube.icetray import I3Units
import I3Tray
import math
import numpy
import scipy.stats


class TestMonopoleGeneratorKnownBugTests(unittest.TestCase):
    def test_max_length_of_particles_is_correct(self):
        tray = I3Tray.I3Tray()

        def frame_tester(fr, unit_tester=self):
            for particle in fr["I3MCTree"]:
                if particle.type == "monopole":
                    unit_tester.assertEqual(particle.length, 10 * I3Units.m)

        tray.AddModule("I3InfiniteSource", "infinite", Stream=icetray.I3Frame.DAQ)
        tray.AddService("I3GSLRandomServiceFactory", "random")
        tray.AddModule("I3MonopoleGenerator", "generator", Mass=1e12*I3Units.GeV, Nevents=1, BetaRange=[0.5])
        tray.AddModule("I3MonopolePropagator",
                       MaxLength             = 10 * I3Units.m,
                       MinLength             = 10 * I3Units.m,
                       StepSize              = float("NAN"),
                       )
        tray.AddModule(frame_tester, Streams=[icetray.I3Frame.DAQ])
        tray.Execute()


class TestMonopolePropagator_working_usecases(unittest.TestCase):
    def test_max_length_of_particles_is_correct(self):
        tray = I3Tray.I3Tray()
        tray.AddModule("I3InfiniteSource", "infinite", Stream=icetray.I3Frame.DAQ)
        tray.AddService("I3GSLRandomServiceFactory", "random")
        tray.AddModule("I3MonopoleGenerator", "generator", Mass=1e11*I3Units.GeV, Nevents=1, BetaRange=[0.5], Disk_dist=1000 * I3Units.m, Disk_rad = 0)
        tray.AddModule("I3MonopolePropagator",
                       MaxLength             = 10 * I3Units.m,
                       MinLength             = 1 * I3Units.m,
                       StepSize              = float("NAN"),
                       )
        tray.Execute()

    def test_check_that_Stepsize_gets_properly_overwritten(self):
        tray = I3Tray.I3Tray()
        def frame_tester(fr, unit_tester=self):
            for particle in fr["I3MCTree"]:
                if particle.type == "monopole":
                    unit_tester.assertEqual(particle.length, 10 * I3Units.m)

        tray.AddModule("I3InfiniteSource", "infinite", Stream=icetray.I3Frame.DAQ)
        tray.AddService("I3GSLRandomServiceFactory", "random")
        tray.AddModule("I3MonopoleGenerator", "generator", Mass=1e12*I3Units.GeV, Nevents=1, BetaRange=[0.5])
        tray.AddModule("I3MonopolePropagator",
                       MaxLength             = 10 * I3Units.m,
                       MinLength             = 10 * I3Units.m,
                       StepSize              = 100 * I3Units.m,
                       )
        tray.AddModule(frame_tester, Streams=[icetray.I3Frame.DAQ])
        tray.Execute()


class TestMonopoleLogFatalShouldWork(unittest.TestCase):
    def test_max_length_of_particles_is_correct(self):
        tray = I3Tray.I3Tray()
        tray.AddModule("I3InfiniteSource", "infinite", Stream=icetray.I3Frame.DAQ)
        tray.AddService("I3GSLRandomServiceFactory", "random")
        tray.AddModule("I3MonopoleGenerator", "generator", Mass=1e11*I3Units.GeV, Nevents=1, BetaRange=[0.5], Disk_dist=1000 * I3Units.m, Disk_rad = 0)
        tray.AddModule("I3MonopolePropagator",
                           MaxLength             = 1 * I3Units.m,
                           MinLength             = 10 * I3Units.m,
                           StepSize              = float("NAN"),
                           )
        with self.assertRaisesRegexp(Exception, "Oops, MaxLength<MinLength."):
            tray.Execute()

#TODO
"""
* This test is designed to comprehensively check the interface of the Propagate module through various ways
* (1) That it only propagates Monopoles (done by adding non-monopole primary/secondaries to test generation tree)
* (2) That the monopoles all lose energy as they propagate
* (3) That the number of children remains as expected - constant if non-monopole, and the correct number of
*     segments if they are monopoles (the num_child tests)
* (4) That setting the monopole to beyond the max length will cause it to propagate beyond the detector the same amount
*     as it started (tree1 above)
* (5) That, otherwise, it will propagate to the set max distance from the detector center (tree3 above)
* (6) That it stops propagating when it falls below user specified minimum speed (tree2 above)
* (7) That direction is preserved
* (8) That a monopole with no length gets the starting particle set to user defined min length (tree2, 3)
* (9) That when CalculateNextLength gives something larger than user-defined max length,
*     length is set to this instead (tree 1)
*/
"""


if __name__ == '__main__':
    unittest.main()
