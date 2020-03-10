
from icecube import dataclasses
from . import Composite, RangeVector, make_pair

class Parametrization(object):
    def transform(self, x, frame):
        """
        Transform any objects in `frame` according to the parameters `x`
        """
        raise NotImplementedError

class Perturber(object):
    def __init__(self):
        self._distribution = Composite()
        self.components = dict()
        self._offset = 0
    def add(self, name, parametrization, distribution):
        """
        :param parametrization: an instance of Parametrization that transforms
            a parameter vector into frame objects
        :param distribution: an instance of `Distribution` with the same
            dimensionality as `parametrization`
        :param name: a descriptive name for the parametrization. This should
            contain enough information so that downstream analyzers will know
            how they affect the simulation
        """
        assert distribution.size() > 0
        self._distribution.add(distribution)
        self.components[self._offset] = {
            'name': name,
            'parametrization': parametrization,
            'distribution': distribution,
        }
        self._offset += distribution.size()
    def to_frame(self, frame):
        """
        Store bookkeeping info in an I3Frame
        """
        offsets = sorted(self.components.keys())
        names = [self.components[k]['name'] for k in offsets]
        distributions = [self.components[k]['distribution'] for k in offsets]
        frame['SnowstormProposalDistribution'] = self._distribution
        frame['SnowstormParametrizations'] = dataclasses.I3VectorString(names)
        frame['SnowstormParameterRanges'] = RangeVector([make_pair(o, o+d.size()) for o,d in zip(offsets, distributions)])
        return frame
    def perturb(self, rng, frame):
        x = self._distribution.Sample(rng)
        frame['SnowstormParameters'] = dataclasses.I3VectorDouble(x);
        offsets = sorted(self.components.keys())
        for offset in offsets:
            component = self.components[offset]
            size = component['distribution'].size()
            component['parametrization'].transform(x[offset:offset+size], frame)
        return frame