#!/usr/bin/env python

import argparse
import os

from icecube import WaveCalibrator, dataio, icetray, tpx
from I3Tray import I3Tray

parser = argparse.ArgumentParser(description='Run TPX.')
parser.add_argument('-i', '--input', dest = 'infile', type = str,
                  default = os.path.expandvars('${I3_TESTDATA}/icetop/Raw_IC79_data_Run00117306_10events_IT.i3.bz2'),
                  help = 'input file', metavar = 'FILE')
parser.add_argument('-o', '--output', dest = 'outfile', type = str,
                  default = 'run_tpx.i3',
                  help = 'output file', metavar = 'FILE')
args = parser.parse_args()

tray = I3Tray()

tray.AddModule('I3Reader', 'Reader',
    Filename = args.infile
    )

# WaveCalibrator, see the module documentation for more
tray.AddModule('I3WaveCalibrator', 'WaveCalibrator',
    Errata               = 'IceTopErrata',
    Launches             = 'IceTopRawData',
    WaveformRange        = '',
    Waveforms            = 'IceTopCalibratedWaveforms'
    )

# Split waveforms into ATWD, FADC and SLC. We don't need FADC
# See WaveCalibrator for details
tray.AddModule('I3WaveformSplitter', 'WaveformSplitter',
    Force               = True,
    Input               = 'IceTopCalibratedWaveforms',
    HLC_ATWD            = 'CalibratedIceTopATWD_HLC',
    HLC_FADC            = 'CalibratedIceTopFADC_HLC',
    PickUnsaturatedATWD = True,
    SLC                 = 'CalibratedIceTopATWD_SLC'
    )

# Extract HLC pulses
tray.AddModule('I3TopHLCPulseExtractor', 'TopHLCPulseExtractor',
    PEPulses  = 'IceTopHLCPEPulses',         # Pulses in PE, set to empty string to disable output
    PulseInfo = 'IceTopHLCPulseInfo',        # PulseInfo: amplitude, rise time, etc. Empty string to disable
    VEMPulses = 'IceTopHLCVEMPulses',        # Pulses in VEM, set to empty string to disable
    Waveforms = 'CalibratedIceTopATWD_HLC'   # Input HLC waveforms from WaveCalibrator
    )

# Extract SLC pulses
tray.AddModule('I3TopSLCPulseExtractor', 'TopSLCPulseExtractor',
    PEPulses  = 'IceTopSLCPEPulses',         # (see above ...)
    VEMPulses = 'IceTopSLCVEMPulses',
    Waveforms = 'CalibratedIceTopATWD_SLC'   # Input SLC waveforms from WaveCalibrator
    )

# Write things out
tray.AddModule('I3Writer', 'Writer',
    FileName = args.outfile,
    # Don't write calibrated waveforms
    SkipKeys = ['IceTopCalibratedWaveforms',
                'CalibratedIceTopATWD_HLC',
                'CalibratedIceTopATWD_SLC',
                'CalibratedIceTopFADC_HLC',],
    Streams  = [icetray.I3Frame.Geometry,
                icetray.I3Frame.Calibration,
                icetray.I3Frame.DetectorStatus,
                icetray.I3Frame.DAQ,
                icetray.I3Frame.Physics]
    )



tray.Execute()

