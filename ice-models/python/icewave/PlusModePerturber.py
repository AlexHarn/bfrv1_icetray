# -*- coding: utf-8 -*-
# Copyright (c) 2019
# Ben Jones <ben.jones@uta.edu>
# and the IceCube Collaboration <http://www.icecube.wisc.edu>
#
# Permission to use, copy, modify, and/or distribute this software for any
# purpose with or without fee is hereby granted, provided that the above
# copyright notice and this permission notice appear in all copies.
#
# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
# WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
# SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
# WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
# OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
# CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
#
# $Id$
#
# @file PlusModePerturber.py
# @version $Revision$
# @date $Date$
# @author Ben Jones, Jakob van Santen

import numpy as np
import copy

from FourierToolset import FourierSeries, PerturbPhases, PerturbAmplitudes
from icecube import clsim

class PlusModePerturber:
    """
    An example perturber that varies the plus modes of the logarithmic
    abs+scat FFTs.
    """

    # proposal distributions for amplitude and phase, hard-coded for now
    amp_sigmas = np.asarray([0.00500100, 0.03900780, 0.04500900, 0.17903581, 0.07101420, 0.30306061, 0.14502901, 0.09501900, 0.16103221, 0.13302661, 0.15703141, 0.13302661])
    phase_sigmas = np.asarray([0.00000001, 0.01664937, 0.02708014, 0.43171273, 0.02351273, 2.33565571, 0.16767628, 0.05414841, 0.31355088, 0.04227052, 0.27955606, 4.02237848])
    modes_to_shift = np.arange(12)
    
    def __init__(self, medium):
        """
        :param medium: a clsim.MediumProperties giving the base model
        """
        self.base_model = medium
        scattering_coefficients = []
        absorption_coefficients = []
        for i in range(medium.GetLayersNum()):
            # CLSim changes the effective scattering coefficient to the
            # scattering coefficient by dividing by 1/(1-cos(dir))
            # where cos(dir) is 0.9
            scattering_coefficients.append(medium.GetScatteringLength(i).b400)
            # Note: This is just the absorption due to dust. There is another
            # term in PPC that is for the absorption of ice.
            # We'll just use the absorption of dust as an approximation.
            absorption_coefficients.append(medium.GetAbsorptionLength(i).aDust400)

        # The deepest layer represents the bedrock, and its absorption
        # coefficient is set to 999. This messes with the Fourier expansion, so
        # we treat it separately
        self._bedrock = [scattering_coefficients.pop(0), absorption_coefficients.pop(0)]

        self._sca = np.asarray(scattering_coefficients)
        self._abs = np.asarray(absorption_coefficients)

        # calculate central models via log prescription
        self._central_plus  = 0.5 * np.log10(self._abs*self._sca)
        self._central_minus = 0.5 * np.log10(self._abs/self._sca)

        # get central Fourier series
        z = medium.GetLayersZStart() + np.arange(1,medium.GetLayersNum())*medium.GetLayersHeight()
        self._central_fs_plus  = FourierSeries(z, self._central_plus)

    def perturb(self, randomService):
        amp_shifts = np.asarray([randomService.gaus(0,a) for a in self.amp_sigmas])
        phase_shifts = np.asarray([randomService.gaus(0,p) for p in self.phase_sigmas])

        fs_plus = PerturbPhases(PerturbAmplitudes(self._central_fs_plus, self.modes_to_shift, amp_shifts), self.modes_to_shift, phase_shifts)

        # convert from frequency space and add bedrock
        scattering_coefficients = np.concatenate(([self._bedrock[0]], 10**(fs_plus[1] - self._central_minus)))
        absorption_coefficients = np.concatenate(([self._bedrock[1]], 10**(fs_plus[1] + self._central_minus)))
        # force top layer to be identical
        scattering_coefficients[-1] = self._sca[-1]
        absorption_coefficients[-1] = self._abs[-1]

        # synthesize a new MediumProperties object
        medium = copy.deepcopy(self.base_model)
        for i in range(0,medium.GetLayersNum()):
            oldScat = medium.GetScatteringLength(i)
            oldAbs  = medium.GetAbsorptionLength(i)

            newScat = clsim.I3CLSimFunctionScatLenIceCube(
                alpha = oldScat.alpha,
                b400  = float(scattering_coefficients[i])
            )

            newAbs = clsim.I3CLSimFunctionAbsLenIceCube(
                kappa    = oldAbs.kappa,
                A        = oldAbs.A,
                B        = oldAbs.B,
                D        = oldAbs.D,
                E        = oldAbs.E,
                aDust400    = float(absorption_coefficients[i]),
                deltaTau = oldAbs.deltaTau
            )

            medium.SetScatteringLength(i, newScat)
            medium.SetAbsorptionLength(i, newAbs)

        return medium
