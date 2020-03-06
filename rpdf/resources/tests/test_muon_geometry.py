#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""Unit tests for `icecube.rpdf`

"""
import itertools
import unittest

import numpy as np
from icecube import dataclasses, phys_services, rpdf


class TestMuonGeometry(unittest.TestCase):
    """Test case for `icecube.rpdf.muon_geometry`

    """
    def test_cherenkov_time(self):
        """Test Cherenkov time.

        """
        particles = itertools.product(
            np.linspace(0., np.pi, 10),
            np.linspace(0., 2.*np.pi, 10),
            np.linspace(-1000., 1000., 10),
            np.linspace(-1000., 1000., 10),
            np.linspace(-1000., 1000., 10))

        for zenith, azimuth, px, py, pz in particles:
            particle = dataclasses.I3Particle(
                dataclasses.I3Position(pz, px, py),
                dataclasses.I3Direction(zenith, azimuth), 0.)

            particle.shape = dataclasses.I3Particle.InfiniteTrack

            geometry = rpdf.muon_geometry(
                om=particle.pos, track=particle, ice_model=rpdf.H2)

            cherenkov_time = phys_services.I3Calculator.cherenkov_time(
                particle, particle.pos)

            self.assertAlmostEqual(geometry.first, cherenkov_time, 2)


if __name__ == "__main__":
    unittest.main()
