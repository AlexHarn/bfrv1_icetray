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


class TestParticleDistributions(icetray.I3Module):
    betaRange = (0.001, 0.999)

    def __init__(self, context):
        icetray.I3Module.__init__(self, context)
        self.zeniths = []
        self.azimuths = []
        self.betas = []
        self.weights = []
        self.AddParameter("res", "Result list to be passed out")

    def Configure(self):
        self.res = self.GetParameter("res")

    def Physics(self, frame):
        pass

    def DAQ(self, frame):
        zenith = frame["MPInfoDict"]["Zenith"]
        azimuth = frame["MPInfoDict"]["Azimuth"]
        beta = frame["MPInfoDict"]["Beta"]
        w = frame["MPInfoDict"]["OneWeight"]
        self.weights.append(w)
        self.betas.append(beta)
        self.azimuths.append(azimuth)
        self.zeniths.append(zenith)

    def Finish(self):
        self.zeniths = numpy.cos(numpy.asarray(self.zeniths))
        self.azimuths = numpy.asarray(self.azimuths)
        self.betas = numpy.asarray(self.betas)
        self.weights = numpy.asarray(self.weights) * len(self.weights)

        hist_zenith,_ = numpy.histogram(self.zeniths, bins=10000, range=(-1, 1))
        hist_azimuth,_ = numpy.histogram(self.azimuths, bins=100000, range=(0, 2*math.pi))
        hist_beta,_ = numpy.histogram(self.betas, bins=10000, weights=self.weights, range=self.betaRange)
        _, p_zenith = scipy.stats.chisquare(hist_zenith)
        _, p_azimuth = scipy.stats.chisquare(hist_azimuth)
        _, p_beta = scipy.stats.chisquare(hist_beta)
        self.res.extend((p_zenith, p_azimuth, p_beta))


class TestMonopoleGeneratorMissingElementsInTray(unittest.TestCase):
    def test_missing_random_number_generator_shoud_raise(self):
        tray = I3Tray.I3Tray()
        tray.AddModule("I3InfiniteSource", "infinite", Stream=icetray.I3Frame.DAQ)
        tray.AddModule("I3MonopoleGenerator", "generator", Mass=1E12*I3Units.GeV, Nevents=1, BetaRange=[0.001])
        with self.assertRaisesRegexp(Exception, 'Failed to Get Random Service'):
            tray.Execute()


