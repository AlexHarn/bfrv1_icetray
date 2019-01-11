#!/usr/bin/env python

from os.path import expandvars

from I3Tray import *
from icecube import icetray, dataclasses, dataio, gulliver, finiteReco


infile  = expandvars('$I3_TESTDATA/event-viewer/Level3aGCD_IC79_EEData_Run00115990.i3')
outfile = 'advancedLengthReco.i3'

tray = I3Tray()

tray.AddModule('I3Reader', 'reader',
               Filename = infile
               )

tray.AddSegment(finiteReco.advancedLengthReco, 'advancedLengthReco',
                inputPulses        = 'OfflinePulses_FR_DCVeto',
                inputReco          = 'MPEFit',
                geometry           = 'IC79',
                PhotonicsDir       = expandvars('$I3_TESTDATA/finiteReco/'),
                PhotonicsDriverDir = expandvars('$I3_TESTDATA/finiteReco/driverfiles/'),
                PhotonicsListFile  = 'SPICEMie_i3coords_level2_muon_finiteRecoTestONLY.list'
                )

tray.AddModule('I3Writer', 'writer',
               Streams  = [icetray.I3Frame.DAQ, icetray.I3Frame.Physics],
               Filename = outfile
               )


tray.Execute()

