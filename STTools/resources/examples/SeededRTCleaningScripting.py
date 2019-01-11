import sys

from icecube import icetray, dataio
from icecube.icetray import I3Units

from icecube import STTools
from icecube.STTools.seededRT.configuration_services import I3DOMLinkSeededRTConfigurationService

icetray.set_log_level(icetray.I3LogLevel.LOG_TRACE)
icetray.set_log_level_for_unit('STToolsSeededRT',                icetray.I3LogLevel.LOG_TRACE)
icetray.set_log_level_for_unit('OMKeyPairMap',                   icetray.I3LogLevel.LOG_DEBUG)
icetray.set_log_level_for_unit('OMKeyHasher',                    icetray.I3LogLevel.LOG_DEBUG)
icetray.set_log_level_for_unit('I3SeededRTConfigurationService', icetray.I3LogLevel.LOG_DEBUG)

try:
    gcdfilename = sys.argv[1]
    infilename  = sys.argv[2]
    outfilename = sys.argv[3]
except:
    raise RuntimeError("Usage: %s gcdfile infile outfile"%(sys.argv[0]))

# Create a seededRT configuration service, based on DOM link ST settings.
seededRTConfigService = I3DOMLinkSeededRTConfigurationService(
    allowSelfCoincidence        = False,           # Default: False
    useDustlayerCorrection      = True,            # Default: True
    treat_string_36_as_deepcore = True,            # Default: True
    dustlayerUpperZBoundary     = 0*I3Units.m,
    dustlayerLowerZBoundary     = -150*I3Units.m,
    ic_ic_RTTime                = 1000*I3Units.ns,
    ic_ic_RTRadius              = 150*I3Units.m
)

gcdfile = dataio.I3File(gcdfilename, "r")
infile  = dataio.I3File(infilename, "r")
outfile = dataio.I3File(outfilename, "w")

# Get the first Geometry, the first DAQ, and the first Physics frame from the
# input files.
gframe = gcdfile.pop_frame()
while gframe.Stop != icetray.I3Frame.Geometry:
    gframe = gcdfile.pop_frame()
qframe = infile.pop_frame()
while qframe.Stop != icetray.I3Frame.DAQ:
    qframe = infile.pop_frame()
pframe = infile.pop_frame()
while pframe.Stop != icetray.I3Frame.Physics:
    pframe = infile.pop_frame()

# Setup the spatial context for the retrieved geometry.
seededRTConfigService.SetupSContext(gframe['I3Geometry'].omgeo)

# Create the object for the seed procedure.
seedProcedure = STTools.seededRT.seed_with_HLC_core_hits_I3RecoPulse_(
    stConfigService = seededRTConfigService,
    nHitsThreshold  = 2,
    allowNoSeedHits = False)

# Do the actual seededRT cleaning.
srtCleanedPulseMap = STTools.seededRT.doSeededRTCleaning_RecoPulse_(
    stConfigService       = seededRTConfigService,
    seedProcedure         = seedProcedure,
    frame                 = pframe,
    inputHitSeriesMapName = "TWOfflinePulses_DC",
    maxNIterations        = -1,   # Optional, default is -1.
    allowNoSeedHits       = False # Optional, default is False.
    )

pframe.Put("SRTOnTWOfflinePulses_DC", srtCleanedPulseMap)
pframe.Put("SRTConfiguration", seededRTConfigService.st_config_vec)
outfile.push(qframe)
outfile.push(pframe)

gcdfile.close()
infile.close()
outfile.close()

# Print out the ST configuration.
for e in seededRTConfigService.st_config_vec:
    print(str(e))
