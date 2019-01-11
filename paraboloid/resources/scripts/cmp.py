#!/usr/bin/env python

# Comparing old (ROOT-based) and new (scipy-based) paraboloid
# note that this script does not really test anything yet
# except that it can run without crashing
# If you are interested in this stuff then you could extend this
# script to e.g. run over more events, use tableio to write analysis
# output and study how much better/worse the performances are.
# The new method does not (yet) try to be better, only to implement
# the existing algorithm. It als probably also slower.

import os, sys
from os.path import expandvars
testdata=os.environ["I3_TESTDATA"]
gcd = os.path.join(testdata,"sim/GeoCalibDetectorStatus_IC86.55697_corrected_V2.i3.gz")
data = os.path.join(testdata,"sim/Level2_IC86.2011_corsika.010281.001664.00.i3.bz2")
pulses="SRTOfflinePulses"

##################################################################
# LOAD LIBS, CREATE A TRAY
##################################################################

from I3Tray import *
from icecube import icetray,dataclasses,dataio,linefit

try:
    from icecube import tableio,rootwriter,hdfwriter
    from icecube import gulliver,lilliput,paraboloid,gulliver_modules
    from icecube.paraboloid.pyraboloid import pyraboloid
    import icecube.lilliput.segments
except ImportError:
    icetray.logging.log_notice("Looks like you built with USE_ROOT=OFF.")
    icetray.logging.log_notice("Going to skip this test.")
    sys.exit(0)
from noisy_pybindings_reporter import noisy_pybindings_reporter

tray = I3Tray()

# read input files
tray.AddModule( "I3Reader", "reader",
                filenameList=[gcd,data] )
tablekeys=["I3EventHeader","I3MCTree",pulses]

# get first guess track
tray.AddSegment(linefit.simple,
    fitName="linefit",
    inputResponse=pulses,
)
tablekeys+=["linefit","linefitParams"]

# standard Pandel SPE fit
mininame,paraname,llhname,seedname,fitname = \
        tray.AddSegment(lilliput.segments.I3SinglePandelFitter,"SPEpandelfit",
                pulses=pulses,
                seeds=["linefit"])
tablekeys+=[fitname,fitname+"FitParams"]

# standard paraboloid for Pandel SPE fit
pbf_mininame,pbf_llhname,pbf_input_seedname,pbf_grid_seedname,pbf_name = \
tray.AddSegment(lilliput.segments.I3ParaboloidPandelFitter, "pbf",
    pulses=pulses,
    inputtrack=fitname,
    input_tstype='TNone',
    grid_tstype='TFirst'
)
tablekeys+=["pbf","pbfFitParams"]

# DING! pyraboloid for Pandel SPE fit
# make sure it gets the same input and services as standard paraboloid,
# so that we can compare the results
tray.Add(pyraboloid, "pybf",
        LikelihoodService=pbf_llhname,
        SeedService=pbf_input_seedname,
        GridPointSeedService=pbf_grid_seedname,
        MinimizerService=pbf_mininame,
        VertexStepSize=5.*I3Units.m,
        NRings=3,
        NPointsPerRing=8,
        Radius=2.*I3Units.degree)

tray.Add(noisy_pybindings_reporter)

# DO IT
tray.Execute(23)

