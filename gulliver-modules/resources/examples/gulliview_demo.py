#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""Run LineFit and a Pandel likelihood fit; plot the likelihood space
around the minimum.
"""

import os

import icecube
import icecube.icetray
import icecube.dataclasses
import icecube.dataio
import icecube.gulliver_modules
import icecube.gulliver_modules.gulliview
import icecube.lilliput
import icecube.lilliput.segments
import icecube.linefit

from I3Tray import I3Tray

testfile = os.path.expandvars(
    "$I3_TESTDATA/event-viewer/Level3aGCD_IC79_EEData_Run00115990.i3")


def main(inputfiles=[testfile], outputfile="./gulliview",
         pulses="SRTOfflinePulses", llh="SPE1st"):
    tray = I3Tray()

    tray.context["I3FileStager"] = icecube.dataio.get_stagers()
    tray.Add("I3Reader",
             filenamelist=inputfiles)

    # Run LineFit as first guess.
    tray.Add("I3LineFit",
             Name="linefit",
             InputRecoPulses=pulses,
             AmpWeightPower=1.0)

    # Minimize first.
    mininame, paraname, llhname, seedname, name = tray.Add(
        icecube.lilliput.segments.I3SinglePandelFitter,
        fitname="PandelFit",
        pulses=pulses,
        domllh=llh,
        seeds=["linefit"])

    # Plot the likelihood space around the minimum.
    seeder = icecube.lilliput.segments.add_seed_service(
        tray, pulses, ["PandelFit"])

    tray.Add(icecube.gulliver_modules.gulliview.GulliView,
             SeedService=seeder,
             Parametrization=paraname,
             LogLikelihood=llhname,
             Filename=outputfile)

    tray.Execute()
    


if __name__ == "__main__":
    import argparse
    parser = argparse.ArgumentParser(description=__doc__)

    parser.add_argument("inputfiles",
                        nargs="*",
                        default=[testfile],
                        help="input i3-files",
                        metavar="FILE")
    parser.add_argument("--outputfile",
                        default="./gulliview",
                        help="filname base for plots [%(default)s]")
    parser.add_argument("--pulses",
                        default="SRTOfflinePulses",
                        help="pulses to use for the fit [%(default)s]")
    parser.add_argument("--llh",
                        default="SPE1st",
                        choices=("SPE1st", "MPE"),
                        help="type of likelihood to use [%(default)s]")
    parser.add_argument("--interactive",
                        action="store_true",
                        help="show plots instead of saving them")

    args = parser.parse_args()

    if args.interactive:
        args.outputfile = None

    main(args.inputfiles, args.outputfile, args.pulses, args.llh)
