# This script demonstatrates the typical usage of the topeventcleaning project.
# It processes an input file that contains VEM calibrated DOM pulses.
# 1. I3HLCTankPulseMerger is run, making tank pulses from DOM pulses by choosing between the HG or LG DOM
# 2. I3TopHLCClusterCleaning is run, the IceTop event splitter


import optparse

from icecube import dataio, icetray, topeventcleaning
from I3Tray import I3Tray

parser = optparse.OptionParser()
parser.add_option('-i', '--input', dest = 'infile', type = 'str',
                  help = 'input file (use e.g. the output of tpx/resources/examples/run_tpx.py)', metavar = 'FILE')
parser.add_option('-o', '--output', dest = 'outfile', type = 'str',
                  default = 'run_topeventcleaning.i3',
                  help = 'output file', metavar = 'FILE')
(options, args) = parser.parse_args()

tray = I3Tray()

tray.AddModule('I3Reader', 'Reader',
    Filename = options.infile
    )

# Merge VEM pulses to tank pulses, i.e. choose between HG and LG
# Outputs a list of excluded stations (e.g. because HG and LG pulses do not match)
tray.AddModule('I3HLCTankPulseMerger', 'HLCTankPulseMerger',
    BadDomList       = 'IceTopBadDOMs',                  # list of DOMs to exclude
    BadTankList      = 'IceTopBadTanks',                 # list of tanks to exclude
    ExcludedTanks    = 'TankPulseMergerExcludedTanks',   # list of tanks excluded by this module
    InputVEMPulses   = 'IceTopHLCVEMPulses',             # VEM calibrated pulses
    MaxHGLGTimeDiff  = 40.0 * icetray.I3Units.ns,        # max time difference between HG and LG pulse
    OutputTankPulses = 'IceTopHLCTankPulses'             # merged tank pulses
    )

tray.AddModule('I3TopHLCClusterCleaning', 'IceTopSplit',
    BadTankList               = 'TankPulseMergerExcludedTanks',   # list of bad tanks (including those excluded by previous module)
    ExcludedTanks             = 'ClusterCleaningExcludedTanks',   # bad tank list produced by this module
    InputPulses               = 'IceTopHLCTankPulses',            # input: the pulses merged in the previous module
    InterStationTimeTolerance = 200.0 * icetray.I3Units.ns,       # time tolerance between stations (for splitting)
    IntraStationTimeTolerance = 200.0 * icetray.I3Units.ns,       # time tolerance within a station
    OutputPulses              = 'CleanedHLCTankPulses'            # name of output, it's actually an I3RecoPulseSeriesMapMask
    )

tray.AddModule('I3Writer', 'Writer',
    Filename = options.outfile,
    Streams  = [icetray.I3Frame.DAQ,
                icetray.I3Frame.Physics]
    )



tray.Execute()

