#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""Unit tests for `icecube.rpf`

"""
import unittest

import numpy as np
from scipy.integrate import quad
from scipy.stats import gamma
from icecube import rpdf


class TestPandelIntegral(unittest.TestCase):
    """Pandel integral test

    This test checks if the Pandel survival values are the actual
    integral of the Pandel PDF. It samples values from the Pandel
    survival function and compares it with the integral of the PDF using
    Gaussian quadrature.

    """
    def test_pandel_integral(self):
        """Test for several distances and jitters.

        """
        ice_model = rpdf.H2
        for distance in np.logspace(0., 3.3, 20):
            tmin = gamma.isf(
                0.001, distance/ice_model.scattering_length,
                scale=1./ice_model.rho)

            tmax = gamma.isf(
                0.01, distance/ice_model.scattering_length,
                scale=1./ice_model.rho)

            pandel = rpdf.UnconvolutedPandel(ice_model)
            self.check_integral(pandel, distance, tmin, tmax=(0., tmax))

            for jitter in range(0, 25, 5):
                pandel = rpdf.FastConvolutedPandel(jitter, ice_model)

                self.check_integral(
                    pandel, distance, tmin, tmax=(-4.*jitter, tmax))

    def check_integral(self, pandel, distance, tmin, tmax):
        times = np.linspace(tmax[0], tmax[1], 50)

        q0 = np.array([pandel.sf(time, distance) for time in times])
        self.assertAlmostEqual(q0[0], 1., 7)
        self.assertAlmostEqual(q0[-1], 0., 1)

        # The integration is more precice for large distances, so be
        # more strict there.
        precision = min(int(np.log10(distance)) + 1, 3)

        for i, time in enumerate(times):
            q1 = -quad(pandel.pdf, 20.*tmin, time, args=(distance,))[0]
            self.assertAlmostEqual(q0[i], q1, precision)


if __name__ == "__main__":
    unittest.main()
