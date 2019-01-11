import sys
import os.path
from argparse import ArgumentParser
from icecube import icetray, dataio, dataclasses, icetop_Level3_scripts
from I3Tray import I3Tray

parser = ArgumentParser(usage='%s [arguments] -o <filename>.i3[.bz2|.gz] {N}'%os.path.basename(sys.argv[0]))
parser.add_argument("--histos", action="store", type=str, dest="histos", help="Histograms file name. Needs to be pickle file", metavar="BASENAME")
parser.add_argument("--livetime", action="store", type=str, dest="livetime", help="Livetime file name, only needed for data. Needs to be pickle file", metavar="BASENAME")
parser.add_argument("-n", action="store", type=int, dest="n", help="number of frames to process")
parser.add_argument("-m","--isMC", action="store_true",dest="isMC", help= "Is this data or MC?")
parser.add_argument('inputFiles',help="Input file(s)",type=str,nargs="*")

(args) = parser.parse_args()

if not (len(args.inputFiles)>0 and args.histos):
    parser.print_help()
    exit(0)

if not args.isMC:
    if not args.livetime:
        parser.print_help()
        exit(0)
    else:
        if not args.livetime[-7:]==".pickle":
            icetray.logging.log_fatal("livetime parameter needs to be a .pickle file.")

if not args.histos[-7:]==".pickle":
    icetray.logging.log_fatal("histos parameter needs to be a .pickle file.")

tray=I3Tray()
tray.AddModule("I3Reader","reader",FilenameList=args.inputFiles)

from icecube.production_histograms import ProductionHistogramModule

tray.AddSegment(icetop_Level3_scripts.segments.MakeHistograms, "makeHistos", OutputFilename=args.histos, isMC=args.isMC)

if not args.isMC:
    tray.Add(ProductionHistogramModule, "LivetimeHistogram",
             Histograms = [icetop_Level3_scripts.histograms.Livetime],
             OutputFilename = args.livetime
             )

tray.Execute()

