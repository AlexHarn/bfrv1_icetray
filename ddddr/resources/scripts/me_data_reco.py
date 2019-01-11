#!/usr/bin/env python
import sys, os, math
from os.path import expandvars

from optparse import OptionParser

from I3Tray import *
from icecube import icetray, dataclasses, dataio, ddddr

# take some test data that is already there
testdata = os.path.join(expandvars("$I3_TESTDATA"), 'icetop')
GCD = os.path.join(expandvars("$I3_TESTDATA"), 'sim', 'GeoCalibDetectorStatus_IC86.55697_corrected_V2.i3.gz')
L2FILE = os.path.join(testdata, 'Level3_IC86.2012_data_Run00120244_Part00_IT73_IT_coinc.i3.tgz')

########### Options ############
usage = "usage: %prog [options] inputfiles"
parser = OptionParser(usage)
parser.add_option("-o", "--outputfile", default=None,
                  dest="OUTFILE", help="Write output to OUTFILE (.i3{.bz2} format)")
parser.add_option("-p", "--pulse-map", default="InIcePulses",
        dest="PULSEMAP", help="PulseMap to use, default: %default")
parser.add_option("-t", "--seedtrack", default="Laputop",
                  dest="SEEDTRACKNAME", help="Seed Track to use, default: %default")
parser.add_option("-s", "--slant-bin-size", default=50, type="float",
                  dest="SLANTBINSIZE", help="Bin size for slant depth, default: %default")
parser.add_option("-g", "--gcd-file", default=GCD,
                  dest="GCDFILE")
parser.add_option("--max-impact", default=100,
                  dest="MAXIMPACT", type="float",
                  help="Max impact factor for DOMs to be considered for energy reconstruction, default: %default")
parser.add_option("--save-dom-results", action="store_true", default=False,
                  dest='SAVE_DOM_RESULTS', help='Save the energy loss of all DOMs and slant depth bins.')

# parse command line options, args are input files
(options, args) = parser.parse_args()

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


icetray.logging.set_level(icetray.logging.I3LogLevel.LOG_DEBUG)
# define tray and modules
tray = I3Tray()
tray.AddModule('I3Reader', 'reader', FilenameList=filenamelist)

tray.AddModule('I3MuonEnergy', 'me',
               BinWidth      = options.SLANTBINSIZE,
               InputPulses   = options.PULSEMAP,
               MaxImpact     = options.MAXIMPACT,
               Seed          = options.SEEDTRACKNAME,
               SaveDomResults = options.SAVE_DOM_RESULTS,
               Prefix         = 'D4R'
              )

if options.OUTFILE is not None:
    tray.AddModule('I3Writer', 'write',
                   FileName = options.OUTFILE,
                   Streams  = [icetray.I3Frame.TrayInfo, icetray.I3Frame.Geometry, icetray.I3Frame.DAQ, icetray.I3Frame.Physics]
              )


tray.Execute()


