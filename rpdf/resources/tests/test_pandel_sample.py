#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""Unit tests for `icecube.rpdf`

"""
import unittest

from scipy.stats import kstest, gamma
from icecube import phys_services, rpdf


class TestPandelSample(unittest.TestCase):
    """Test case for `icecube.rpdf.pandel_sample`

    This test case samples from `icecube.rpdf.pandel_sample` and checks
    it agains the CDF of the gamma distribution to ensure that it is
    sampling from the correct distribution.

    """
    def test_sample(self):
        """Test sample.

        """
        ice_model = rpdf.H2

        rng = phys_services.I3GSLRandomService(0)
        sample_size = 10000

        for distance in [1., 2., 5., 10., 20., 50., 1e2, 2e2, 5e2, 1e3, 2e3]:
            distribution = gamma(
                distance/ice_model.scattering_length,
                scale=1./ice_model.rho)

            sample = [
                rpdf.pandel_sample(distance, ice_model, rng)
                for x in range(sample_size)
                ]

            ks = kstest(sample, distribution.cdf)

            self.assertGreater(
                ks.pvalue, 0.05,
                "Test fails for a distance value of {:.0f}.".format(distance))


if __name__ == "__main__":
    unittest.main()
