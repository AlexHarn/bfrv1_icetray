#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""Unit tests for `icecube.rpdf`

"""
from math import log, exp, sqrt
import unittest

import numpy as np
from scipy import special, stats
from icecube import rpdf


class TestUnconvolutedPandel(unittest.TestCase):
    """Test case for `icecube.rpdf.UnconvolutedPandel`

    """
    def test_density(self):
        """Test probability densities.

        """
        ice_model = rpdf.H2
        pandel = rpdf.UnconvolutedPandel(ice_model)

        distances = [
            -1., 0., 1., 2., 5., 10., 20., 50., 1e2, 2e2, 5e2, 1e3, 2e3
            ]

        for distance in distances:
            times = np.linspace(-2.*distance, 20.*distance, 200)

            for time in times:
                p1 = pandel.pdf(time, distance)
                p2 = self.pandel(time, distance, ice_model)

                self.assertAlmostEqual(p1, p2, 15)

                if p1 > 0. and p2 > 0.:
                    self.assertAlmostEqual(1., p1/p2, 12)

    @staticmethod
    def pandel(time, distance, ice_model):
        """Pandel function

        This Implementation of the Pandel Function uses SciPy's gamma
        distribution.

        """
        if time <= 0.:
            return 0.

        if distance <= 0.:
            return 0.

        return stats.gamma.pdf(
            time, distance/ice_model.scattering_length,
            scale=1./ice_model.rho)


class TestConvolutedPandel(unittest.TestCase):
    """Test case `icecube.rpdf.FastConvolutedPandel`

    """
    def test_density(self):
        """Test probability densities.

        """
        ice_model = rpdf.H2

        distances = [
            -1., 0., 1., 2., 5., 10., 20., 50., 1e2, 2e2, 5e2, 1e3, 2e3
            ]

        for jitter in [-5., 0., 5., 10., 15., 20.]:
            pandel = rpdf.FastConvolutedPandel(jitter, ice_model)

            for distance in distances:
                times = np.linspace(
                    min(-5., -5.*jitter), max(5.*jitter, 20.*distance), 200)

                for time in times:
                    p1 = pandel.pdf(time, distance)
                    p2 = self.pandel(time, distance, jitter, ice_model)

                    self.assertAlmostEqual(p1, p2, 5)

                    if p1 >= 1e-11 and p2 > 0.:
                        self.assertAlmostEqual(1., p1/p2, 1)

    @staticmethod
    def pandel(time, distance, sigma, ice_model):
        """Convoluted Pandel function

        This is a really simplified version of the convoluted pandel
        function it has two regions and is not optimized for speed but
        should be accurate.

        Region 1: use the complete hypergeometrice 1F1 expansion.
        Region 2: in regions were 1F1 overflows use an approximation.

        """
        if sigma <= 0.:
            return TestUnconvolutedPandel.pandel(time, distance, ice_model)

        if distance < 0.:
            return stats.norm.pdf(time, scale=sigma)

        ksi = distance/ice_model.scattering_length
        eta = ice_model.rho*sigma - time/sigma

        M_LN2 = log(2.)
        M_SQRT2 = sqrt(2.)

        if abs(eta) < 34.64:
            xi1 = 0.5*(ksi + 1.)
            xi2 = 0.5*ksi

            eta2 = 0.5*eta**2

            f1 = special.hyp1f1(xi2, 0.5, eta2)
            f2 = special.hyp1f1(xi1, 1.5, eta2)

            return (
                (
                    exp(
                        -0.5*time**2/sigma**2 +
                        ksi*(log(ice_model.rho*sigma) - 0.5*M_LN2)
                        ) /
                    (sigma*M_SQRT2)
                    ) *
                (f1/special.gamma(xi1) - eta*M_SQRT2*f2/special.gamma(xi2))
                )
        else:
            return (
                exp(
                    ksi*log(ice_model.rho*time) - ice_model.rho*time +
                    0.5*ice_model.rho**2*sigma**2
                    ) /
                (time*special.gamma(ksi))
                )


if __name__ == "__main__":
    unittest.main()
