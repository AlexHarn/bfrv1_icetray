import copy

from icecube import clsim

from ..parametrization import Parametrization


class Absorption(Parametrization):
    def transform(self, x, frame):
        """
        Scale the global ice-absorption by a factor.
        """

        assert len(x) == 1

        # get MediumProperties
        medium = frame['MediumProperties']
        new_medium = copy.deepcopy(medium)

        # loop over all layers
        for i in range(0, medium.GetLayersNum()):
            # scale the absorption length
            new_medium.SetAbsorptionLength(i, medium.GetAbsorptionLength(i)*(1.0/x[0]))

        # update MediumProperties by deleting and re-inserting them into the frame
        del(frame['MediumProperties'])
        frame['MediumProperties'] = new_medium

        # cleanup
        del(medium, new_medium)

        # return frame
        return frame