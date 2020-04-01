
from icecube import dataclasses
from . import Composite, RangeVector, make_pair


class Perturber(object):
    def __init__(self):
        self._distribution = Composite()
        self.components = dict()
        self._offset = 0

    def add(self, name, parametrization, distribution):
        """
        Adds a new systematic parametrization to the perturber.

        :param name: A descriptive name for the parametrization to add.
            This should contain enough information so that analyzers can later identify the parametrizations and
            know how they affect the simulation.
        
        :param parametrization: An instance of `Parametrization` that transforms
            a parameter vector into frame objects.

        :param distribution: An instance of `Distribution` with the same
            dimensionality as `parametrization`.
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
        Store bookkeeping info in an I3Frame.
        This will store the serialized sampling distributions for all parametrizations along with their name in the frame.

        :param frame: The frame to hold the bookkeeping info.
        """
        offsets = sorted(self.components.keys())
        names = [self.components[k]['name'] for k in offsets]
        distributions = [self.components[k]['distribution'] for k in offsets]
        frame['SnowstormProposalDistribution'] = self._distribution
        frame['SnowstormParametrizations'] = dataclasses.I3VectorString(names)
        frame['SnowstormParameterRanges'] = RangeVector([make_pair(o, o+d.size()) for o,d in zip(offsets, distributions)])
        return frame

    def perturb(self, rng, frame):
        """
        Apply all parametrizations that were previously added.

        :param rng: An instance of an I3RandomService to use for sampling.

        :param frame: The frame the parametrizations will be applied to.
        """
        x = self._distribution.Sample(rng)
        frame['SnowstormParameters'] = dataclasses.I3VectorDouble(x)
        offsets = sorted(self.components.keys())
        for offset in offsets:
            component = self.components[offset]
            size = component['distribution'].size()
            component['parametrization'].transform(x[offset:offset+size], frame)
        return frame