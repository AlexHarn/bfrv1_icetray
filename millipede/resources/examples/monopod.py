#!/usr/bin/env python

"""
Example monopod energy fit.
input: offline reconstructed .i3 file(s)
""" 

from icecube import icetray, dataclasses, dataio, millipede, photonics_service
from I3Tray import I3Tray

import sys, os
infiles = sys.argv[2:]

table_base = os.path.expandvars('$I3_DATA/photon-tables/splines/ems_spice1_z20_a10.%s.fits')
cascade_service = photonics_service.I3PhotoSplineService(table_base % 'abs', table_base % 'prob', 0)

tray = I3Tray()

tray.AddModule('I3Reader', 'reader', filenamelist=infiles)

tray.AddModule('Monopod', 'monopod',
    CascadePhotonicsService=cascade_service,
    Pulses='TWOfflinePulsesHLC',
    Seed='CascadeLlhVertexFit',
    Output='MonopodFit',
)

tray.AddModule('I3Writer', 'writer', filename=sys.argv[1])


tray.Execute()

