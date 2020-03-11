import copy
import numpy as np

from icecube import clsim
from icecube.ice_models import angsens_unified


class HoleIceForward_Unified:
    """
    Modify the angular acceptance in the forward direction following
    the 'unified model' by Phillipp Eller:
    https://github.com/philippeller/angular_acceptance
    """

    def transform(self, x, frame):
        """
        pass this function a length 2 array with new values for p0 and p1.
        Use the unified_angular_acceptance hole ice model to
        recalculate the Angular Acceptance
        """
        assert len(x) == 2
        p0, p1 = x

        # get AngularAcceptance from frame
        AngularAcceptance = frame["AngularAcceptance"]

        # initialize the new angular acceptance function:
        self.angular_acceptance = lambda vals: angsens_unified.angular_acceptance_function([p0, p1], vals)

        # xp is range of values for cosine of photon zenith angle. cos(eta) = xp = 1 is upgoing!
        xp = np.linspace(-1, 1, 1001)
        # yp is then the new hole-ice forward
        yp = self.angular_acceptance(xp)

        # trapz integrates, using the trapezoidal rule, along the given axis.
        # scales the new function to the not forward one. (preserves the integral)
        # note: it is normalized to an integral of 0.68
        yp *= 0.68/np.trapz(yp, xp)
        coefficients = np.polyfit(xp, yp, 10)[::-1]

        # replace the previous AngularAcceptance with a new one. An 11-term polynomial describing angular acceptance wrt cos(eta)
        new_AngularAcceptance = clsim.I3CLSimFunctionPolynomial(coefficients, -1.0, 1.0)

        # only way to update frame items is deleting and re-inserting
        del(frame['AngularAcceptance'])
        frame['AngularAcceptance'] = new_AngularAcceptance

        # rescale the WavelengthGenerationBias according to the new peak AngularAcceptance
        peak = np.max([AngularAcceptance.GetValue(i) for i in np.linspace(-1, 1, 1001)])
        new_peak = np.max([new_AngularAcceptance.GetValue(i) for i in np.linspace(-1, 1, 1001)])
        peak_ratio = new_peak / peak

        # scale the peak ratio up by 1% to be sure
        #peak_ratio *= 1.01

        # get the WavelengthGenerationBias
        bias = frame['WavelengthGenerationBias']

        # do the scaling
        scaled_bias = bias * peak_ratio

        # only way to update frame items is deleting and re-inserting
        del(frame['WavelengthGenerationBias'])
        frame['WavelengthGenerationBias'] = scaled_bias

        # cleanup
        del(bias, scaled_bias, AngularAcceptance, new_AngularAcceptance)

        # return modified M-Frame
        return frame