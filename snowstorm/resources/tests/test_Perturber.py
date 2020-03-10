#!/usr/bin/env python

"""
Test serialization of Snowstorm perturber
"""

from icecube.snowstorm import Perturber, MultivariateNormal
from icecube.icetray import I3Frame
from icecube.phys_services import I3GSLRandomService
from icecube.dataclasses import I3Matrix, I3Double

import pickle

import numpy as np
from scipy import stats

covariance = 2*np.eye(3)
covariance[1,2] = -1
covariance[2,1] = -1
covariance[0,1] = 1
covariance[1,0] = 1
mean = [-1,3.5,0]

class PotemkinParametrization(object):
    def transform(self, x, frame):
        for i, v in enumerate(x):
            frame['Value{:02d}'.format(i)] = I3Double(v)
        return frame

p = Perturber()
p.add('PotemkinValues', PotemkinParametrization(), MultivariateNormal(I3Matrix(covariance), np.array(mean)))

# ensure that everything is serializable
frame = p.to_frame(I3Frame('S'))
frame2 = pickle.loads(pickle.dumps(frame))

# ensure that everything is serializable
rng = I3GSLRandomService(0)
mframe = p.perturb(rng, I3Frame('M'))
mframe2 = pickle.loads(pickle.dumps(mframe))

