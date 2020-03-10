#!/usr/bin/env python

"""
Test MultivariateNormal against reference scipy implementation
"""

from icecube.snowstorm import MultivariateNormal
from icecube.phys_services import I3GSLRandomService
from icecube.dataclasses import I3Matrix

import numpy as np
from scipy import stats

covariance = 2*np.eye(3)
covariance[1,2] = -1
covariance[2,1] = -1
covariance[0,1] = 1
covariance[1,0] = 1
mean = [-1,3.5,0]

dist = MultivariateNormal(I3Matrix(covariance), np.array(mean))
rdist = stats.multivariate_normal(mean=mean, cov=covariance)

rng = I3GSLRandomService(0)
np.random.seed(0)

samples = 10000
pv = np.asarray([rdist.logpdf(dist.Sample(rng)) for _ in range(samples)])
pv_ref = rdist.logpdf(rdist.rvs(samples))

ks = stats.ks_2samp(pv, pv_ref)
try:
    assert ks.pvalue > 0.01 and ks.pvalue < 0.99
except AssertionError:
    print('KS test failed! {}'.format(ks))
    raise
