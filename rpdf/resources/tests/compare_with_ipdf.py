#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""Unit test for `icecube.rpdf`

"""
import os
import unittest

import icecube.icetray
import icecube.dataio
import icecube.rpdf
import icecube.gulliver_modules
import icecube.lilliput.segments as ls
from I3Tray import I3Tray, I3Units


class TestRecoLLHFactory(unittest.TestCase):
    """Test case for `I3RecoLLHFactory`

    Test reconstructed zenith and azimuth angles for equality,
    using `icecube.idf` and `icecube.rpdf`, respectively.

    """
    def test_zenith_angle(self):
        """Test reconstructed zenith angle.

        """
        p1 = self.frame["SPEFitSingle_HV_ipdf"]
        p2 = self.frame["SPEFitSingle_HV_rpdf"]
        self.assertLess(abs(p1.dir.zenith - p2.dir.zenith), 1e-6)

    def test_azimuth_angle(self):
        """Test reconstructed azimuth angle.

        """
        p1 = self.frame["SPEFitSingle_HV_ipdf"]
        p2 = self.frame["SPEFitSingle_HV_rpdf"]
        self.assertLess(abs(p1.dir.azimuth - p2.dir.azimuth), 1e-6)


def main(inputfile, gcdfile):
    tray = I3Tray()

    tray.Add(
        "I3Reader",
        filenamelist=[gcdfile, inputfile])

    pulses = "SRTHVInIcePulses"
    jitter = 15.*I3Units.ns
    noise = 10.*I3Units.hertz

    tray.AddService(
        "I3RecoLLHFactory", "recollh",
        inputreadout=pulses,
        likelihood="SPE1st",
        jittertime=jitter,
        peprob="GaussConvoluted",
        noiseprobability=noise)

    tray.AddService(
        "I3GulliverIPDFPandelFactory", "pandel",
        InputReadout=pulses,
        EventType="InfiniteMuon",
        Likelihood="SPE1st",
        PEProb="GaussConvolutedFastApproximation",
        JitterTime=jitter,
        NoiseProbability=noise)

    minimizer_service = ls.add_minuit_simplex_minimizer_service(tray)
    param_service = ls.add_simple_track_parametrization_service(tray)

    seed_service = ls.add_seed_service(
        tray,
        pulses=pulses,
        seeds=["LineFit_HV"])

    tray.Add(
        "I3SimpleFitter",
        outputname="SPEFitSingle_HV_ipdf",
        seedservice=seed_service,
        parametrization=param_service,
        loglikelihood="pandel",
        minimizer=minimizer_service)

    tray.Add(
        "I3SimpleFitter",
        outputname="SPEFitSingle_HV_rpdf",
        seedservice=seed_service,
        parametrization=param_service,
        loglikelihood="recollh",
        minimizer=minimizer_service)

    tray.Add(icecube.icetray.I3TestModuleFactory(TestRecoLLHFactory))

    tray.Execute()


if __name__ == "__main__":
    import argparse
    parser = argparse.ArgumentParser(description=__doc__)

    inputfiles = [
        "sim/Level3_nugen_numu_IC86.2012.011069.000000_20events.i3.bz2",
        "GCD/GeoCalibDetectorStatus_2012.56063_V0.i3.gz"
        ]

    inputfiles = [
        os.path.join(os.getenv("I3_TESTDATA"), f) for f in inputfiles
        ]

    parser.add_argument(
        "--in",
        nargs="?",
        type=str,
        help="path to input i3 file",
        default=inputfiles[0],
        metavar="PATH",
        dest="inputfile")

    parser.add_argument(
        "--gcd",
        nargs="?",
        type=str,
        help="path to geometry i3 file",
        default=inputfiles[1],
        metavar="PATH",
        dest="gcdfile")

    args = parser.parse_args()

    main(args.inputfile, args.gcdfile)
