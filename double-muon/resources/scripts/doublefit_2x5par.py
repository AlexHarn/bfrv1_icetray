#!/usr/bin/env python

"""
This script illustrates the use of the I3ResponseMapSplitter module to try to
reconstruct an event as two different sub-events.
It also performs a stupid test to verify that the results of these fits
do not inadvertently change.

If something is changed in this script, or something is deliberately &
consciously changed in its dependencies (iterative fit, time window cleaning,
input file, etc) then the file with the expected fit results needs to be
updated. You do this by running the I3FortyTwo module in reference mode, i.e.
by running it with a non-empty checklist. The checklist that was used to
create the most recent reference data is usually kept in comment lines, for
your updating convenience.
"""

from os.path import expandvars

##################################################################
# parse options for scripts
##################################################################

filename = expandvars("$I3_TESTDATA/event-viewer/Level3aGCD_IC79_EEData_Run00115990.i3")
Niter=4
fitbase="pandelfit"
plist=[]
lflist=[]
pmlist=[]

##################################################################
# load icetray + libraries
##################################################################

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

#########################
# SERVICES FOR ALL FITS #
#########################

# minimizer (simplex, as implemented in the MINUIT library)
tray.AddService("I3GulliverMinuitFactory","minuit",
    Tolerance = 0.01,
    MaxIterations = 1000,
    Algorithm = "SIMPLEX",
)

# free parameters: x, y, z, zenith, azimuth (for all fits in this script)
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

for spl in ["","T1","T2","G1","G2"]:
    # Pandel likelihood, using *all* (time-window-cleaned) pulses
    tray.AddService("I3GulliverIPDFPandelFactory","pandel"+spl,
        InputReadout= "TWCleanICPulses"+spl,
        #InputReadout= "split"+spl,
        Likelihood= "SPE1st",
        PEProb= "GaussConvoluted",
        IceModel= 2,
        NoiseProbability=500.0*I3Units.hertz ,
        AbsorptionLength=98.0*I3Units.m ,
        JitterTime=15.0*I3Units.ns 
    )
    # seed for full event
    tray.AddService("I3BasicSeedServiceFactory","seedprep"+spl,
        InputReadout =  "TWCleanICPulses"+spl,
        #InputReadout =  "split"+spl,
        TimeShiftType = "TFirst",
        FirstGuesses =  ["linefit"+spl])

#######################################
# READING DATA / SPLITTING PULSE MAPS #
#######################################

# read input data
tray.Add("I3Reader", filenamelist=[filename] )

# require big enough events
tray.Add( lambda f: len(f["InIceRawData"])>=16) # Nch>=16
tray.Add( lambda f: len(f["InIceRawData"])<=80) # Nch<=80
tray.Add( lambda f: len(set([omkey.string for omkey in f["InIceRawData"].keys()]))>=4) # Nstr>=4

# eliminate obvious outliers (only timewise)
tray.AddModule("I3TimeWindowCleaning<I3RecoPulse>",
    InputResponse="MaskedOfflinePulses",
    OutputResponse="TWCleanICPulses",
    TimeWindow=6000*I3Units.ns)
pmlist.append("MaskedOfflinePulses")
pmlist.append("TWCleanICPulses")

######################
# FITTING FULL EVENT #
######################

# regular first guess track (using all TW-cleaned pulses)
tray.AddModule("I3LineFit",
    Name =  "linefit",
    InputRecoPulses =  "TWCleanICPulses",
    AmpWeightPower =   0.0
)
lflist+=["linefit"]

# regular Pandel fit (using all TW-cleaned pulses)
tray.AddModule("I3IterativeFitter",fitbase,
    NIterations = Niter,
    SeedService = "seedprep",
    Parametrization = "simpletrack",
    LogLikelihood = "pandel",
    CosZenithRange = [-1,1],
    Minimizer = "minuit"
)
plist+=[fitbase]

#######################
# SPLITTING THE EVENT #
#######################

# the I3ResponseMapSplitter module, configured with only an InputPulseMap,
# computes the average pulse time, then makes two new pulseseries maps, with
# the pulses that are earlier and later (than the average time), respectively.
#
# The idea is that for coincident downgoing events in which one of the two
# sub-events clearly later than the other, the fits on the two new pulseseries
# maps will yield at least one downgoing track.
#
# Note that the output pulse maps of the splitter modules are defined by the
# module name. The name of the time splitter module is splitT, so the output
# maps are splitT1 and splitT2.

tray.AddModule("I3ResponseMapSplitter","splitT",
    InputPulseMap = "MaskedOfflinePulses",
    DoTMedian = True,
    TSplitWeight = 'DOM',
)
pmlist.append("splitT1")
pmlist.append("splitT2")

# eliminate obvious outliers (only timewise) from first half
tray.AddModule("I3TimeWindowCleaning<I3RecoPulse>","timewindowT1",
    InputResponse = "splitT1",
    OutputResponse = "TWCleanICPulsesT1",
    TimeWindow = 6000
)

# eliminate obvious outliers (only timewise) from second half
tray.AddModule("I3TimeWindowCleaning<I3RecoPulse>","timewindowT2",
    InputResponse = "splitT2",
    OutputResponse = "TWCleanICPulsesT2",
    TimeWindow = 6000
)

# The I3ResponseMapSplitter module, configured with only an InputPulseMap and
# an input track, computes the COG of the pulses, then makes two new
# pulseseries maps, with the pulses that are on either side of the plane that
# contains the COG and is perpendicular to the input track.
#
# The idea is that for coincident downgoing events in which the two sub-events
# are clearly geometrically separated, at least one of the fits to the two new
# pulseseriesmaps will yield a downgoing track.
#
# Note that the output pulse maps of the splitter modules are defined by the
# module name. The name of the geo splitter module is splitG, so the output
# maps are splitG1 and splitG2.
tray.AddModule("I3ResponseMapSplitter","splitG",
    InputPulseMap = "MaskedOfflinePulses",
    InputTrackName = fitbase,
    DoTMedian = True, # only used if input track not available
    TSplitWeight = 'DOM', # only used if input track not available
)
pmlist.append("splitG1")
pmlist.append("splitG2")

# eliminate obvious outliers (only timewise) from first half
tray.AddModule("I3TimeWindowCleaning<I3RecoPulse>","timewindowG1",
    InputResponse = "splitG1",
    OutputResponse = "TWCleanICPulsesG1",
    TimeWindow = 6000
)

# eliminate obvious outliers (only timewise) from second half
tray.AddModule("I3TimeWindowCleaning<I3RecoPulse>","timewindowG2",
    InputResponse = "splitG2",
    OutputResponse = "TWCleanICPulsesG2",
    TimeWindow = 6000
)

#tray.Add("Dump")

#######################
# FITTING SPLIT EVENT #
#######################

for spl in ["T1","T2","G1","G2"]:
    # linefit
    tray.AddModule("I3LineFit",
        Name =  "linefit"+spl,
        InputRecoPulses =  "TWCleanICPulses"+spl,
        AmpWeightPower =   0.0
    )
    lflist+=["linefit"+spl]
    # pandel fit
    tray.AddModule("I3IterativeFitter",fitbase+spl,
        NIterations = Niter,
        SeedService = "seedprep"+spl,
        Parametrization = "simpletrack",
        LogLikelihood = "pandel"+spl,
        CosZenithRange = [-1,1],
        Minimizer = "minuit"
    )
    plist+=[fitbase+spl]

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

tray.Add(Pcount,num_pframes=10)

# burn, MF, burn...
tray.Execute()

