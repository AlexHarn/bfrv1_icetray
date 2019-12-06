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
                   metavar="<geo file>", default=expandvars("$I3_TESTDATA/sim/GeoCalibDetectorStatus_2013.56429_V1.i3.gz"),
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

#----------------------------------------------------------------- 
# And finally run the NoiseEngine filter. This produces a map of
# angles from all possible pairs of hits in the detector that are
# within an apparent velocity and time window. If more than threshold
# pairs are within a given angular bin, then the event passes and
# is likely due to some kind of interesting physics in the detector.
# otherwise, the event is probably caused by detector noise.
#----------------------------------------------------------------- 
tray.AddModule("NoiseEngine","NoiseEngine",
               HitSeriesName = "TWSRTOfflinePulses",
               ChargeWeight = True,
               EndVelocity = 1.0,
               HealpixOrder = 1,
               IcePickServiceKey = '',
               NChLimit = 20,
               OutputName = 'NoiseEngine_module_bool',
               StartVelocity = 0.1,
               Threshold = 3.0, 
               TimeWindowLength = 750,
               )

npassed_module = 0
def counter(frame):
    global npassed_module
    if not frame.Has("NoiseEngine_module_bool"): return 
    npassed_module  += frame["NoiseEngine_module_bool"].value

tray.AddModule(counter, "count_frames")

##################################################################
# Keep going to the next frame
##################################################################
tray.Execute(1000)


# This file should have 492 events passing of the first 1000
if not npassed_module == 492:
    print("Failure. Expected 492, found ", str(npassed_module))
    sys.exit(1)
else:
    sys.exit(0)
