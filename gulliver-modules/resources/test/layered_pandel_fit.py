#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""Test script for Gulliver

This script reads simple-muon-generated data and does two
reconstructions:

    1. ``I3LineFit`` from `linefit`: old first-guess method, superseded
       by `improvedLinefit`
    2. ``I3SimpleFitter`` from `gulliver_modules`: SRT-cleaned offline
       pulses, ipdf convoluted SPE1st Pandel, LineFit seed

It uses the `fortytwo` functionality to check that the current results
are equal to (or compatible with) the reference results.

"""
import os

import numpy

import icecube
import icecube.icetray
import icecube.dataclasses
import icecube.dataio
import icecube.linefit
import icecube.gulliver
import icecube.gulliver_modules
import icecube.gulliver_modules.fortytwo
import icecube.lilliput
from I3Tray import I3Tray

icecube.icetray.logging.set_level_for_unit("I3FortyTwo", "INFO")

if "I3_TESTDATA" in os.environ:
    testdata = os.environ["I3_TESTDATA"]
else:
    raise IOError("Cannot find test data file, please define either "
                  "I3_TESTDATA or I3_PORTS")

gcd = os.path.join(
    testdata, "sim", "GeoCalibDetectorStatus_IC86.55697_corrected_V2.i3.gz")
qpqpqpqp = os.path.join(
    testdata, "sim", "Level2_IC86.2011_corsika.010281.001664.00.i3.bz2")
icefile_mie = os.path.join(
    os.environ["I3_BUILD"], "phys-services", "resources", "ice_tables",
    "Ice_table.mie.i3coords.cos090.08Apr2011.txt")
icefile_wham = os.path.join(
    os.environ["I3_BUILD"], "phys-services", "resources", "ice_tables",
    "Ice_table.wham.i3coords.cos080.11jul2011.txt")

nevents = 10
pulses = "SRTOfflinePulses"

# ----IceTray------------------------------------------------------------------
tray = I3Tray()

# ----Services for Gulliver reconstruction-------------------------------------
tray.AddService("I3SimpleParametrizationFactory", "xyzza",
                StepX=20.*icecube.icetray.I3Units.m,
                StepY=20.*icecube.icetray.I3Units.m,
                StepZ=20.*icecube.icetray.I3Units.m,
                StepZenith=0.1*icecube.icetray.I3Units.radian,
                StepAzimuth=0.2*icecube.icetray.I3Units.radian,
                BoundsX=[-2000.*icecube.icetray.I3Units.m,
                         2000.*icecube.icetray.I3Units.m],
                BoundsY=[-2000.*icecube.icetray.I3Units.m,
                         2000.*icecube.icetray.I3Units.m],
                BoundsZ=[-2000.*icecube.icetray.I3Units.m,
                         2000.*icecube.icetray.I3Units.m])

tray.AddService("I3GulliverMinuitFactory", "minuit",
                Algorithm="SIMPLEX")

# Usual bulk Pandel, layered Pandel with two different ice files
for icename, icefile in zip(["bulk", "mie", "wham"],
                            ["", icefile_mie, icefile_wham]):
    tray.AddService("I3GulliverIPDFPandelFactory", "spe1st_"+icename,
                    InputReadout=pulses,
                    Likelihood="SPE1st",
                    PEProb="GaussConvoluted",
                    IceModel=2,
                    IceFile=icefile,
                    NoiseProbability=1000.*icecube.icetray.I3Units.hertz,
                    JitterTime=4.*icecube.icetray.I3Units.ns)

tray.AddService("I3BasicSeedServiceFactory", "linefitseed",
                InputReadout=pulses,
                TimeShiftType="TChargeFraction",
                FirstGuesses=["linefit"])

# ----Modules------------------------------------------------------------------
tray.AddModule("I3Reader", "reader",
               FileNameList=[gcd, qpqpqpqp])

tray.AddModule(icecube.gulliver_modules.fortytwo.MultiplicityCutModule,
               "enough_doms_and_strings",
               pulses=pulses,
               nchmin=12,
               nstrmin=3)

tray.AddModule("I3LineFit", "linefit",
               Name="linefit",
               InputRecoPulses=pulses,
               AmpWeightPower=1.)

particles = ["linefit"]
lhfitparams = []


# def print_direction(frame, fit="aap"):
#     particle = frame[fit]
#
#     icetray.logging.log_info(
#         "Fit = %s, status = %s, zen = %f, azi = %f."
#         % (fit, particle.fit_status_string,
#            particle.dir.zenith/icecube.icetray.I3Units.degree,
#            particle.dir.azimuth/icecube.icetray.I3Units.degree),
#         unit="print_direction")
#
#     return True


class TrackCheck(icecube.icetray.I3ConditionalModule):
    def __init__(self, ctx):
        super(TrackCheck, self).__init__(ctx)
        self.AddOutBox("OutBox")
        self.AddParameter("fit", "Name of reconstructed track to check", "")

        self.nbad = 0
        self.ncoinc = 0
        self.psi = []

    def Configure(self):
        self.trackname = self.GetParameter("fit")
        return

    def Physics(self, frame):
        mc = frame["I3MCTree"]
        if len(mc.primaries) == 1:
            primary = mc.primaries[0]
            track = frame[self.trackname]

            if track.fit_status == icecube.dataclasses.I3Particle.OK:
                self.psi.append(numpy.arccos(track.dir*primary.dir) /
                                icecube.icetray.I3Units.degree)
            else:
                self.nbad += 1
        else:
            self.ncoinc += 1

        self.PushFrame(frame)
        return

    def Finish(self):
        message = ["\n*********** %s ************" % self.trackname,
                   "nbad = %d" % self.nbad,
                   "ncoinc = %d" % self.ncoinc
                   ]

        if len(self.psi) > 0:
            message.append("ngood = %d" % len(self.psi))

            apsi = numpy.array(self.psi)
            message.append("psi average = %f, median = %f"
                           % (numpy.mean(apsi), numpy.median(apsi)))

        icecube.icetray.logging.log_info("\n".join(message), unit="TrackCheck")
        return


for icename in ["bulk", "mie", "wham"]:
    fitname = "spefit_" + icename

    tray.AddModule("I3SimpleFitter",
                   OutputName=fitname,
                   SeedService="linefitseed",
                   Parametrization="xyzza",
                   LogLikelihood="spe1st_"+icename,
                   Minimizer="minuit")

    # tray.Add(dirprint,fit=fitname)
    tray.Add(TrackCheck, fit=fitname)

    particles.append(fitname)
    lhfitparams.append(fitname+"FitParams")

# Create/update reference data.
# pc = icecube.gulliver_modules.fortytwo.I3ParticleChecker(particles)
# lhfpc = icecube.gulliver_modules.fortytwo.I3LogLikelihoodFitParamsChecker(
#     lhfitparams)

#
# tray.AddModule(icecube.gulliver_modules.fortytwo.I3FortyTwo, "test_suite",
#                checklist=[pc, lhfpc])

# Run test module.
tray.AddModule(icecube.gulliver_modules.fortytwo.I3FortyTwo, "test_suite")



# ----Execute------------------------------------------------------------------
tray.Execute(3 + 2*nevents)

