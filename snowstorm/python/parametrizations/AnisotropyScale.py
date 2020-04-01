import copy

from icecube import clsim

from ..parametrization import Parametrization


class AnisotropyScale(Parametrization):
    def transform(self, x, frame):
        """
        Scale the anisotropy strength by a factor.
        """
        assert len(x) == 1

        # get MediumProperties
        medium = frame['MediumProperties']
        new_medium = copy.deepcopy(medium)

        # get Anisotropy parameters and do the scaling
        anisotropyDirAzimuth, magnitudeAlongDir, magnitudePerpToDir = medium.GetAnisotropyParameters()
        absLenScaling, preScatterTransform, postScatterTransform = \
            clsim.util.GetSpiceLeaAnisotropyTransforms(
                anisotropyDirAzimuth,
                magnitudeAlongDir*x[0],
                magnitudePerpToDir*x[0])

        new_medium.SetDirectionalAbsorptionLengthCorrection(absLenScaling)
        new_medium.SetPreScatterDirectionTransform(preScatterTransform)
        new_medium.SetPostScatterDirectionTransform(postScatterTransform)
        new_medium.SetAnisotropyParameters(anisotropyDirAzimuth, magnitudeAlongDir*x[0], magnitudePerpToDir*x[0])

        # update MediumProperties by deleting and re-inserting them into the frame
        del(frame['MediumProperties'])
        frame['MediumProperties'] = new_medium

        # cleanup
        del(medium, new_medium)

        # return frame
        return frame