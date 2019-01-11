#!/usr/bin/env python

from I3Tray import *
from icecube import icetray, dataclasses, dataio, WaveCalibrator, tpx
from os.path import expandvars, dirname, basename

load('vemcal')

test_data = expandvars('$I3_PORTS') + '/test-data/icetop/Raw_IC79_data_Run00117306_10events_IT.i3.bz2'

tray = I3Tray()

tray.AddModule('I3Reader', 'reader',
               FileName = test_data)

tray.AddModule('I3WaveCalibrator', 'wavecalibrator',
               Launches = 'IceTopRawData',
               Waveforms = 'IceTopCalibratedWaveforms',
               ATWDSaturationMargin = 123)

tray.AddModule('I3WaveformSplitter', 'waveformsplitter',
               Input = 'IceTopCalibratedWaveforms',
               HLC_ATWD = 'IceTopHLCATWD',
               HLC_FADC = 'IceTopHLCFADC',
               SLC = 'IceTopSLC',
               PickUnsaturatedATWD = True
               )

    
tray.AddModule("I3TopHLCPulseExtractor", "tophlcpulseextractor",
               Waveforms="IceTopHLCATWD",
               PEPulses="IceTopPEPulses",
               VEMPulses="",
               PulseInfo=""
               )
    
tray.AddModule("I3HGLGPairSelector", "hglgpairselector",
               InputPulses = "IceTopPEPulses",
               OutputPulseMask = "IceTopHGLGData"
               )

def TestHGLGPairSelector(frame, HGLGData = "IceTopHGLGData"):
    if HGLGData in frame:
        hglgdata = dataclasses.I3RecoPulseSeriesMap.from_frame(frame, HGLGData)
        tankkeymap = dict()
        for omkey in hglgdata.iterkeys():
            tankkey = dataclasses.TankKey(omkey)
            if tankkey not in tankkeymap:
                tankkeymap[tankkey] = set()
            tankkeymap[tankkey].add(omkey)
        for omkeyset in tankkeymap.itervalues():
            if len(omkeyset) != 2:
                raise RuntimeError("HGLG data should contain two DOMs per tank. Instead I found %d"%len(omkeyset))
    print 'Event okay.'
    return True

tray.AddModule(TestHGLGPairSelector, 'testhglgpairselector',
               HGLGData = 'IceTopHGLGData',
               Streams = [icetray.I3Frame.DAQ])

#tray.AddModule('Dump', 'dump')



tray.Execute()

