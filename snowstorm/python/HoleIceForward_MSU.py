import copy
import numpy as np

from icecube import clsim


class HoleIceForward_MSU:
    """
    Modify the angular acceptance in the forward direction following
    https://wiki.icecube.wisc.edu/index.php/MSU_Forward_Hole_Ice
    """
    @classmethod
    def DimaHI(cls, x, p1 = 0.25):
        # Dima's hole ice model
        # The magic numbers come from a fit of the ice.
        angular_acceptance = 0.34*( 1 + 1.5*x - (x*x*x)/2. ) + p1*x*( x*x - 1.)**3

        return(angular_acceptance)

    @classmethod
    def DimaHI_fwd(cls, x, p1=0.25, p2 = 0.0):
        # MSU forward hole ice. Modifies Dima's model, but only in the forward direction.
        #   as above, magic numbers come from a fit of the ice
        MSU_angular_acceptance = cls.DimaHI(x, p1) + p2*np.exp( 10.*(x-1.2) )

        return(MSU_angular_acceptance)

    def transform(self, x, frame):
        """
        pass this function a length 2 array with new values for p1 and p2.
        Use Dima's hole ice model to recalculate the Angular Acceptance
        """
        assert len(x) == 2
        p1, p2 = x

        # get AngularAcceptance from frame
        AngularAcceptance = frame["AngularAcceptance"]

        # xp is range of values for cosine of photon zenith angle. cos(eta) = xp = 1 is upgoing!
        xp = np.linspace(-1, 1, 1001)
        # yp is then the Dima hole ice forward
        yp = self.DimaHI_fwd(xp, p1, p2)

        # trapz integrates, using the trapezoidal rule, along the given axis.
        # scales the HI_fwd function to the not forward one. (preserves the integral)
        # note: Dima_HI is normalized to an integral of 0.68
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