#!/usr/bin/env python

# This script makes sure that the correct number of PE- and VEM-pulses is extracted
# Assumption: The test data file does not contain bad waveforms that cannot be extracted

import os
import unittest

from icecube import WaveCalibrator, dataclasses, dataio, icetray, tpx
from I3Tray import I3Tray

def count_objects(series_map):
    count = 0
    for key, series in series_map:
        count += len(series)
    return count


class TestPulseExtraction(unittest.TestCase):
    def test_number_of_objects(self):
        wvfs_hlc = count_objects(self.frame['CalibratedIceTopATWD_HLC'])
        pe_pulses_hlc = count_objects(dataclasses.I3RecoPulseSeriesMap.from_frame(self.frame, 'IceTopHLCPEPulses'))
        vem_pulses_hlc = count_objects(dataclasses.I3RecoPulseSeriesMap.from_frame(self.frame, 'IceTopHLCVEMPulses'))
        pulse_info = count_objects(self.frame['IceTopHLCPulseInfo'])

        self.assertEqual(wvfs_hlc, pe_pulses_hlc, "number of HLC waveforms and PE pulses should be the same.")
        self.assertEqual(wvfs_hlc, vem_pulses_hlc, "number of HLC waveforms and VEM pulses should be the same.")
        self.assertEqual(wvfs_hlc, pulse_info, "number of HLC waveforms and pulse info objects should be the same.")

        wvfs_slc = count_objects(self.frame['CalibratedIceTopATWD_SLC'])
        pe_pulses_slc = count_objects(dataclasses.I3RecoPulseSeriesMap.from_frame(self.frame, 'IceTopSLCPEPulses'))
        vem_pulses_slc = count_objects(dataclasses.I3RecoPulseSeriesMap.from_frame(self.frame, 'IceTopSLCVEMPulses'))

        self.assertEqual(wvfs_slc, pe_pulses_slc, "number of SLC waveforms and PE pulses should be the same.")
        self.assertEqual(wvfs_slc, vem_pulses_slc, "number of SLC waveforms and VEM pulses should be the same.")


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

tray.Add('I3TopSLCPulseExtractor',
    Waveforms = 'CalibratedIceTopATWD_SLC',
    PEPulses  = 'IceTopSLCPEPulses',
    VEMPulses = 'IceTopSLCVEMPulses'
    )

tray.Add(icetray.I3TestModuleFactory(TestPulseExtraction),
    Streams = [icetray.I3Frame.DAQ]
    )



tray.Execute()

