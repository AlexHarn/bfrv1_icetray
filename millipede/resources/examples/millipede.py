#!/usr/bin/env python

"""
Example millipede muon energy loss fits.  The first fits the
loss pattern as stochastic losses (e.g. from a high-energy
muon), and the second as continuous losses.
input: offline reconstructed .i3 file(s)
"""

from I3Tray import *
import sys
from icecube import icetray, dataio, dataclasses, photonics_service
load('millipede')

if len(sys.argv) < 3:
	print('Usage: %s output.i3 input1.i3 [input2.i3] ...' % sys.argv[0])
	sys.exit(1)


files = sys.argv[2:]

table_base = os.path.expandvars('$I3_DATA/photon-tables/splines/emu_%s.fits')
muon_service = photonics_service.I3PhotoSplineService(table_base % 'abs', table_base % 'prob', 0)
table_base = os.path.expandvars('$I3_DATA/photon-tables/splines/ems_spice1_z20_a10.%s.fits')
cascade_service = photonics_service.I3PhotoSplineService(table_base % 'abs', table_base % 'prob', 0)

tray = I3Tray()
tray.AddModule('I3Reader', 'reader', FilenameList=files)

tray.AddModule('MuMillipede', 'millipede_highenergy',
    MuonPhotonicsService=muon_service, CascadePhotonicsService=cascade_service,
    PhotonsPerBin=15, MuonRegularization=0, ShowerRegularization=0,
    MuonSpacing=0, ShowerSpacing=10, SeedTrack='MPEFit',
    Output='MillipedeHighEnergy', Pulses='MuonPulseSeriesReco')

tray.AddModule('MuMillipede', 'millipede_lowenergy',
    MuonPhotonicsService=muon_service, CascadePhotonicsService=cascade_service,
    PhotonsPerBin=10, MuonRegularization=2, ShowerRegularization=0,
    MuonSpacing=15, ShowerSpacing=0, SeedTrack='MPEFit',
    Output='MillipedeLowEnergy', Pulses='MuonPulseSeriesReco')

tray.AddModule('I3Writer', 'writer', filename=sys.argv[1])

tray.Execute()


