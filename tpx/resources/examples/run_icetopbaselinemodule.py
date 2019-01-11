#!/usr/bin/env python

import sys
from I3Tray import *
from icecube import icetray, dataio, WaveCalibrator, tpx, tableio
from icecube.rootwriter import I3ROOTWriter


import argparse
parser = argparse.ArgumentParser(description='Run IceTop baseline module.')
parser.add_argument('-o', '--output', metavar='root_file', type=str, help='Output filename',
                    default='icetop_baselines.root')
parser.add_argument('inputfiles', metavar='i3_file', type=str, nargs='?', help='Input files',
                    default=[os.path.expandvars('${I3_TESTDATA}/icetop/Raw_IC79_data_Run00117306_10events_IT.i3.bz2')]
                )
args = parser.parse_args()

outfile = args.output
infiles = args.inputfiles



tray = I3Tray()

tray.AddModule('I3Reader', 'reader',
               FileNameList = infiles)

tray.AddModule('I3WaveCalibrator', 'WaveCalibrator',
               Launches             = 'IceTopRawData',
               Waveforms            = 'CalibratedIceTopWaveforms',
               ATWDSaturationMargin = 123, # 1023-900 == 123  <-- this is IceTop specific
               FADCSaturationMargin = 0
               )

tray.AddModule('I3IceTopBaselineModule', 'IceTopBaselineModule',
               Waveforms            = 'CalibratedIceTopWaveforms',
               BaselineRange        = [-45, -5],
               Output               = 'IceTopBaselines'
               )

tray.AddModule('I3NullSplitter', 'nullsplit')

tray.AddSegment(I3ROOTWriter, 'RootWriter',
                Output = outfile,
                SubEventStreams = ['nullsplit'],
                Keys = ['IceTopBaselines']
                )



tray.Execute()

