#!/usr/bin/env python

"""
Test MultivariateNormal against reference scipy implementation
"""

from icecube.snowstorm import DeltaDistribution
from icecube.phys_services import I3GSLRandomService

x0 = 0.56

dist = DeltaDistribution([x0])

rng = I3GSLRandomService(0)

samples = 10
pv = [dist.Sample(rng)[0] for _ in range(samples)]

try:
    for v in pv:
        assert v == x0
except AssertionError:
    print('DeltaDistribution sampling test failed! Seeded x0={}, but sampled {}'.format(x0, v))
    raise
