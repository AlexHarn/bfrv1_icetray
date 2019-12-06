#!/usr/bin/env python

 ###################################################################################
 # Example Script: NoiseEngine
 # @version $Id: $
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
from I3Tray import *
from icecube import icetray, STTools, NoiseEngine
from icecube.STTools.seededRT.configuration_services import I3DOMLinkSeededRTConfigurationService
infile = ["/data/sim/IceCube/2010/filtered/level2a/neutrino-generator/6359/GeoCalibDetectorStatus_IC79.55380_L2a.i3.gz",
	"/data/sim/IceCube/2010/filtered/level2a/neutrino-generator/6359/00000-00999/Level2a_nugen_numu_IC79.006359.000030.i3.bz2"]

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
                OutputName = "NoiseEngine_bool",
                writePulses = True)

#----------------------------------------------------------------- 
#Run as separate modules
# This illustrates the hit cleaning used by NoiseEngine's tray
# segment. The static time window cleaning  runs first to remove 
# spurious noise hits from the leading and trailing edges of the
# frame.
#-----------------------------------------------------------------
icetray.load("libstatic-twc")
tray.AddModule( "I3StaticTWC<I3RecoPulseSeries>", "StaticTWC",
                InputResponse = "OfflinePulses",
                OutputResponse = "OfflinePulses_StaticTWC",
                WindowMinus = 3000.0,
                WindowPlus = 2000.0,
                TriggerName = "I3TriggerHierarchy")

#----------------------------------------------------------------- 
# Next, the RT hit cleaning module runs in an effort to focus
# the NoiseEngine algorithm on those hits most likely to be due
# to physics in the detector.
#----------------------------------------------------------------- 
seededRTConfigService_nodust = I3DOMLinkSeededRTConfigurationService(
    useDustlayerCorrection  = False,
    ic_ic_RTTime           = 750*I3Units.ns,
    ic_ic_RTRadius         = 150*I3Units.m
)

tray.AddModule("I3SeededRTCleaning_RecoPulse_Module", "RTCleaning_STTools",
               AllowNoSeedHits = False,
               InputHitSeriesMapName = "OfflinePulses_StaticTWC_STW",
               OutputHitSeriesMapName = "OfflinePulses_StaticTWC_STW_ClassicRT",
               STConfigService = seededRTConfigService_nodust,
               MaxNIterations = 0,
               SeedProcedure = "AllCoreHits",
)

#----------------------------------------------------------------- 
# And finally run the NoiseEngine filter. This produces a map of
# angles from all possible pairs of hits in the detector that are
# within an apparent velocity and time window. If more than threshold
# pairs are within a given angular bin, then the event passes and
# is likely due to some kind of interesting physics in the detector.
# otherwise, the event is probably caused by detector noise.
#----------------------------------------------------------------- 
tray.AddModule("NoiseEngine","NoiseEngine",
               HitSeriesName = "OfflinePulses_StaticTWC_ClassicRT",
               ChargeWeight = True,
               EndVelocity = 1.0,
               HealpixOrder = 1,
               IcePickServiceKey = '',
               NChLimit = 20,
               OutputName = 'NoiseEngine',
               StartVelocity = 0.1,
               Threshold = 3.0, 
               TimeWindowLength = 750,
               )

#----------------------------------------------------------------- 
# Finally, delete the produced hit series to prevent clutter in the
# physics frame.
#----------------------------------------------------------------- 
tray.AddModule("Delete", "NoiseEngine_cleanup", 
               keys = ["OfflinePulses_StaticTWC","OfflinePulses_ClassicRT"]
               )

##################################################################
# Write output to file and move to next frame
##################################################################

tray.AddModule("I3Writer","writer",
               FileName = "example.i3",
               CompressionLevel = 0)


    
tray.Execute(100)

