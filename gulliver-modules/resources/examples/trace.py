#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""Illustrate the tracing functionality for debugging.

Reconstructs 10 events and makes two plots of this:

1. Six plots of parameter (and logl) values versus function call number.
2. An aitoff projection of the evolution of the direction coordinates
   during the fit.
"""

import os

import matplotlib
import numpy

import icecube
import icecube.icetray
import icecube.dataclasses
import icecube.dataio
import icecube.lilliput
import icecube.lilliput.segments
import icecube.linefit

from I3Tray import I3Tray

matplotlib.use("Agg")

testfile = os.path.expandvars(
    "$I3_TESTDATA/event-viewer/Level3aGCD_IC79_EEData_Run00115990.i3")


def main(inputfiles=[testfile], outputdir=".", pulses="SRTOfflinePulses",
         llh="SPE1st", nevents=10):
    tray = I3Tray()

    tray.context["I3FileStager"] = icecube.dataio.get_stagers()
    tray.Add("I3Reader",
             filenamelist=inputfiles)

    # Run LineFit as first guess.
    tray.Add("I3LineFit",
             Name="linefit",
             InputRecoPulses=pulses,
             AmpWeightPower=1.0)

    # Minimize.
    tray.Add(icecube.lilliput.segments.I3SinglePandelFitter, "PandelFit",
             fitname="PandelFit",
             pulses=pulses,
             domllh=llh,
             seeds=["linefit"])
    tray.SetParameter("PandelFit", "TraceMode", "Single")

    tray.Add("I3EventCounter",
             NEvents=nevents,
             CounterStep=1)

    traceplotter.nevent = 0
    tray.Add(traceplotter, output=outputdir)

    tray.Execute()
    


def traceplotter(frame, output="."):
    import matplotlib.pyplot
    trace = frame["PandelFit_TRACE"]
    if trace is None:
        print("No trace found. Skip event.")
        return True
    traceplotter.nevent += 1

    nevent = traceplotter.nevent
    count = range(len(trace)/6)
    print("%d. got trace with %d entries (%d function calls)."
          % (nevent, len(trace), len(count)))

    fig = matplotlib.pyplot.figure(figsize=(6, 10))
    fig.set_tight_layout(True)
    fig.suptitle("%d function calls" % len(count))

    labels = ["x", "y", "z", "zenith", "azimuth", "logl"]
    for i in range(6):
        ax = fig.add_subplot(6, 1, i+1)
        values = numpy.array(trace)[i::6]
        ax.plot(count, values, label=labels[i])
        ax.legend()

    figname = os.path.join(output, "alltrace_%03d.png" % nevent)
    fig.savefig(figname)
    matplotlib.pyplot.close(fig)
    print("Saved figure %s." % figname)

    # Force zenith and azimuth in range.
    zenith = numpy.array(trace)[3::6]
    azimuth = numpy.array(trace)[4::6]

    theta = numpy.arccos(numpy.cos(zenith)) - numpy.pi/2
    phi = numpy.arctan2(numpy.sin(zenith)*numpy.sin(azimuth),
                        numpy.sin(zenith)*numpy.cos(azimuth))

    # Plot aitoff (weird: you give values in radian, it plots in degrees).
    fig = matplotlib.pyplot.figure()
    fig.set_tight_layout(True)

    ax = fig.add_subplot(111, projection='aitoff')
    ax.plot(phi, theta)

    figname = os.path.join(output, "dirtrace_%03d.png" % nevent)
    fig.savefig(figname)
    matplotlib.pyplot.close(fig)
    print("Saved figure %s" % figname)

    print("DONE (event %d)" % nevent)
    return True


if __name__ == "__main__":
    import argparse
    parser = argparse.ArgumentParser(description=__doc__)

    parser.add_argument("inputfiles",
                        nargs="*",
                        default=[testfile],
                        help="input i3-files",
                        metavar="FILE")
    parser.add_argument("--outputdir",
                        default=".",
                        help="output directory for plots [%(default)s]",
                        metavar="DIR")
    parser.add_argument("--pulses",
                        default="SRTOfflinePulses",
                        help="pulses to use for the fit [%(default)s]")
    parser.add_argument("--llh",
                        default="SPE1st",
                        choices=("SPE1st", "MPE"),
                        help="type of likelihood to use [%(default)s]")
    parser.add_argument("--nevents",
                        default=10,
                        type=int,
                        help="number of events to reconstruct [%(default)s]",
                        metavar="NUM")

    args = parser.parse_args()
    main(args.inputfiles, args.outputdir, args.pulses, args.llh, args.nevents)
