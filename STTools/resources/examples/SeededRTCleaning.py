import sys

from I3Tray import I3Tray
from icecube import icetray, dataclasses, dataio
from icecube.icetray import I3Units
from icecube.STTools.seededRT.configuration_services import I3DOMLinkSeededRTConfigurationService

#icetray.set_log_level_for_unit('STTools', icetray.I3LogLevel.LOG_TRACE)
icetray.set_log_level_for_unit('OMKeyPairMap', icetray.I3LogLevel.LOG_DEBUG)
icetray.set_log_level_for_unit('OMKeyHasher', icetray.I3LogLevel.LOG_DEBUG)
icetray.set_log_level_for_unit('I3SeededRTConfigurationService', icetray.I3LogLevel.LOG_DEBUG)
icetray.set_log_level_for_unit('I3SeededRTCleaningModule', icetray.I3LogLevel.LOG_DEBUG)

try:
    gcdfile = sys.argv[1]
    infile  = sys.argv[2]
    outfile = sys.argv[3]
except:
    raise RuntimeError("Usage: %s gcdfile infile outfile"%(sys.argv[0]))

# Create a seededRT configuration service for the classic ST configuration as
# it is used by the SeededRTCleaning project.
stConfigService = I3DOMLinkSeededRTConfigurationService(
    allowSelfCoincidence    = False,           # Default
    useDustlayerCorrection  = True,            # Default
    dustlayerUpperZBoundary = 0*I3Units.m,     # Default
    dustlayerLowerZBoundary = -150*I3Units.m,  # Default
    ic_ic_RTTime            = 1000*I3Units.ns, # Default
    ic_ic_RTRadius          = 150*I3Units.m    # Default
)

tray = I3Tray()

tray.AddModule("I3Reader", "reader",
    filenamelist = [gcdfile, infile]
)

"""
# Test the HitSeriesMapHitsFromFrame seed procedure.
tray.AddModule(lambda f: f.Put("MySeedHitSeriesMap", f.Get("OfflinePulses")))
tray.AddModule("I3SeededRTCleaning_RecoPulse_Module", "seededRTcleaning",
    STConfigService         = stConfigService,
    InputHitSeriesMapName   = "OfflinePulses",
    OutputHitSeriesMapName  = "SRTCleaningOnOfflinePulses",
    SeedProcedure           = "HitSeriesMapHitsFromFrame",
    SeedHitSeriesMapName    = "MySeedHitSeriesMap",
    Streams                 = [icetray.I3Frame.Physics]
)
"""

# Do the classic RT cleaning on all hits. First it will select all hits
# and then deselect those hits, which have less than NHitsThreshold
# corresponding hits fulfilling the ST configuration, i.e. ST partners.
tray.AddModule("I3SeededRTCleaning_RecoPulse_Module", "classic_RTcleaning",
    STConfigService         = stConfigService,
    InputHitSeriesMapName   = "TWOfflinePulses_DC",
    OutputHitSeriesMapName  = "RTCleaningOnTWOfflinePulses_DC",
    SeedProcedure           = "AllCoreHits",
    NHitsThreshold          = 2,
    MaxNIterations          = 0,
    Streams                 = [icetray.I3Frame.Physics]
)

# Do the classic seeded RT cleaning.
tray.AddModule("I3SeededRTCleaning_RecoPulse_Module", "seededRTcleaning",
    STConfigService         = stConfigService,
    InputHitSeriesMapName   = "TWOfflinePulses_DC",
    OutputHitSeriesMapName  = "SRTCleaningOnTWOfflinePulses_DC",
    SeedProcedure           = "AllHLCHits",
    MaxNIterations          = -1,
    Streams                 = [icetray.I3Frame.Physics]
)

tray.AddModule("I3Writer", "writer",
    filename          = outfile,
    Streams           = [icetray.I3Frame.TrayInfo,
                         icetray.I3Frame.DAQ, icetray.I3Frame.Physics],
    DropOrphanStreams = [icetray.I3Frame.DAQ]
)


tray.Execute()

