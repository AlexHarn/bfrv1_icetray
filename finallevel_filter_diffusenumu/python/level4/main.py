#!/usr/bin/env python

from I3Tray import *
from icecube import dataio, dataclasses, icetray
from segment import IC12L4

from argparse import ArgumentParser
import glob
from ConfigParser import ConfigParser

def main():

    icetray.logging.set_level("WARN")
    parser=ArgumentParser()

    parser.add_argument("-i",
                        "--input",
                        dest="infile",
                        required=True,
                        nargs="+",
                        help="Input i3 file(s)")

    parser.add_argument("-g",
                        "--gcd",
                        dest="gcdfile",
                        required=True,
                        help="GCD file")

    parser.add_argument("-o",
                        "--output",
                        dest="outfile",
                        required=True,
                        help="Main i3 output file")

    parser.add_argument("--isNuMu",
                        action="store_true",
                        dest="is_numu",
                        help="Remove bug events in nugen simulation")

    args = parser.parse_args()

    # Load table paths from cfg file
    parser = ConfigParser()
    parser.readfp(open("../paths.cfg"))
    paths = dict(parser.items("main"))
    tray = I3Tray()

    tray.Add(IC12L4,
             gcdfile=args.gcdfile,
             infiles=args.infile,
             table_paths=paths,
             is_numu=args.is_numu)

    # write output
    tray.AddModule("I3Writer",
        Filename = args.outfile,
        DropOrphanStreams=[icetray.I3Frame.DAQ],
        Streams=[icetray.I3Frame.DAQ, icetray.I3Frame.Physics])

    tray.Execute()

    usagemap = tray.Usage()

    for mod in usagemap:
        print(mod)

if __name__=="__main__":
    main()
