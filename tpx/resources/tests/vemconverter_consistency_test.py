#!/usr/bin/env python

# This script makes sure that VEM calibration in I3TopHLCPulseExtractor and I3VEMConverter delivers identical results

import os
import unittest

from icecube import WaveCalibrator, dataclasses, dataio, icetray, tpx
from I3Tray import I3Tray

class CompareHLCPulseExtractorToVEMCalibrator(unittest.TestCase):
    def test_frame_objects_equal(self):
        psm1 = dataclasses.I3RecoPulseSeriesMap.from_frame(self.frame, 'IceTopHLCVEMPulses')
        psm2 = dataclasses.I3RecoPulseSeriesMap.from_frame(self.frame, 'I3VEMConverterPulses')
        
        self.assertEqual(psm1, psm2, "these should be the same.")


infile = os.path.expandvars('$I3_TESTDATA') + '/icetop/Raw_IC79_data_Run00117306_10events_IT.i3.bz2'

tray = I3Tray()

tray.Add('I3Reader',
    Filename = infile
    )

tray.Add('I3WaveCalibrator',
    Launches             = 'IceTopRawData',
    Waveforms            = 'CalibratedWaveforms',
    ATWDSaturationMargin = 123, # 1023-900 == 123  <-- this is IceTop specific
    FADCSaturationMargin = 0
    )

tray.Add('I3WaveformSplitter',
    Force               = True,
    HLC_ATWD            = 'CalibratedIceTopATWD_HLC',
    HLC_FADC            = 'CalibratedIceTopFADC_HLC',
    SLC                 = 'CalibratedIceTopATWD_SLC',
    Input               = 'CalibratedWaveforms',
    PickUnsaturatedATWD = True
    )

tray.Add('I3TopHLCPulseExtractor',
    Waveforms = 'CalibratedIceTopATWD_HLC',   # Input HLC waveforms from WaveCalibrator
    PEPulses  = 'IceTopHLCPEPulses',          # Pulses in PE, set to empty string to disable output
    PulseInfo = 'IceTopHLCPulseInfo',         # PulseInfo: amplitude, rise time, etc. Empty string to disable
    VEMPulses = 'IceTopHLCVEMPulses'          # Pulses in VEM, set to empty string to disable
    )

tray.Add('I3VEMConverter',
    PEPulses  = 'IceTopHLCPEPulses',
    VEMPulses = 'I3VEMConverterPulses'
    )

tray.Add(icetray.I3TestModuleFactory(CompareHLCPulseExtractorToVEMCalibrator),
    Streams = [icetray.I3Frame.DAQ]
    )



tray.Execute()

