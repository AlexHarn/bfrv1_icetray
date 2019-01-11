#!/usr/bin/env python

# special request from Marcel Zoll: paraboloid fit should run on frames with insufficient hits
# unclear why...
# I made it possible, it won't crash but user will be flooded with angry error messages.

from os.path import expandvars
from icecube import icetray, dataclasses, dataio, gulliver, paraboloid
from I3Tray import *
load("linefit")
load("lilliput")
load("gulliver-modules")

#################################################################################
# BOGOGENESIS: module to generate bogus pulsemaps with nch=0, 1, 2, etc
#################################################################################
npulses=0
def bogogenesis(frame):
    global npulses
    pulses=dataclasses.I3RecoPulseSeriesMap()
    icetray.logging.log_notice("BOGOGENESIS: making a map with %d bogus pulses" % npulses)
    for i in range(npulses):
        p=dataclasses.I3RecoPulse()
        p.time=(10000+300.*i)*I3Units.ns
        p.charge=1
        p.width=42
        pulseseries=dataclasses.I3RecoPulseSeries()
        pulseseries.append(p)
        omlist=frame["I3Geometry"].omgeo.keys()
        omkey=omlist[i%len(omlist)]
        pulses[omkey]=pulseseries
    nch=len(pulses.keys())
    #if not nch==npulses:
    #    icetray.logging.log_error("BOGOGENESIS ERROR: npulses=%d but nch=%d" % (npulses,nch))
    #    icetray.logging.log_fatal("PROGRAMMING ERROR")
    frame.Put("ZollMap",pulses)
    npulses += 1 # next time the map will have more pulses

#################################################################################
# ZOLLKONTROLLE: module to check that event complies with user's expectations
#################################################################################
def zollkontrolle(frame):
    if not frame.Has("ZollMap"):
        icetray.logging.log_fatal("ZollMap FEHLT!")
    if not frame.Has("parazoll"):
        icetray.logging.log_fatal("parazoll FEHLT!")
    if not frame.Has("parazollFitParams"):
        icetray.logging.log_fatal("parazollFitParams FEHLT!")
    nch=len(frame["ZollMap"])
    ndf=nch-5
    fitstatus=frame.Get("parazoll").fit_status
    pbfstatus=frame.Get("parazollFitParams").pbfStatus
    icetray.logging.log_notice("ndf=%d, fit_status=%s, pbfStatus=%s" % (ndf,fitstatus,pbfstatus))
    if ndf<=0:
        icetray.logging.log_notice("zollkontrolle: ndf=%d<=0, going to check fit status" % ndf)
        if (fitstatus==0 or pbfstatus==0):
            icetray.logging.log_fatal("PROGRAMMING ERROR: ndf<=0 event with OK paraboloid status")
        else:
            icetray.logging.log_notice("zollkontrolle: fit failed, as it should.")
    else:
        icetray.logging.log_notice("zollkontrolle: ndf=%d>0, fit might fail or succeed as it pleases" % ndf)

#################################################################################
# TRAY: add i3 reader, bogogenesis, linefit, paraboloid, and zollkontrolle
#################################################################################

tray = I3Tray()

# read GCD and then a steady stream of Q and P frames
tray.AddModule("I3Reader","dummyinput",
    FileNameList=[expandvars("$I3_TESTDATA/event-viewer/Level3aGCD_IC79_EEData_Run00115990.i3")]
)

# create test pulses with increasing nch
tray.AddModule(bogogenesis,Streams=[icetray.I3Frame.Physics])

# minimizer (simplex, as implemented in ROOT's Minuit library)
tray.AddService("I3GulliverMinuitFactory","minuit",
    Algorithm = "SIMPLEX",
)

# Gauss convoluted Pandel, using the bogus ZollMap
tray.AddService("I3GulliverIPDFPandelFactory","spe1st",
    InputReadout = "ZollMap",
    Likelihood = "SPE1st",
    PEProb = "GaussConvoluted",
    NoiseProbability = 1.0e4*I3Units.hertz,
    IceModel = 2,
    AbsorptionLength = 98.0*I3Units.m ,
    JitterTime = 4.0*I3Units.ns ,
)

# DO NOT COPY
# THIS IS INTENTIONALLY WRONG
# IT DOES NOT MAKE SENSE TO RUN PARABOLOID WITH LINEFIT
# WE DO THIS HERE ONLY FOR THE SAKE OF THIS PARTICULAR TEST
tray.AddService("I3BasicSeedServiceFactory","paraseedprep",
    InputReadout = "ZollMap",
    TimeShiftType = "TFirst",
    FirstGuesses = ["linefit"],
)

# stupid fit, succeeds also with Nch=2
tray.AddModule("I3LineFit","linefit",
    Name =  "linefit",
    InputRecoPulses =  "ZollMap",
    AmpWeightPower =  1.0,
)

# DO NOT COPY
# THIS IS INTENTIONALLY WRONG
# IT DOES NOT MAKE SENSE TO RUN PARABOLOID WITH LINEFIT
# WE DO THIS HERE ONLY FOR THE SAKE OF THIS PARTICULAR TEST
tray.AddModule("I3ParaboloidFitter", "parazoll",
    OutputName="parazoll",
    SeedService="paraseedprep",
    LogLikelihood="spe1st",
    VertexStepSize=5.0*I3Units.m,
    Minimizer="minuit",
)

# This module will check that:
# * ALL physics frames have a parazoll and a parazollFitParams object
# * if NDF<=0, both fit_status and pbfStatus should be nonzero
# Failure to comply will result in runtime exceptions
tray.AddModule(zollkontrolle, Streams=[icetray.I3Frame.Physics])

tray.Execute(23)

