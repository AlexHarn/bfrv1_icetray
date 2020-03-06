#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""Unit tests for `icecube.rpdf`

"""
import unittest
from math import log, sqrt

from icecube import rpdf


class TestPandelRegions(unittest.TestCase):
    """Test case for `icecube.rpdf.FastConvolutedPandel`

    This tests the Pandel function at the boundaries of the different
    regions it uses for calculations. The calculation is performed
    differently depending on where in the ``(t_res, d_eff)`` plane.
    There are 9 triple points and 3 corner points. Each one of these is
    tested by evaluating the Pandel function close to the point in each
    region and testing to make sure that the values they return are are
    close.

    """
    def setUp(self):
        self.epsilon = 0.00000001
        self.sigma = 15.
        self.ice = rpdf.H2
        self.fcp = rpdf.FastConvolutedPandel(self.sigma, self.ice)

        self.tAC = -self.sigma*(1.35 - self.ice.rho*self.sigma)
        self.tBC = self.sigma*(self.ice.rho*self.sigma+sqrt(200.))
        self.t12 = 30.*self.sigma
        self.t15 = -5.*self.sigma
        self.t34 = rpdf.H2.rho * self.sigma**2

        self.dBC = 0.05*self.ice.scattering_length
        self.d13 = 5.*self.ice.scattering_length
        self.d23 = self.ice.scattering_length

    def test_ABC(self):
        regionA = log(self.fcp.pdf(self.tAC*(1. + self.epsilon), self.dBC))

        regionB = log(self.fcp.pdf(
            self.tAC*(1. - self.epsilon), self.dBC*(1. + self.epsilon)))

        regionC = log(self.fcp.pdf(
            self.tAC*(1. - self.epsilon), self.dBC*(1. - self.epsilon)))

        self.assertAlmostEqual(regionA, regionB, 7)
        self.assertAlmostEqual(regionB, regionC, 8)
        self.assertAlmostEqual(regionC, regionA, 7)

    def test_BC(self):
        regionB = log(self.fcp.pdf(
            self.tBC*(1 - self.epsilon), self.dBC*(1 + self.epsilon)))

        regionC = log(self.fcp.pdf(
            self.tBC*(1 + self.epsilon), self.dBC*(1 - self.epsilon)))

        self.assertAlmostEqual(regionB, regionC, 6)

    def test_BC3(self):
        regionB = log(self.fcp.pdf(
            self.tBC*(1. - self.epsilon), self.d13*(1. - self.epsilon)))

        regionC = log(self.fcp.pdf(
            self.tBC*(1. + self.epsilon), self.d13*(1. - self.epsilon)))

        region3 = log(self.fcp.pdf(self.tBC, self.d13*(1. + self.epsilon)))

        self.assertAlmostEqual(region3, regionB, 6)
        self.assertAlmostEqual(regionB, regionC, 6)
        self.assertAlmostEqual(regionC, region3, 6)

    def test_0C2(self):
        region0 = self.fcp.pdf(self.t12, -self.epsilon)
        regionC = self.fcp.pdf(self.t12*(1. - self.epsilon), self.epsilon)
        region2 = self.fcp.pdf(self.t12*(1. + self.epsilon), self.epsilon)

        self.assertAlmostEqual(region0, regionC, 12)
        self.assertAlmostEqual(regionC, region2, 15)
        self.assertAlmostEqual(region2, region0, 12)

    def test_0A5(self):
        region5 = log(
            self.fcp.pdf(self.t15*(1. + self.epsilon), self.epsilon))

        regionA = log(
            self.fcp.pdf(self.t15*(1. - self.epsilon), self.epsilon))

        region0 = log(self.fcp.pdf(self.t15, (-self.epsilon)))

        self.assertAlmostEqual(region5, regionA, 6)
        self.assertAlmostEqual(regionA, region0, 6)
        self.assertAlmostEqual(region0, region5, 6)

    def test_AC0(self):
        regionA = log(
            self.fcp.pdf(self.tAC*(1. + self.epsilon), self.epsilon))

        regionC = log(
            self.fcp.pdf(self.tAC*(1. - self.epsilon), self.epsilon))

        region0 = log(self.fcp.pdf(self.tAC, -self.epsilon))

        self.assertAlmostEqual(regionA, regionC, 7)
        self.assertAlmostEqual(regionC, region0, 7)
        self.assertAlmostEqual(region0, regionA, 7)

    def test_AB4(self):
        regionA = log(self.fcp.pdf(
            self.tAC*(1. + self.epsilon), self.d13*(1. - self.epsilon)))

        regionB = log(self.fcp.pdf(
            self.tAC*(1. - self.epsilon), self.d13*(1. - self.epsilon)))

        region4 = log(self.fcp.pdf(self.tAC, self.d13*(1. + self.epsilon)))

        self.assertAlmostEqual(regionA, regionB, 6)
        self.assertAlmostEqual(regionB, region4, 5)
        self.assertAlmostEqual(region4, regionA, 5)

    def test_123(self):
        region1 = log(self.fcp.pdf(self.t12*(1. - self.epsilon), self.d23))

        region2 = log(self.fcp.pdf(
            self.t12*(1. + self.epsilon), self.d23*(1. - self.epsilon)))

        region3 = log(self.fcp.pdf(
            self.t12*(1. + self.epsilon), self.d23*(1. + self.epsilon)))

        self.assertAlmostEqual(region1, region2, 7)
        self.assertAlmostEqual(region2, region3, 7)
        self.assertAlmostEqual(region3, region1, 7)

    def test_13(self):
        region1 = log(self.fcp.pdf(
            self.t12*(1. - self.epsilon), self.d13*(1. - self.epsilon)))

        region3 = log(self.fcp.pdf(
            self.t12*(1. + self.epsilon), self.d13*(1. + self.epsilon)))

        self.assertAlmostEqual(region1, region3, 7)

    def test_145(self):
        region1 = log(self.fcp.pdf(self.t15*(1. - self.epsilon), self.d23))

        region4 = log(self.fcp.pdf(
            self.t15*(1. + self.epsilon), self.d23*(1. + self.epsilon)))

        region5 = log(self.fcp.pdf(
            self.t15*(1. + self.epsilon), self.d23*(1. - self.epsilon)))

        self.assertAlmostEqual(region1, region4, 4)
        self.assertAlmostEqual(region4, region5, 1)
        self.assertAlmostEqual(region5, region1, 1)

    def test_14(self):
        region1 = log(self.fcp.pdf(
            self.t12*(1 - self.epsilon), self.d13*(1 - self.epsilon)))

        region3 = log(self.fcp.pdf(
            self.t12*(1 + self.epsilon), self.d13*(1 + self.epsilon)))

        self.assertAlmostEqual(region1, region3, 6)

    def test_134(self):
        region1 = log(self.fcp.pdf(self.t34, self.d13*(1. - self.epsilon)))

        region3 = log(self.fcp.pdf(
            self.t34*(1. + self.epsilon), self.d13*(1. + self.epsilon)))

        region4 = log(self.fcp.pdf(
            self.t34*(1. - self.epsilon), self.d13*(1. + self.epsilon)))

        self.assertAlmostEqual(region1, region3, 4)
        self.assertAlmostEqual(region1, region4, 4)
        self.assertAlmostEqual(region3, region4, 4)


if __name__ == "__main__":
    unittest.main()
