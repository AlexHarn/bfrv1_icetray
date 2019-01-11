#!/usr/bin/env python
"""
Run snow correction on selected pulses.

Note that there are many caveats with this approach.
A perfect snow correction on data does not exist. Signals lost
due to snow cannot be 'invented' back into the frame, and since 
snow correction is ignorant to the source particle, it smears
and shifts the muon peak at 1 VEM in data.

Snow corrected pulses mainly exist to make transparent what the
snow correction service in Laputop does, and to allow for a
direct comparison of measured points with the fitted LDF in
steamshovel. They can also be used to re-compute the chi2 for
the LDF fit.

Snow correction requires a hypothesis about the true shower,
since it needs to estimate the ratio of em to muon signal in
each tank. That's why a Laputop reconstruction is required as
input.

WARNING: The snow correction uses some parameters which change
by year. You need to set these by hand, there is no guarantee
that the same parameters were used in Laputop that you use here.
"""

import os
import sys
import argparse

from I3Tray import *
from icecube.icetray import I3Frame
from icecube import icetray, toprec, dataio
from icecube.dataclasses import I3RecoPulseSeriesMap, I3RecoPulseSeries, I3RecoPulse

def SnowCorrection(frame, pulses, reconstruction, snow_correction):
    geo = frame["I3Geometry"]
    if reconstruction not in frame:
        return
    part = frame[reconstruction]
    params = frame[reconstruction + "Params"]

    psm_nosnow = I3RecoPulseSeriesMap.from_frame(frame, pulses)
    psm = I3RecoPulseSeriesMap()
    for omkey, series_nosnow in psm_nosnow.items():
        for tank in geo.stationgeo[omkey.string]:
            if omkey in tank.omkey_list:
                pos = tank.position
                snow_depth = tank.snowheight
                att = snow_correction.attenuation_factor(pos, snow_depth, part, params)
                break
        series = I3RecoPulseSeries()
        for pulse_nosnow in series_nosnow:
            pulse = I3RecoPulse(pulse_nosnow)
            pulse.charge = pulse_nosnow.charge / att
            series.append(pulse)
        psm[omkey] = series
    frame[pulses + "_SnowCorrected"] = psm

## Set logging
## Uncomment me for gooey details!
#icetray.I3Logger.global_logger = icetray.I3PrintfLogger()
#icetray.set_log_level(icetray.I3LogLevel.LOG_DEBUG)
#icetray.set_log_level_for_unit('Laputop', icetray.I3LogLevel.LOG_DEBUG)

### Input and Output
parser = argparse.ArgumentParser()
parser.add_argument("input", nargs="+",
                    help="An ordered list of files with GCD and Q and P frames")
parser.add_argument("-o", required=True,
                    help="Output filename")
parser.add_argument("-p", default="CleanedHLCTankPulses",
                    help="Key with the pulses to correct")
parser.add_argument("-r", default="Laputop",
                    help="Key with the reconstruction to use for snow correction")
parser.add_argument("-s", choices=("simple", "BORS"), default="simple",
                    help="which snow serive to use")

args = parser.parse_args()

if args.s == "simple":
    lambd = 2.1 # effective attenuation coefficient
    snow_correction = toprec.I3SimpleSnowCorrectionService("snow_correction", lambd)
else:
    snow_correction = toprec.I3BORSSnowCorrectionService("snow_correction")

tray = I3Tray()

tray.AddModule("I3Reader", "reader",
    FileNameList=args.input)

tray.AddModule(SnowCorrection, "snow_corrector",
    pulses=args.p,
    reconstruction=args.r,
    snow_correction=snow_correction,
    Streams=[I3Frame.Physics])

tray.AddModule('I3Writer', 'writer',
               Filename=args.o,
               Streams=[I3Frame.DAQ, I3Frame.Physics])


 
tray.Execute()

