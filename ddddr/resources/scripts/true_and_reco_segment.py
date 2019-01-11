#!/usr/bin/env python
import sys, os, math
from os.path import expandvars

from optparse import OptionParser

from I3Tray import *
from icecube import icetray, dataclasses, dataio, ddddr
from icecube.ddddr import TrueAndRecoMuonEnergy

# take some test data that is already there
testdata = expandvars("$I3_TESTDATA")
GCD = os.path.join(testdata, 'sim', 'GeoCalibDetectorStatus_IC86.55697_corrected_V2.i3.gz')
L2FILE = os.path.join(testdata, 'sim', 'Level2_IC86.2011_corsika.010281.001664.00.i3.bz2')

########### Options ############
usage = "usage: %prog [options] inputfiles"
parser = OptionParser(usage)
parser.add_option("-o", "--outputfile", default=None,
                  dest="OUTFILE", help="Write output to OUTFILE (.i3{.bz2} format), default: %default")
parser.add_option("-p", "--pulse-map", default="TWOfflinePulsesHLC",
                  dest="PULSEMAP", help="PulseMap to use, default: % default")
parser.add_option("--max-impact", default=100,
                  dest="MAXIMPACT", type="float",
                  help="Max impact factor for DOMs to be considered for energy reconstruction, default: %default")
parser.add_option("--mc-tree-name", default="I3MCTree",
                  dest="MCTREENAME", help="Name of I3MCTREE, default: %default")
parser.add_option("--mmc-track-list", default="MMCTrackList",
                  dest="MMCTRACKLIST", help="Name of the MMCTrackList, default: %default")
parser.add_option("-s", "--slant-bin-size", default=50, type="float",
                  dest="SLANTBINSIZE", help="Bin size for slant depth, default: %default")
parser.add_option("-g", "--gcd-file", default=GCD,
                  dest="GCDFILE")

# parse command line options, args are input files
(options, args) = parser.parse_args()
#if len(args) < 1:
#    problem = "No input files specified"
#    parser.error(problem)

filenamelist = []
if options.GCDFILE:
    filenamelist.append(options.GCDFILE)
filenamelist.extend(args)
if len(filenamelist) == 1:
    if os.path.exists(L2FILE):
        filenamelist.append(L2FILE)
    else:
        icetray.logging.log_warn("No input file specified and "+\
                                 "default test file %s not available. Quitting now."%L2FILE)
        sys.exit()

# define tray and modules
tray = I3Tray()
tray.AddModule('I3Reader', 'reader', FilenameList=filenamelist)

tray.AddSegment(TrueAndRecoMuonEnergy, 'TrueAndReco',
                BinWidth     = options.SLANTBINSIZE,
                I3MCTree     = options.MCTREENAME,
                MMCTrackList = options.MMCTRACKLIST,
                InputPulses  = options.PULSEMAP,
                MaxImpact    = options.MAXIMPACT,
                Prefix       = 'I3MuonEnergy_',
                SaveDomResults = True,
               )
                    
if options.OUTFILE is not None:
    tray.AddModule('I3Writer', 'write',
                   FileName = options.OUTFILE,
                   Streams  = [icetray.I3Frame.DAQ, icetray.I3Frame.Physics])


tray.Execute(100)


