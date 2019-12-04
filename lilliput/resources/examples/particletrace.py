#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""This script illustrates the use of the `ParticleTrace` functionality
of ``I3SimpleParametrization`` in `lilliput`.
"""
import os

import numpy
import matplotlib

import icecube
import icecube.icetray
import icecube.dataclasses
import icecube.dataio
import icecube.phys_services
import icecube.linefit
import icecube.gulliver
import icecube.gulliver_modules
import icecube.gulliver_modules.iceopt
import icecube.lilliput
from I3Tray import I3Tray

matplotlib.use("Agg")

filename = os.path.expandvars(
    "$I3_TESTDATA/topological-splitter/Topological_Splitter_testcase.i3.bz2")

scriptname = "simplefit"
pulsesname = "SRTOfflinePulses"

opts = icecube.gulliver_modules.iceopt.IceTrayScriptOptions(
    name=scriptname, definput=filename, defverbose=True, defnevents=10)

# ----IceTray------------------------------------------------------------------
tray = I3Tray()

# ----Services to do Gulliver reconstruction----------------------------------
tray.Add("I3SimpleParametrizationFactory", "simpletrack",
         StepX=20.*icecube.icetray.I3Units.m,
         StepY=20*icecube.icetray.I3Units.m,
         StepZ=20*icecube.icetray.I3Units.m,
         StepZenith=0.1*icecube.icetray.I3Units.radian,
         StepAzimuth=0.2*icecube.icetray.I3Units.radian,
         BoundsX=[-2000.*icecube.icetray.I3Units.m,
                  2000.*icecube.icetray.I3Units.m],
         BoundsY=[-2000.*icecube.icetray.I3Units.m,
                  2000.*icecube.icetray.I3Units.m],
         BoundsZ=[-2000.*icecube.icetray.I3Units.m,
                  2000*icecube.icetray.I3Units.m],
         ParticleTrace=True)

tray.Add("I3GulliverMinuitFactory", "minuit",
         Algorithm="SIMPLEX")

tray.Add("I3GulliverIPDFPandelFactory", "ipdfGCpandel",
         InputReadout=pulsesname,
         Likelihood="SPEqAll",
         PEProb="GaussConvoluted",
         IceModel=2,
         AbsorptionLength=98.*icecube.icetray.I3Units.m,
         NoiseProbability=100.*icecube.icetray.I3Units.hertz,
         JitterTime=15.*icecube.icetray.I3Units.ns)

tray.Add("I3BasicSeedServiceFactory", "seed",
         InputReadout=pulsesname,
         TimeShiftType="TFirst",
         FirstGuesses=["linefit"])

# ----Modules------------------------------------------------------------------
tray.Add("I3Reader",
         FilenameList=opts.infiles)


def TT0BigEnough(frame):
    eh = frame["I3EventHeader"]
    if eh.sub_event_stream != "OriginalTTrigger":
        return False
    if eh.sub_event_id != 0:
        return False
    pulses = icecube.dataclasses.I3RecoPulseSeriesMap.from_frame(
        frame, pulsesname)
    nch = len(pulses)
    nstr = len(set([dom.string for dom in pulses.keys()]))
    return nch >= 10 and nstr >= 3


tray.Add("I3LineFit",
         If=TT0BigEnough,
         Name="linefit",
         InputRecoPulses=pulsesname,
         AmpWeightPower=1.)

tray.Add("I3SimpleFitter",
         If=TT0BigEnough,
         SeedService="seed",
         Parametrization="simpletrack",
         LogLikelihood="ipdfGCpandel",
         Minimizer="minuit",
         OutputName="llhfit")

tray.Add("Dump")


class TracePlot(icecube.icetray.I3ConditionalModule):
    def __init__(self, context):
        icecube.icetray.I3ConditionalModule.__init__(self, context)

        self.AddOutBox("OutBox")
        self.AddParameter("trackname", "Name of track")
        self.AddParameter("parname", "Name of parametrization service")
        self.AddParameter("nevents", "Number of events")

        self.nr = 0

    def Configure(self):
        self.trackname = self.GetParameter("trackname")
        self.parname = self.GetParameter("parname")
        self.nevents = self.GetParameter("nevents")
        self.tracename = self.trackname + "_" + self.parname
        return

    def Physics(self, frame):
        import matplotlib.pyplot
        print("Running on event %s." % frame["I3EventHeader"])
        self.nr += 1

        linefit = frame["linefit"]
        particlevect = frame[self.tracename]

        psilist = []
        zendifflist = []
        azidifflist = []

        for p in particlevect:
            psilist.append(numpy.rad2deg(
                icecube.phys_services.I3Calculator.angle(linefit, p)))
            zendifflist.append(linefit.dir.zenith-p.dir.zenith)
            azidifflist.append(linefit.dir.azimuth-p.dir.azimuth)

        zendiffs = numpy.rad2deg(numpy.array(zendifflist))
        ad = numpy.array(azidifflist)
        azidiffs = numpy.rad2deg(numpy.arctan2(numpy.sin(ad), numpy.cos(ad)))

        fig = matplotlib.pyplot.figure()
        ax = fig.add_subplot(111)

        ax.plot(psilist, label="psi [deg]")
        ax.plot(zendiffs, label="$\Delta$zenith [deg]")
        ax.plot(azidiffs, label="$\Delta$azimuth [deg]")

        ax.set_xlabel("fit step")
        ax.set_ylabel("angular difference [deg]")
        ax.legend(loc="lower right")

        fig.savefig("particletrace_%s_%04d.png" % (self.trackname, self.nr))
        matplotlib.pyplot.close(fig)

        self.PushFrame(frame)
        return


tray.Add(TracePlot,
         If=TT0BigEnough,
         trackname="llhfit",
         parname="simpletrack",
         nevents=opts.nevents)

# ----Execute------------------------------------------------------------------
if opts.nevents > 0:
    tray.Execute(opts.nevents)
else:
    tray.Execute()
