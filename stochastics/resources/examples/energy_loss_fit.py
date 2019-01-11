#!/usr/bin/env python

import os, sys

from argparse import ArgumentParser
parser = ArgumentParser(usage='%s [options] -o <filename>.i3[.bz2|.gz] {i3 file list}'%os.path.basename(sys.argv[0]))
parser.add_argument('inputFiles',help="Input file(s)",nargs="*")
parser.add_argument("-o", "--output", action="store", type=str, dest="output", help="Output filename", metavar="<filename>", default='energy_losses.i3')
parser.add_argument("-l", "--losses", action="store", type=str, dest="losses", help="vector<I3Particle> containing the energy losses", metavar="<loss profile>")
parser.add_argument("-n", action="store", type=int, dest="n", help="number of frames to process")
parser.add_argument("-m", action="store_true", dest="minimal", help="If specified, only some objects are saved (for testing)", default=False)
parser.add_argument("--true", action="store_true", help="If specified, the existing Stoch_Reco is just renamed (for testing)", default=False)
(args) = parser.parse_args()

ok = True
if not len(args.inputFiles)>0:
    print("Error: Need to specify input files")
    ok = False

if not args.losses and not args.generate:
    print("Error: Need to specify vector of I3Particle containing energy losses unless you want to generate a test reference file.")
    ok = False

if not ok:
    parser.print_help()
    parser.exit(1)

from I3Tray import *
from icecube import dataio, icetray, stochastics, dataclasses

tray=I3Tray()

tray.AddModule("I3Reader","my_reader",FilenameList=args.inputFiles)

if args.true:
    # For creating the ref. file:
    # Stoch_Reco is in CR L3, Stoch_Reco_red not. So we rename "Stoch_Reco" from CR L3 to "Stoch_Reco_TEST" and generate "Stoch_Reco_red_TEST"
    tray.AddModule('I3Stochastics','stoch_standard',
                   A_Param = 0,
                   B_Param = 5,
                   C_Param = 0.8,
                   FreeParams = 1,
                   InputParticleVector=args.losses,
                   Minimizer = 'MIGRAD',
                   OutputName = 'Stoch_Reco_TEST',
                   OutputName_red = 'Stoch_Reco_red_TEST',
                   Verbose = False,
                   SelectionType = 'Type2')
    
    tray.AddModule("Delete",Keys=["Stoch_Reco_TEST"])
    tray.AddModule("Rename",
                   Keys=['Stoch_Reco', 'Stoch_Reco_TEST'])


else:
    # Parameters are the standard ones used in the cosmic ray IC79 composition analysis.
    tray.AddModule('I3Stochastics','stoch_standard',
                   A_Param = 0,
                   B_Param = 5,
                   C_Param = 0.8,
                   FreeParams = 1,
                   InputParticleVector=args.losses,
                   Minimizer = 'MIGRAD',
                   OutputName = 'Stoch_Reco_TEST',
                   OutputName_red = 'Stoch_Reco_red_TEST',
                   Verbose = False,
                   SelectionType = 'Type2')

if args.minimal: tray.AddModule('Keep', Keys=['Stoch_Reco_TEST', 'Stoch_Reco_red_TEST',
                                              'Stoch_Reco', 'Stoch_Reco_red'])
tray.AddModule('I3Writer',
               Filename = args.output,
               Streams  = [icetray.I3Frame.Physics])


if args.n:
    tray.Execute(args.n)
else:
    tray.Execute()


