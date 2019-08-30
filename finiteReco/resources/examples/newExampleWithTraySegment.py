#!/usr/bin/env python

from os.path import expandvars, join

from I3Tray import *
from icecube import icetray, dataclasses, dataio, gulliver, finiteReco


infile  = expandvars('$I3_TESTDATA/event-viewer/Level3aGCD_IC79_EEData_Run00115990.i3')
outfile = 'advancedLengthReco.i3'

tray = I3Tray()

tray.AddModule('I3Reader', 'reader',
               Filename = infile
               )

# Using old tables
tray.AddSegment(finiteReco.advancedLengthReco, 'advancedLengthReco',
                inputPulses        = 'OfflinePulses_FR_DCVeto',
                inputReco          = 'MPEFit',
                geometry           = 'IC79',
                PhotonicsDir       = expandvars('$I3_TESTDATA/finiteReco/'),
                PhotonicsDriverDir = expandvars('$I3_TESTDATA/finiteReco/driverfiles/'),
                PhotonicsListFile  = 'SPICEMie_i3coords_level2_muon_finiteRecoTestONLY.list'
                )

# Using splines instead of tables
splinedir = '/cvmfs/icecube.opensciencegrid.org/data/photon-tables/splines/'
tray.AddSegment(finiteReco.advancedSplineLengthReco, 'advancedSplineLengthReco',
                inputPulses        = 'OfflinePulses_FR_DCVeto',
                inputReco          = 'MPEFit',
                geometry           = 'IC79',
                AmplitudeTable     = join(splinedir, 'InfBareMu_mie_abs_z20a10_V2.fits'),
                TimingTable        = join(splinedir, 'InfBareMu_mie_prob_z20a10_V2.fits'),
)

tray.AddModule('I3Writer', 'writer',
               Streams  = [icetray.I3Frame.DAQ, icetray.I3Frame.Physics],
               Filename = outfile
               )


tray.Execute()