class TestMonopoleGeneratorInterface(unittest.TestCase):
    def test_setting_beta_negative_range(self):
        tray = I3Tray.I3Tray()
        tray.AddModule("I3InfiniteSource", "infinite", Stream=icetray.I3Frame.DAQ)
        tray.AddService("I3GSLRandomServiceFactory", "random")
        tray.AddModule("I3MonopoleGenerator", "generator", Mass=1E12*I3Units.GeV, Nevents=1, BetaRange=[0.7, 0.5])
        with self.assertRaisesRegexp(Exception, 'Negative beta range'):
            tray.Execute()

    def test_setting_beta_empty_range(self):
        tray = I3Tray.I3Tray()
        tray.AddModule("I3InfiniteSource", "infinite", Stream=icetray.I3Frame.DAQ)
        tray.AddService("I3GSLRandomServiceFactory", "random")
        tray.AddModule("I3MonopoleGenerator", "generator", Mass=1E12*I3Units.GeV, Nevents=1, BetaRange=[])
        with self.assertRaisesRegexp(Exception, 'Beta needs to be one or two non nan elements'):
            tray.Execute()

    def test_setting_beta_three_elements(self):
        tray = I3Tray.I3Tray()
        tray.AddModule("I3InfiniteSource", "infinite", Stream=icetray.I3Frame.DAQ)
        tray.AddService("I3GSLRandomServiceFactory", "random")
        tray.AddModule("I3MonopoleGenerator","generator", Mass=1E12*I3Units.GeV, Nevents=1, BetaRange=[1, 2, 3])
        with self.assertRaisesRegexp(Exception, 'Beta needs to be one or two non nan elements'):
            tray.Execute()

    def test_setting_beta_one_element(self):
        tray = I3Tray.I3Tray()
        tray.AddModule("I3InfiniteSource", "infinite", Stream=icetray.I3Frame.DAQ)
        tray.AddService("I3GSLRandomServiceFactory", "random")
        tray.AddModule("I3MonopoleGenerator", "generator", Mass=1E12*I3Units.GeV, Nevents=1, BetaRange=[0.5])
        tray.Execute()

    def test_setting_beta_two_element_different(self):
        tray = I3Tray.I3Tray()
        tray.AddModule("I3InfiniteSource", "infinite", Stream=icetray.I3Frame.DAQ)
        tray.AddService("I3GSLRandomServiceFactory", "random")
        tray.AddModule("I3MonopoleGenerator", "generator", Mass=1E12*I3Units.GeV, Nevents=1, BetaRange=[0.5, 0.6])
        tray.Execute()

    def test_setting_beta_two_element_same(self):
        tray = I3Tray.I3Tray()
        tray.AddModule("I3InfiniteSource", "infinite", Stream=icetray.I3Frame.DAQ)
        tray.AddService("I3GSLRandomServiceFactory", "random")
        tray.AddModule("I3MonopoleGenerator", "generator", Mass=1E12*I3Units.GeV, Nevents=1, BetaRange=[0.5, 0.5])
        tray.Execute()

    def test_setting_beta_range_one_element_too_big(self):
        tray = I3Tray.I3Tray()
        tray.AddModule("I3InfiniteSource", "infinite", Stream=icetray.I3Frame.DAQ)
        tray.AddService("I3GSLRandomServiceFactory", "random")
        tray.AddModule("I3MonopoleGenerator", "generator", Mass=1E12*I3Units.GeV, Nevents=1, BetaRange=[1.1])
        with self.assertRaisesRegexp(Exception, 'out of range.'):
            tray.Execute()

    def test_setting_beta_range_one_element_too_small(self):
        tray = I3Tray.I3Tray()
        tray.AddModule("I3InfiniteSource", "infinite", Stream=icetray.I3Frame.DAQ)
        tray.AddService("I3GSLRandomServiceFactory", "random")
        tray.AddModule("I3MonopoleGenerator", "generator", Mass=1E12*I3Units.GeV, Nevents=1, BetaRange=[0.0])
        with self.assertRaisesRegexp(Exception, 'out of range.'):
            tray.Execute()

    def test_setting_beta_range_two_elements_too_small(self):
        tray = I3Tray.I3Tray()
        tray.AddModule("I3InfiniteSource", "infinite", Stream=icetray.I3Frame.DAQ)
        tray.AddService("I3GSLRandomServiceFactory", "random")
        tray.AddModule("I3MonopoleGenerator", "generator", Mass=1E12*I3Units.GeV, Nevents=1, BetaRange=[0.0, 0.5])
        with self.assertRaisesRegexp(Exception, 'out of range.'):
            tray.Execute()

    def test_setting_beta_range_two_elements_too_big(self):
        tray = I3Tray.I3Tray()
        tray.AddModule("I3InfiniteSource", "infinite", Stream=icetray.I3Frame.DAQ)
        tray.AddService("I3GSLRandomServiceFactory", "random")
        tray.AddModule("I3MonopoleGenerator", "generator", Mass=1E12*I3Units.GeV, Nevents=1, BetaRange=[0.5, 1])
        with self.assertRaisesRegexp(Exception, 'out of range.'):
            tray.Execute()

    def test_setting_beta_with_powerlawindex(self):
        tray = I3Tray.I3Tray()
        tray.AddModule("I3InfiniteSource", "infinite", Stream=icetray.I3Frame.DAQ)
        tray.AddService("I3GSLRandomServiceFactory", "random")
        tray.AddModule("I3MonopoleGenerator", "generator", Mass=1E12*I3Units.GeV, Nevents=1, BetaRange=[0.5], powerLawIndex=1)
        with self.assertRaisesRegexp(Exception, 'No range for beta is defined, cannot apply a power law'):
            tray.Execute()

    def test_setting_beta_max_range(self):
        tray = I3Tray.I3Tray()
        tray.AddModule("I3InfiniteSource", "infinite", Stream=icetray.I3Frame.DAQ)
        tray.AddService("I3GSLRandomServiceFactory", "random")
        tray.AddModule("I3MonopoleGenerator", "generator", Mass=1E12*I3Units.GeV, Nevents=1, BetaRange=[0.0001, 0.9999])
        tray.Execute()

    def test_setting_azimuth_negativ_range(self):
        tray = I3Tray.I3Tray()
        tray.AddModule("I3InfiniteSource", "infinite", Stream=icetray.I3Frame.DAQ)
        tray.AddService("I3GSLRandomServiceFactory", "random")
        tray.AddModule("I3MonopoleGenerator", "generator", Mass=1E12*I3Units.GeV, Nevents=1, BetaRange=[0.5], AzimuthRange=[1, 0])
        with self.assertRaisesRegexp(Exception, "Can't have lower boundary > upper boundary."):
            tray.Execute()

    def test_setting_azimuth_empty_range(self):
        tray = I3Tray.I3Tray()
        tray.AddModule("I3InfiniteSource", "infinite", Stream=icetray.I3Frame.DAQ)
        tray.AddService("I3GSLRandomServiceFactory", "random")
        tray.AddModule("I3MonopoleGenerator", "generator", Mass=1E12*I3Units.GeV, Nevents=1, BetaRange=[0.5], AzimuthRange=[])
        with self.assertRaisesRegexp(Exception, "Please configure azimuthRange with a list containing \*only\* the minimum and the maximum"):
            tray.Execute()

    def test_setting_azimuth_three_elements(self):
        tray = I3Tray.I3Tray()
        tray.AddModule("I3InfiniteSource", "infinite", Stream=icetray.I3Frame.DAQ)
        tray.AddService("I3GSLRandomServiceFactory", "random")
        tray.AddModule("I3MonopoleGenerator", "generator", Mass=1E12*I3Units.GeV, Nevents=1, BetaRange=[0.5], AzimuthRange=[0, 1, 2])
        with self.assertRaisesRegexp(Exception, "Please configure azimuthRange with a list containing \*only\* the minimum and the maximum"):
            tray.Execute()

    def test_setting_azimuth_too_small(self):
        tray = I3Tray.I3Tray()
        tray.AddModule("I3InfiniteSource", "infinite", Stream=icetray.I3Frame.DAQ)
        tray.AddService("I3GSLRandomServiceFactory", "random")
        tray.AddModule("I3MonopoleGenerator", "generator", Mass=1E12*I3Units.GeV, Nevents=1, BetaRange=[0.5], AzimuthRange=[-0.1, 2])
        with self.assertRaisesRegexp(Exception, "out of range."):
            tray.Execute()

    def test_setting_azimuth_too_big(self):
        tray = I3Tray.I3Tray()
        tray.AddModule("I3InfiniteSource", "infinite", Stream=icetray.I3Frame.DAQ)
        tray.AddService("I3GSLRandomServiceFactory", "random")
        tray.AddModule("I3MonopoleGenerator", "generator", Mass=1E12*I3Units.GeV, Nevents=1, BetaRange=[0.5], AzimuthRange=[2, 7])
        with self.assertRaisesRegexp(Exception, "out of range."):
            tray.Execute()

    def test_setting_azimuth_max_range(self):
        tray = I3Tray.I3Tray()
        tray.AddModule("I3InfiniteSource", "infinite", Stream=icetray.I3Frame.DAQ)
        tray.AddService("I3GSLRandomServiceFactory", "random")
        tray.AddModule("I3MonopoleGenerator", "generator", Mass=1E12*I3Units.GeV, Nevents=1, BetaRange=[0.5], AzimuthRange=[0, 2*math.pi])
        tray.Execute()

    def test_setting_zenith_negativ_range(self):
        tray = I3Tray.I3Tray()
        tray.AddModule("I3InfiniteSource", "infinite", Stream=icetray.I3Frame.DAQ)
        tray.AddService("I3GSLRandomServiceFactory", "random")
        tray.AddModule("I3MonopoleGenerator", "generator", Mass=1E12*I3Units.GeV, Nevents=1, BetaRange=[0.5], ZenithRange=[1, 0])
        with self.assertRaisesRegexp(Exception, "Can't have lower boundary > upper boundary."):
            tray.Execute()

    def test_setting_zenith_empty_range(self):
        tray = I3Tray.I3Tray()
        tray.AddModule("I3InfiniteSource", "infinite", Stream=icetray.I3Frame.DAQ)
        tray.AddService("I3GSLRandomServiceFactory", "random")
        tray.AddModule("I3MonopoleGenerator", "generator", Mass=1E12*I3Units.GeV, Nevents=1, BetaRange=[0.5], ZenithRange=[])
        with self.assertRaisesRegexp(Exception, "Please configure zenithRange with a list containing \*only\* the minimum and the maximum"):
            tray.Execute()

    def test_setting_zenith_three_elements(self):
        tray = I3Tray.I3Tray()
        tray.AddModule("I3InfiniteSource", "infinite", Stream=icetray.I3Frame.DAQ)
        tray.AddService("I3GSLRandomServiceFactory", "random")
        tray.AddModule("I3MonopoleGenerator", "generator", Mass=1E12*I3Units.GeV, Nevents=1, BetaRange=[0.5], ZenithRange=[0, 1, 2])
        with self.assertRaisesRegexp(Exception, "Please configure zenithRange with a list containing \*only\* the minimum and the maximum"):
            tray.Execute()

    def test_setting_zenith_too_small(self):
        tray = I3Tray.I3Tray()
        tray.AddModule("I3InfiniteSource", "infinite", Stream=icetray.I3Frame.DAQ)
        tray.AddService("I3GSLRandomServiceFactory", "random")
        tray.AddModule("I3MonopoleGenerator", "generator", Mass=1E12*I3Units.GeV, Nevents=1, BetaRange=[0.5], ZenithRange=[-0.1, 2])
        with self.assertRaisesRegexp(Exception, "out of range."):
            tray.Execute()

    def test_setting_zenith_too_big(self):
        tray = I3Tray.I3Tray()
        tray.AddModule("I3InfiniteSource", "infinite", Stream=icetray.I3Frame.DAQ)
        tray.AddService("I3GSLRandomServiceFactory", "random")
        tray.AddModule("I3MonopoleGenerator", "generator", Mass=1E12*I3Units.GeV, Nevents=1, BetaRange=[0.5], ZenithRange=[2, 4])
        with self.assertRaisesRegexp(Exception, "out of range."):
            tray.Execute()

    def test_setting_zenith_max_range(self):
        tray = I3Tray.I3Tray()
        tray.AddModule("I3InfiniteSource", "infinite", Stream=icetray.I3Frame.DAQ)
        tray.AddService("I3GSLRandomServiceFactory", "random")
        tray.AddModule("I3MonopoleGenerator", "generator", Mass=1E12*I3Units.GeV, Nevents=1, BetaRange=[0.5], ZenithRange=[0, math.pi])
        tray.Execute()

    def test_setting_mass_too_small(self):
        tray = I3Tray.I3Tray()
        tray.AddModule("I3InfiniteSource", "infinite", Stream=icetray.I3Frame.DAQ)
        tray.AddService("I3GSLRandomServiceFactory", "random")
        tray.AddModule("I3MonopoleGenerator", "generator", Nevents=1, BetaRange=[0.5], Mass=0.9e5 * I3Units.GeV)
        with self.assertRaisesRegexp(Exception, "out of range."):
            tray.Execute()

    def test_setting_mass_too_big(self):
        tray = I3Tray.I3Tray()
        tray.AddModule("I3InfiniteSource", "infinite", Stream=icetray.I3Frame.DAQ)
        tray.AddService("I3GSLRandomServiceFactory", "random")
        tray.AddModule("I3MonopoleGenerator", "generator", Nevents=1, BetaRange=[0.5], Mass=2e17 * I3Units.GeV)
        with self.assertRaisesRegexp(Exception, "out of range."):
            tray.Execute()

    def test_setting_negativ_length(self):
        tray = I3Tray.I3Tray()
        tray.AddModule("I3InfiniteSource", "infinite", Stream=icetray.I3Frame.DAQ)
        tray.AddService("I3GSLRandomServiceFactory", "random")
        tray.AddModule("I3MonopoleGenerator", "generator", Mass=1E12*I3Units.GeV, Nevents=1, BetaRange=[0.5], Length=-10)
        with self.assertRaisesRegexp(Exception, "Can't have a negative length besides -1 to indicate default value."):
            tray.Execute()

    def test_setting_disk_dist_0(self):
        tray = I3Tray.I3Tray()
        tray.AddModule("I3InfiniteSource", "infinite", Stream=icetray.I3Frame.DAQ)
        tray.AddService("I3GSLRandomServiceFactory", "random")
        tray.AddModule("I3MonopoleGenerator", "generator", Mass=1E12*I3Units.GeV, Nevents=1, BetaRange=[0.5], Disk_dist=0)
        tray.Execute()

    def test_setting_disk_dist_negativ(self):
        tray = I3Tray.I3Tray()
        tray.AddModule("I3InfiniteSource", "infinite", Stream=icetray.I3Frame.DAQ)
        tray.AddService("I3GSLRandomServiceFactory", "random")
        tray.AddModule("I3MonopoleGenerator", "generator", Mass=1E12*I3Units.GeV, Nevents=1, BetaRange=[0.5], Disk_dist=-1)
        with self.assertRaisesRegexp(Exception, "out of range."):
            tray.Execute()

    def test_setting_disk_dist_too_big(self):
        tray = I3Tray.I3Tray()
        tray.AddModule("I3InfiniteSource", "infinite", Stream=icetray.I3Frame.DAQ)
        tray.AddService("I3GSLRandomServiceFactory", "random")
        tray.AddModule("I3MonopoleGenerator", "generator", Mass=1E12*I3Units.GeV, Nevents=1, BetaRange=[0.5], Disk_dist=6 * I3Units.km)
        with self.assertRaisesRegexp(Exception, "out of range."):
            tray.Execute()

    def test_setting_disk_rad_0(self):
        tray = I3Tray.I3Tray()
        tray.AddModule("I3InfiniteSource", "infinite", Stream=icetray.I3Frame.DAQ)
        tray.AddService("I3GSLRandomServiceFactory", "random")
        tray.AddModule("I3MonopoleGenerator", "generator", Mass=1E12*I3Units.GeV, Nevents=1, BetaRange=[0.5], Disk_rad=0)
        tray.Execute()

    def test_setting_disk_dist_rad(self):
        tray = I3Tray.I3Tray()
        tray.AddModule("I3InfiniteSource", "infinite", Stream=icetray.I3Frame.DAQ)
        tray.AddService("I3GSLRandomServiceFactory", "random")
        tray.AddModule("I3MonopoleGenerator", "generator", Mass=1E12*I3Units.GeV, Nevents=1, BetaRange=[0.5], Disk_rad=-1)
        with self.assertRaisesRegexp(Exception, "out of range."):
            tray.Execute()

    def test_setting_NEvent_negativ(self):
        tray = I3Tray.I3Tray()
        tray.AddModule("I3InfiniteSource", "infinite", Stream=icetray.I3Frame.DAQ)
        tray.AddService("I3GSLRandomServiceFactory", "random")
        tray.AddModule("I3MonopoleGenerator", "generator", Mass=1E12*I3Units.GeV, Nevents=-1, BetaRange=[0.5])
        with self.assertRaisesRegexp(Exception, "Requested less than 1 event to be generated. Abort!"):
            tray.Execute()

    def test_setting_NEvent_0(self):
        tray = I3Tray.I3Tray()
        tray.AddModule("I3InfiniteSource", "infinite", Stream=icetray.I3Frame.DAQ)
        tray.AddService("I3GSLRandomServiceFactory", "random")
        tray.AddModule("I3MonopoleGenerator", "generator", Mass=1E12*I3Units.GeV, Nevents=0, BetaRange=[0.5])
        with self.assertRaisesRegexp(Exception, "Requested less than 1 event to be generated. Abort!"):
            tray.Execute()


