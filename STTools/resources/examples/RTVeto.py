import sys

from I3Tray import I3Tray
from icecube import icetray, dataclasses, dataio
from icecube.icetray import I3Units
from icecube.STTools.seededRT.configuration_services import I3DOMLinkSeededRTConfigurationService

#icetray.set_log_level_for_unit('STTools', icetray.I3LogLevel.LOG_TRACE)
#icetray.set_log_level_for_unit('OMKeyPairMap', icetray.I3LogLevel.LOG_DEBUG)
#icetray.set_log_level_for_unit('OMKeyHasher', icetray.I3LogLevel.LOG_DEBUG)
#icetray.set_log_level_for_unit('I3SeededRTConfigurationService', icetray.I3LogLevel.LOG_DEBUG)
#icetray.set_log_level_for_unit('I3RTVetoModule', icetray.I3LogLevel.LOG_DEBUG)

try:
    gcdfile = sys.argv[1]
    infile  = sys.argv[2]
    outfile = sys.argv[3]
except:
    raise RuntimeError("Usage: %s gcdfile infile outfile"%(sys.argv[0]))

# Create a seededRT configuration service.
stConfigService = I3DOMLinkSeededRTConfigurationService(
    allowSelfCoincidence    = False,            # Default: False
    useDustlayerCorrection  = True,             # Default: True
    dustlayerUpperZBoundary =    0*I3Units.m,   # Default: 0m
    dustlayerLowerZBoundary = -150*I3Units.m,   # Default: -150m
    ic_ic_RTTime            = 1000*I3Units.ns,  # Default: 1000ns
    ic_ic_RTRadius          =  150*I3Units.m    # Default: 150m
)

tray = I3Tray()

tray.AddModule("I3Reader", "reader",
    filenamelist = [gcdfile, infile]
)
tray.AddModule("I3RTVeto_RecoPulse_Module", "rtveto",
    STConfigService         = stConfigService,
    InputHitSeriesMapName   = "TWOfflinePulses_DC",
    OutputHitSeriesMapName  = "RTVetoOnTWOfflinePulses_DC",
    Streams                 = [icetray.I3Frame.Physics]
)
tray.AddModule("I3Writer", "writer",
    filename          = outfile,
    Streams           = [icetray.I3Frame.TrayInfo,
                         icetray.I3Frame.DAQ, icetray.I3Frame.Physics],
    DropOrphanStreams = [icetray.I3Frame.DAQ]
)


tray.Execute()

