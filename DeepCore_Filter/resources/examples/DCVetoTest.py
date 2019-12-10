#!/usr/bin/env python
#----------------------------------------------------------------------
# The following code is an end-to-end test of the DeepCore_Filter
# modules through the tray segment. The I3_TESTDATA is used as standard
# testing data.
#
# $Id$
# $Date$ $Revision$ $Author$
#----------------------------------------------------------------------
import sys
from os.path import expandvars
from optparse import OptionParser

from icecube import icetray, dataclasses, dataio, DomTools, DeepCore_Filter, WaveCalibrator, wavedeform
from icecube.icetray import I3Units
from I3Tray import *

tray = I3Tray()

parser = OptionParser()

parser.add_option( "-g", "--geo", action="store", type="string", dest="geoFile",
                   metavar="<geo file>", default=expandvars("$I3_TESTDATA/GCD/GeoCalibDetectorStatus_2013.56429_V1.i3.gz"),
                   help="Name of GCD file")

parser.add_option( "-d", "--data_file",
                   dest = "data_file",
                   default = expandvars("$I3_TESTDATA/sim/Level2_IC86.2011_corsika.010281.001664.00.i3.bz2"),
                   help = "Test .i3.gz file")

(options, args) = parser.parse_args()

tray.AddModule("I3Reader", "reader") (
    ("filenamelist", [options.geoFile, options.data_file])
    )

#tray.AddModule("I3WaveCalibrator", "wavecal")

#tray.AddModule("I3Wavedeform", "wavedeform")

tray.AddSegment(DeepCore_Filter.RunFilter, "DCVetoTest", 
                DetectorConfig="IC86",
                pulses = "OfflinePulses")

tray.AddSegment(DeepCore_Filter.DeepCoreTimeVeto, "DCTimeVetoTest", 
                DetectorConfig="IC86",
                pulses = "OfflinePulses")


npassed_veto = 0
npassed_timeveto = 0
def counter(frame):
    global npassed_veto, npassed_timeveto
    if not frame.Has("DCVeto"): return 
    npassed_veto     += frame["DCVeto"].value
    npassed_timeveto += frame["DCTimeVeto"].value

tray.AddModule(counter, "count_frames")



tray.Execute(1000)


# This file should have 4 events passing of the first 1000
if not npassed_veto == 4 or not npassed_timeveto == 4:
    sys.exit(1)
else:
    sys.exit(0)
