#!/usr/bin/env python

from I3Tray import *
from icecube import dataio, dataclasses, icetray
import segments
from argparse import ArgumentParser
import glob

def main():
    icetray.logging.set_level("WARN")

    parser=ArgumentParser()

    parser.add_argument("-i",
                        "--input",
                        required=True,
                        dest="infile",
                        nargs="+",
                        help="Input i3 file(s)")

    parser.add_argument("-o",
                        "--output",
                        required=True,
                        dest="outfile",
                        help="Output file")

    args = parser.parse_args()

    parser = ConfigParser()
    parser.readfp(open(os.path.join(os.path.expandvars('$I3_BUILD'),
                                    'lib/icecube/finallevel_filter_diffusenumu',
                                    'paths.cfg')))
    paths = dict(parser.items("main"))

    tray = I3Tray()

    tray.Add("I3Reader", FilenameList = args.infile)
    tray.Add(segments.Scorer, "doLevel5",
        CutFunc=segments.CutFunc,
        CascCut=0.4)

    tray.Add(segments.millipede_segment, "MillipedeLosses",
             paths["cascadeampsplinepath"],
             paths["cascadeprobsplinepath"])

    # write output
    tray.AddModule("I3Writer",
        Filename = args.outfile,
        DropOrphanStreams=[icetray.I3Frame.DAQ],
        Streams=[icetray.I3Frame.DAQ, icetray.I3Frame.Physics])
    tray.Execute()

    usagemap = tray.Usage()
    for mod in usagemap:
        print(mod)

if (__name__)=="__main__":
    main()
