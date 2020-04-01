#!/usr/bin/env python

"""
Test UniformDistribution against reference scipy implementation
"""

from icecube.snowstorm import UniformDistribution
from icecube.phys_services import I3GSLRandomService
from icecube.dataclasses import make_pair

import numpy as np
from scipy import stats

loc = 0.85
scale = 0.25
bounds = [loc, loc + scale]

dist = UniformDistribution([make_pair(*bounds)])
rdist = stats.uniform(loc=loc, scale=scale)

rng = I3GSLRandomService(0)
np.random.seed(0)

samples = 10000
pv = np.asarray([dist.Sample(rng)[0] for _ in range(samples)])
pv_ref = rdist.rvs(samples)

ks = stats.ks_2samp(pv, pv_ref)
print(ks)
try:
    assert ks.pvalue > 0.01 and ks.pvalue < 0.99
except AssertionError:
    print('KS test failed! {}'.format(ks))
    raise