class TestMonopoleGeneratorWeighting(unittest.TestCase):
    def test_weighting_single_value(self):
        mutable_integrated_total_weight = [0]
        mutable_number_events = [0]

        def frame_tester(fr, total_weight=mutable_integrated_total_weight, number_events=mutable_number_events):
            number_events[0] = number_events[0] + 1
            total_weight[0] = total_weight[0] + fr["MPInfoDict"]["OneWeight"]
        N = 10**5
        tray = I3Tray.I3Tray()
        tray.AddModule("I3InfiniteSource", "infinite", Stream=icetray.I3Frame.DAQ)
        tray.AddService("I3GSLRandomServiceFactory", "random")
        tray.AddModule("I3MonopoleGenerator", "generator", Mass=1E12*I3Units.GeV, Nevents=N, BetaRange=[0.5])
        tray.Add(frame_tester, Streams=[icetray.I3Frame.DAQ])
        tray.Execute()
        self.assertEqual(mutable_number_events[0], N)
        self.assertEqual(round(mutable_integrated_total_weight[0], 11), 1)

    def test_weighting_flat(self):
        mutable_integrated_total_weight = [0]
        mutable_number_events = [0]

        def frame_tester(fr, total_weight=mutable_integrated_total_weight, number_events=mutable_number_events):
            number_events[0] = number_events[0] + 1
            total_weight[0] = total_weight[0] + fr["MPInfoDict"]["OneWeight"]
        N = 10**5
        tray = I3Tray.I3Tray()
        tray.AddModule("I3InfiniteSource", "infinite", Stream=icetray.I3Frame.DAQ)
        tray.AddService("I3GSLRandomServiceFactory", "random")
        tray.AddModule("I3MonopoleGenerator", "generator", Mass=1E12*I3Units.GeV, Nevents=N, BetaRange=[0.5, 0.6])
        tray.Add(frame_tester, Streams=[icetray.I3Frame.DAQ])
        tray.Execute()
        self.assertEqual(mutable_number_events[0], N)
        self.assertEqual(round(mutable_integrated_total_weight[0], 11), 1)

    def test_weighting_powerlaw_1(self):
        mutable_integrated_total_weight = [0]
        mutable_number_events = [0]

        def frame_tester(fr, total_weight=mutable_integrated_total_weight, number_events=mutable_number_events):
            number_events[0] = number_events[0] + 1
            total_weight[0] = total_weight[0] + fr["MPInfoDict"]["OneWeight"]
        N = 10**5
        tray = I3Tray.I3Tray()
        tray.AddModule("I3InfiniteSource", "infinite", Stream=icetray.I3Frame.DAQ)
        tray.AddService("I3GSLRandomServiceFactory", "random")
        tray.AddModule("I3MonopoleGenerator", "generator", Mass=1E12*I3Units.GeV, Nevents=N, BetaRange=[0.5, 0.6], powerLawIndex=1)
        tray.AddModule(frame_tester, Streams=[icetray.I3Frame.DAQ])
        tray.Execute()
        self.assertEqual(mutable_number_events[0], N)
        self.assertEqual(round(mutable_integrated_total_weight[0], 11), 1)

    def test_weighting_powerlaw_5(self):
        mutable_integrated_total_weight = [0]
        mutable_number_events = [0]

        def frame_tester(fr, total_weight=mutable_integrated_total_weight, number_events=mutable_number_events):
            number_events[0] = number_events[0] + 1
            total_weight[0] = total_weight[0] + fr["MPInfoDict"]["OneWeight"]
        N = 10**5
        tray = I3Tray.I3Tray()
        tray.AddModule("I3InfiniteSource", "infinite", Stream=icetray.I3Frame.DAQ)
        tray.AddService("I3GSLRandomServiceFactory", "random")
        tray.AddModule("I3MonopoleGenerator", "generator", Mass=1E12*I3Units.GeV, Nevents=N, BetaRange=[0.5, 0.6], powerLawIndex=5)
        tray.AddModule(frame_tester, Streams=[icetray.I3Frame.DAQ])
        tray.Execute()
        self.assertEqual(mutable_number_events[0], N)
        self.assertEqual(round(mutable_integrated_total_weight[0], 11), 1)


class TestMonopoleGeneratorStatisticalTests(unittest.TestCase):
    def test_check_particle_distributions(self):
        res = []
        tray = I3Tray.I3Tray()
        tray.AddModule("I3InfiniteSource", "infinite", Stream=icetray.I3Frame.DAQ)
        tray.AddService("I3GSLRandomServiceFactory", "random")
        tray.AddModule("I3MonopoleGenerator", "generator", Mass=1E12*I3Units.GeV, Nevents=10**6, BetaRange=TestParticleDistributions.betaRange)#, AzimuthRange=[0,1])#, ZenithRange=TestParticleDistributions.zenithRange)
        tray.AddModule(TestParticleDistributions, res=res)
        tray.Execute()
        self.assertTrue(0.9 <= res[0] <= 1.1, "Pvalue failed for Zenith")
        self.assertTrue(0.9 <= res[1] <= 1.1, "Pvalue failed for Azimuth")
        self.assertTrue(0.9 <= res[2] <= 1.1, "Pvalue failed for Beta")


if __name__ == '__main__':
    unittest.main()
