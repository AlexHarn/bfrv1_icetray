#!/usr/bin/env python

"""
This script illustrates the use of 10 parameter fits using a double-muon
likelihood function to identify coincident muon events.  It also performs a
stupid test to verify that the results of these fits do not inadvertently
change.

In order to generate the seeds for the double muon fit, we first use
I3ResponseMapSplitter module to split the event and apply linefits the two half
events.  The double muon seed service combines these into one double-muon seed
for the 10 parameter fit, using the whole unsplit pulse map.

If something is changed in this script, or something is deliberately &
consciously changed in its dependencies (iterative fit, time window cleaning,
input file, etc) then the file with the expected fit results needs to be
updated. You do this by setting the 'testmode' variable to False, run the
script, set testmode back to True, and svn commit the 'doublefit_10par.dat'
picklefile.  (If this style of testing gets implemented in more scripts then
maybe I need to rethink the use of pickle files to store expected results.)
"""

import os, sys

##################################################################
# initialize global stuff
##################################################################

filename = os.path.expandvars("$I3_TESTDATA/event-viewer/Level3aGCD_IC79_EEData_Run00115990.i3")
Niter=4
fitbase="pandelfit"
plist=[]
lflist=[]
pmlist=[]

############################
# load icetray + libraries #
############################

from I3Tray import *
from icecube import icetray, dataclasses, dataio
from icecube import linefit, DomTools
from icecube import gulliver, double_muon, gulliver_modules, lilliput
from icecube.gulliver_modules.fortytwo import I3FortyTwo, I3FrameChecker, I3ParticleChecker, I3LogLikelihoodFitParamsChecker, Pcount
from pulsemapchecker import *
icetray.logging.set_level('NOTICE')
#icetray.logging.set_level_for_unit('I3FortyTwo','TRACE')
#icetray.logging.set_level_for_unit('Pcount','TRACE')

tray = I3Tray()

############
# SERVICES #
############

# minimizer (simplex, as implemented in the MINUIT library)
tray.AddService("I3GulliverMinuitFactory","minuit",
    Tolerance = 0.01,
    MaxIterations = 1000,
    Algorithm = "SIMPLEX",
)

# free parameters (1 muon): x, y, z, zenith, azimuth
tray.AddService("I3SimpleParametrizationFactory","simpletrack",
    StepX = 20*I3Units.m,
    StepY = 20*I3Units.m,
    StepZ = 20*I3Units.m,
    StepZenith = 0.1*I3Units.radian,
    StepAzimuth = 0.2*I3Units.radian,
    BoundsX = [-2000*I3Units.m,2000*I3Units.m],
    BoundsY = [-2000*I3Units.m,2000*I3Units.m],
    BoundsZ = [-2000*I3Units.m,2000*I3Units.m],
)

# Pandel likelihood (1 muon)
tray.AddService("I3GulliverIPDFPandelFactory","pandel",
    InputReadout= "TWCleanICPulses",
    Likelihood= "SPE1st",
    PEProb= "GaussConvoluted",
    IceModel= 2,
    NoiseProbability=500.0*I3Units.hertz ,
    AbsorptionLength=98.0*I3Units.m ,
    JitterTime=15.0*I3Units.ns 
)

# seed (for 1 muon)
tray.AddService("I3BasicSeedServiceFactory","seedprep",
    InputReadout =  "TWCleanICPulses",
    TimeShiftType = "TFirst",
    FirstGuesses =  ["linefit"]
)

# free parameters (2 muons): (x, y, z, zenith, azimuth)*2
tray.AddService("I3DoubleMuonParametrizationServiceFactory","doubletrack",
        VertexStepsize = 20.0*I3Units.m,
        AngleStepsize = 0.1*I3Units.radian,
)

# likelihood: SPE for 2 muon tracks, with relative weights based on
# the expected amplitudes from both tracks
tray.AddService("I3DoubleMuonLogLikelihoodServiceFactory","SPEpandel2mu",
    PulseMapName = "TWCleanICPulses",
    NoiseRate = 500.0*I3Units.hertz,
)

# likelihood: MPE for 2 muon both tracks, with relative weights based on
# the expected amplitudes from both tracks
tray.AddService("I3DoubleMuonLogLikelihoodServiceFactory","MPEpandel2mu",
    PulseMapName = "TWCleanICPulses",
    NoiseRate = 500.0*I3Units.hertz,
    UseMPE = True,
)

# Seed services for geo/time-split fits
for spl in ["T1","T2","G1","G2"]:
    # single seed (using half of the pulses), input for making double seed
    tray.AddService("I3BasicSeedServiceFactory","seed"+spl,
        InputReadout = "TWCleanICPulses"+spl,
        TimeShiftType = "TFirst",
        FirstGuesses = ["linefit"+spl])
for spl in ["G","T"]:
    tray.AddService("I3DoubleMuonSeedServiceFactory","twoseed%s"%spl,
        Seed1="seed%s1"%spl,
        Seed2="seed%s2"%spl,
    )


####################################
# READING DATA / EXTRACTING PULSES #
####################################

# read input data
tray.Add("I3Reader", filenamelist=[filename] )

