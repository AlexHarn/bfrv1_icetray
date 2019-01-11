#!/usr/bin/env python
	       
from I3Tray import *

from os.path import expandvars

import os
import sys

from icecube import icetray
from icecube import dataclasses
from icecube import phys_services
from icecube import dataio
from icecube import portia
from icecube import DomTools
from icecube import WaveCalibrator

workspace = expandvars("$I3_BUILD")
tools = expandvars("$I3_TESTDATA")
 	
infile = tools + "/event-viewer/Level3aGCD_IC79_EEData_Run00115990.i3"
tray = I3Tray()
 	
tray.AddModule("I3Reader","reader")(
    ("Filename", infile)
    )

#define HLC and SLC-only DOMLaunches
tray.AddModule( 'I3LCCleaning', 'CleaningSLC',
                    InIceInput ='InIceRawData',
                    InIceOutput='InIceRawDataHLC',              #! Name of HLC-only DOMLaunches
                    InIceOutputSLC='InIceRawDataSLC') #! Name of the SLC-only DOMLaunches

# no droop correction for the EHEFilter 
tray.AddModule('I3WaveCalibrator', 'EHEWaveCalibrator',
                   CorrectDroop= False,
                   Launches ='InIceRawDataHLC',
                   Waveforms='EHECalibratedWaveforms',
                   Errata = 'CalibrationErrata',
                   )

# write ATWD and FADC waveforms into two separate maps 
# pick the highest-gain ATWD-channel that is not saturated 
tray.AddModule('I3WaveformSplitter', 'EHEWaveformSplitter',
                   Input   ='EHECalibratedWaveforms',
                   PickUnsaturatedATWD=True,
                   HLC_ATWD='EHECalibratedATWD',
                   HLC_FADC='EHECalibratedFADC',
                   SLC     ='EHECalibratedGarbage',
                   Force   =True,
                   )

tray.AddModule("I3Portia","Portiapulse",
                   DataReadoutName          = 'InIceRawDataHLC',
                   ATWDWaveformName         = 'EHECalibratedATWD',
                   FADCWaveformName         = 'EHECalibratedFADC',
                   MakeBestPulseSeries      =  True,
                   BestPortiaPulseName      = "BestPortiaPulseHLC",
                   )
 	
tray.AddModule("Dump","dump")




tray.Execute(10+3)

