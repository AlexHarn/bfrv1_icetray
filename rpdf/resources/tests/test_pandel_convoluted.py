#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""Unit test for `icecubue.rpdf`

"""
import unittest

import numpy as np
from scipy.integrate import quad
from scipy.stats import norm
from icecube import rpdf


class TestPandelConvoluted(unittest.TestCase):
    """Test case for `icecube.rpdf.FastConvolutedPandel`

    This test checks that the Gauss-convoluted Pandel is as advertized:
    it does the convolution by taking `icecube.rpdf.pandel_pdf` and
    convolving it with a Normal distribution, and compares it with
    what `icecube.rpdf` calculates.

    """
    def test_density(self):
        """Test probability densities.

        """
        ice_model = rpdf.H2
        jitter = 15.

        pandel = rpdf.FastConvolutedPandel(jitter, ice_model)

        for distance in [20., 50., 100., 500., 1000.]:
            times = np.linspace(-jitter, 20.*distance, 100)

            for time in times:
                p1 = pandel.pdf(time, distance)
                p2 = self.pandel(time, distance, jitter, ice_model)

                self.assertAlmostEqual(p1, p2, 4)

                if p1 > 1e-6 and p2 > 0.:
                    self.assertAlmostEqual(1., p1/p2, 2)

    @staticmethod
    def pandel(time, distance, jitter, ice_model):
        """Convoluted Pandel function

        Use Gaussian quadrature integration to convolve the Pandel
        function with a Normal distribution.

        """
        # Convolve Normal distribution and Pandel.
        def convolution(x):
            return (
                norm.pdf(time, loc=x, scale=jitter) *
                rpdf.pandel_pdf(x, distance, ice_model)
                )

        return quad(convolution, -5.*jitter, time + 5.*jitter)[0]


if __name__ == "__main__":
    unittest.main()
