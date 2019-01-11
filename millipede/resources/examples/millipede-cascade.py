#!/usr/bin/env python

"""
Example monopod vertex/angular fit using spline tables in $I3_DATA.
input: offline reconstructed .i3 file(s)
""" 

from I3Tray import *
import sys, os
from icecube import icetray, dataio, dataclasses, phys_services, photonics_service, gulliver, millipede
load('gulliver-modules')

if len(sys.argv) < 3:
	print('Usage: %s output.i3 input1.i3 [input2.i3] ...' % sys.argv[0])
	sys.exit(1)

files = sys.argv[2:]

seed = 'CascadeLast'
table_base = os.path.expandvars('$I3_DATA/photon-tables/splines/ems_mie_z20_a10.%s.fits')
cascade_service = photonics_service.I3PhotoSplineService(table_base % 'abs', table_base % 'prob', 0)

tray = I3Tray()
tray.AddModule('I3Reader', 'reader', FilenameList=files)

tray.AddModule(lambda frame: seed in frame, 'seed_exists')

tray.AddSegment(millipede.MonopodFit, 'Monopod',
    CascadePhotonicsService=cascade_service,
    Seed=seed,
    Minimizer='SIMPLEX',
    Pulses='OfflinePulses',
    PhotonsPerBin=-1)

# some users may not expect arguments to be in the wrong order
assert not os.path.exists(sys.argv[1]), "output file %s already exists" % sys.argv[1]
tray.AddModule('I3Writer', 'writer', filename=sys.argv[1],
	DropOrphanStreams=[icetray.I3Frame.DAQ]
	)

tray.Execute()