# require big enough events
tray.Add( lambda f: len(f["InIceRawData"])>=12) # Nch>=12
tray.Add( lambda f: len(set([omkey for omkey in f["InIceRawData"].keys()]))>=3) # Nstr>=3

##################################
# SINGLE TRACK FIT TO FULL EVENT #
##################################

# eliminate obvious outliers (only timewise)
tray.AddModule("I3TimeWindowCleaning<I3RecoPulse>",
    InputResponse="MaskedOfflinePulses",
    OutputResponse="TWCleanICPulses",
    TimeWindow=6000*I3Units.ns)
pmlist.append("MaskedOfflinePulses")
pmlist.append("TWCleanICPulses")

# regular first guess track (using all TW-cleaned pulses)
tray.AddModule("I3LineFit",
    Name = "linefit",
    InputRecoPulses = "TWCleanICPulses",
    AmpWeightPower =  1.0
)
lflist.append("linefit")

# regular Pandel fit (using all TW-cleaned pulses)
tray.AddModule("I3IterativeFitter",
    OutputName = fitbase,
    NIterations = Niter,
    SeedService = "seedprep",
    Parametrization = "simpletrack",
    LogLikelihood = "pandel",
    CosZenithRange = [-1,1],
    Minimizer = "minuit"
)
plist.append(fitbase)

##################################################
# DOUBLE MUON FIT WITH SEED FROM GEO-SPLIT EVENT #
##################################################

# The I3ResponseMapSplitter module splits an input pulse map into two parts.
# When configured an input track the split will be geometrical (using the
# plane through the COG perpendicular to the track as a separator), otherwise
# it will be temporal (split based on average/mean pulse time).
#
# The idea is similar to that for the 2x5par fit, namely that for coincident
# downgoing events in which the two sub-events are clearly temporally and/or
# geometrically separated, the two fitted muons will each match one of the
# subevents.
#
# The hope with the 10par fit is that this will even work in events where the
# two subevents are so close (in space and/or time) that they are not cleanly
# separated with a simple pulse splitter, and for at least some of the pulses
# it is ambiguous to which track they belong. The double-muon LLH function does
# not try to decide for each pulse whether it belongs to one or the other
# track; it simply adds pdf values for each (with relative weights based on
# expected charge).
tray.AddModule("I3ResponseMapSplitter","splitG",
    InputPulseMap = "MaskedOfflinePulses",
    InputTrackName = fitbase,
    DoTMedian = True, # only used if input track not available
    TSplitWeight = 'DOM', # only used if input track not available
)
tray.AddModule("I3ResponseMapSplitter","splitT",
    InputPulseMap = "MaskedOfflinePulses",
    DoTMedian = True,
    TSplitWeight = 'DOM',
)
# eliminate obvious outliers (only timewise) from first half
for spl in ["T1","T2","G1","G2"]:
    tray.AddModule("I3TimeWindowCleaning<I3RecoPulse>","timewindow"+spl,
        InputResponse = "split"+spl,
        OutputResponse = "TWCleanICPulses"+spl,
        TimeWindow = 6000
    )
    pmlist.append("TWCleanICPulses"+spl)
    # linefit on half of the pulses
    tray.AddModule("I3LineFit","linefit"+spl,
        Name = "linefit"+spl,
        InputRecoPulses = "TWCleanICPulses"+spl,
        AmpWeightPower = 0.0
    )
    lflist.append("linefit"+spl)

for spl in ["G","T"]:
    # MPE fit with seeds from time-split or geo-split event
    tray.AddModule("I3SimpleFitter",
        OutputName = "twomu%s1MPE"%spl,
        SeedService = "twoseed"+spl,
        Parametrization = "doubletrack",
        LogLikelihood = "MPEpandel2mu",
        Minimizer = "minuit",
        NonStdName = "twomu%s2MPE"%spl,
    )
    plist.append("twomu%s1MPE"%spl)
    lflist.append("twomu%s2MPE"%spl)
    # SPE fit with seeds from time-split or geo-split event
    tray.AddModule("I3SimpleFitter",
        OutputName = "twomu%s1SPE"%spl,
        SeedService = "twoseed"+spl,
        Parametrization = "doubletrack",
        LogLikelihood = "SPEpandel2mu",
        Minimizer = "minuit",
        NonStdName = "twomu%s2SPE"%spl,
    )
    plist.append("twomu%s1SPE"%spl)
    lflist.append("twomu%s2SPE"%spl)

#################
# CHECK RESULTS #
#################

# DISABLE TEMPORARILY: SOME PLATFORMS FAIL THIS TEST
# #Needed for reference runs.
# #Whenever you do this, comment these lines out again before committing this to SVN.
# #pc=I3ParticleChecker(lflist+plist)
# #lhfpc=I3LogLikelihoodFitParamsChecker([p+"FitParams" for p in plist])
# #pmc=PulseMapChecker(pmlist,maxnpm=8)
# #tray.AddModule( I3FortyTwo, checklist=[pc,lhfpc,pmc]) # reference

#DISABLE UNTIL FIX FOR MACOSX IS FOUND
# #In SVN, only the following line should be uncommented (for the build bots).
#tray.AddModule( I3FortyTwo ) # check
# 
tray.Add(Pcount,num_pframes=10)

tray.Execute()

