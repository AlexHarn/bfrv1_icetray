from icecube import icetray, dataclasses, dataio
from icecube import gulliver, cramer_rao, linefit
from icecube.icetray import I3Units
import math, os, numpy

from icecube.lilliput.segments import I3SinglePandelFitter, I3IterativePandelFitter

from I3Tray import *

@icetray.traysegment
def SPE(tray, name, Pulses = '', If = lambda f: True, suffix = '',
        LineFit = 'LineFit',
        SPEFitSingle = 'SPEFitSingle',
        SPEFit = 'SPEFit2',
        SPEFitCramerRao = 'SPEFit2CramerRao',
        N_iter = 2,
        ):

    tray.AddSegment( linefit.simple, LineFit+suffix, inputResponse = Pulses, fitName = LineFit+suffix, If = If )

    tray.AddSegment( I3SinglePandelFitter, SPEFitSingle+suffix, pulses = Pulses, seeds = [LineFit+suffix], If = If )
   
    if N_iter > 1:
        tray.AddSegment( I3IterativePandelFitter, SPEFit+suffix, pulses = Pulses, n_iterations = N_iter, seeds = [ SPEFitSingle+suffix ], If = If )

    #use only first hits.  Makes sense for an SPE likelihood
    tray.AddModule('CramerRao', name + '_' + SPEFitCramerRao + suffix,
                   InputResponse = Pulses,
                   InputTrack = SPEFit+suffix,
                   OutputResult = SPEFitCramerRao+suffix,
                   AllHits = False, # ! doesn't make sense to use all hit for SPE pdf
                   DoubleOutput = False, # Default
                   z_dependent_scatter = True, # Default
                   If = If,
                   )
