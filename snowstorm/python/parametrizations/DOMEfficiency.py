import copy

from icecube import clsim

from ..parametrization import Parametrization


class DOMEfficiency(Parametrization):
    def transform(self, x, frame):
        """
        Scale the overall DOM efficiency by a factor.
        """
        assert len(x) == 1
        bias = frame['WavelengthGenerationBias']
        acceptance = frame['WavelengthAcceptance']

        # only way to update frame items is deleting and re-inserting them!
        del(frame['WavelengthGenerationBias'])
        del(frame['WavelengthAcceptance'])

        # scale the DOMEfficiency by scaling the DOM wavelength acceptance
        scaled_acceptance = acceptance * x[0]
        # scale the generation bias accordingly
        scaled_bias = bias * x[0]

        # write bias and acceptance back to the frame
        frame['WavelengthGenerationBias'] = scaled_bias
        frame['WavelengthAcceptance'] = scaled_acceptance

        return frame