#!/usr/bin/env python
 ###################################################################################
 # Example Script: NoiseEngine
 # @Version $Id: $
 # @date: $Date: $
 # @author Michael Larson <mjlarson@crimson.ua.edu>
 # (c) 2011,2012 IceCube Collaboration
 #
 # The NoiseEngine filter module is designed to identify triggers caused by 
 # random detector noise. This script is designed as an example script to
 # demonstrate two methods of running NoiseEngine. The tray segment is recommended
 # although the NoiseEngine algorithm will accept any I3DOMLaunch or I3RecoPulse
 # hit series as input.
 ###################################################################################
import sys
from os.path import expandvars
from optparse import OptionParser

from icecube import icetray, STTools, NoiseEngine

from icecube.icetray import I3Units
from icecube.STTools.seededRT.configuration_services import I3DOMLinkSeededRTConfigurationService

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

infile = [options.geoFile, options.data_file]

##################################################################
# BOOT INTO THE TRAY
##################################################################
tray = I3Tray()
tray.AddModule("I3Reader", "reader", filenamelist = infile)

##################################################################
# Run NoiseEngine
##################################################################
#Note that this example illustrates two ways to use this code.  You can use the
#code as tray segment, or with your own choice of cleaning method
##################################################################

#-----------------------------------------------------------------
# Run as a tray segment
# This includes static time window cleaning, RT hit cleaning, and
# the NoiseEngine filter itself. No frames are dropped during this
# processing, although the hit series used can be deleted after use
# if writePulses is set to False
#-----------------------------------------------------------------
tray.AddSegment(NoiseEngine.WithCleaners,"example",
                HitSeriesName = "OfflinePulses",
                OutputName = "NoiseEngine_TraySegment_bool",
                writePulses = True)

npassed_segment = 0
def counter(frame):
    global npassed_segment 
    if not frame.Has("NoiseEngine_TraySegment_bool"): return 
    npassed_segment += frame["NoiseEngine_TraySegment_bool"].value

tray.AddModule(counter, "count_frames")

##################################################################
# Keep going to the next frame
##################################################################    
tray.Execute(1000)


# This file should have 67 events passing of the first 1000
if not npassed_segment == 67:
    print("Failure. Expected 67, found ", str(npassed_segment))
    sys.exit(1)
else:
    sys.exit(0)
