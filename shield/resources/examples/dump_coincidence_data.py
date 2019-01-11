#!/usr/bin/env python
# This treats all but its last argument as input i3 files, 
# for which it computes IceTop coincidence information (using 
# the MPEFit result as the shower hypothesis) which is then 
# written to the output HDF5 file specified as the last argument
import sys
from I3Tray import *

from icecube import icetray, dataclasses, dataio
from icecube import tableio, hdfwriter
from icecube import shield

trackReco = "MPEFit"

tray = I3Tray()

tray.AddModule("I3Reader", "reader")(
	("FilenameList",sys.argv[1:-1])
	)

tray.AddModule("I3ShieldDataCollector","summarizeHLC")(
	("InputRecoPulses","IceTopHLCVEMPulses"),
	("InputTrack",trackReco),
	("OutputName","HLCShieldResults")
	)

tray.AddModule("I3ShieldDataCollector","summarizeSLC")(
	("InputRecoPulses","IceTopSLCVEMPulses"),
	("InputTrack",trackReco),
	("OutputName","SLCShieldResults")
	)

hdftable = hdfwriter.I3HDFTableService(sys.argv[-1])
tray.AddModule(tableio.I3TableWriter, "writer")(
	("tableservice",hdftable),
	("SubEventStreams",['nullsplit']),
	("keys",[trackReco,"HLCShieldResults","SLCShieldResults"])
	)



tray.Execute()
