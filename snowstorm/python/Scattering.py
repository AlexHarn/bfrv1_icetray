import copy

from icecube import clsim


class Scattering:
    def transform(self, x, frame):
        """
        Scale the scattering by a factor
        """

        assert len(x) == 1

        # get MediumProperties
        medium = frame['MediumProperties']
        new_medium = copy.deepcopy(medium)

        # loop over all layers
        for i in range(medium.GetLayersNum()):
            # scale the scattering length
            new_medium.SetScatteringLength(i, medium.GetScatteringLength(i)*(1.0/x[0]))

        # update MediumProperties by deleting and re-inserting them into the frame
        del(frame['MediumProperties'])
        frame['MediumProperties'] = new_medium

        # cleanup
        del(medium, new_medium)

        # return modified frame
        return frame