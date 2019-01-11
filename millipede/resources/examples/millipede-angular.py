#!/usr/bin/env python

"""
Example millipede muon angular/vertex fit using spline tables in $I3_DATA
input: offline reconstructed .i3 file(s)
""" 

from I3Tray import *
import sys
from icecube import icetray, dataio, dataclasses, photonics_service, gulliver, millipede
load('gulliver-modules')

if len(sys.argv) < 3:
	print('Usage: %s output.i3 input1.i3 [input2.i3] ...' % sys.argv[0])
	sys.exit(1)

files = sys.argv[2:]

table_base = os.path.expandvars('$I3_DATA/photon-tables/splines/emu_%s.fits')
muon_service = photonics_service.I3PhotoSplineService(table_base % 'abs', table_base % 'prob', 0)
table_base = os.path.expandvars('$I3_DATA/photon-tables/splines/ems_mie_z20_a10.%s.fits')
cascade_service = photonics_service.I3PhotoSplineService(table_base % 'abs', table_base % 'prob', 0)

tray = I3Tray()
tray.AddModule('I3Reader', 'reader', FilenameList=files)

# NB: Tuned for high energy and similiar to Monopod defaults!
tray.AddSegment(millipede.MuMillipedeFit, 'MillipedeFit',
    Seed='MPEFit',
    Minimizer='SIMPLEX',
    Pulses='OfflinePulses',
    ReadoutWindow="OfflinePulsesTimeRange",
    ExcludedDOMs=["BadDomsList", "CalibrationErrata", "SaturatedDOMs", "BrightDOMs"],
    MuonPhotonicsService=muon_service,
    CascadePhotonicsService=cascade_service,
    PhotonsPerBin=100,
    MuonRegularization=0,
    ShowerRegularization=0,
    MuonSpacing=0,
    ShowerSpacing=10,
    StepD=5*I3Units.m,
    StepT=15*I3Units.ns,
    StepZenith=5*I3Units.deg,
    StepAzimuth=5*I3Units.deg,
    )

tray.AddModule('I3Writer', 'writer', filename=sys.argv[1])

tray.Execute()

