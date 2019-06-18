#!/usr/bin/env python
"""
Get the GCD Diff between 2012 and 2013 simulation GCD files.
"""

import os

from I3Tray import I3Tray
from icecube import icetray
from icecube.frame_object_diff.segments import compress

test_data = os.path.expandvars('$I3_TESTDATA')

base = os.path.join(test_data,'GCD','GeoCalibDetectorStatus_2012.56063_V0.i3.gz')
infile = os.path.join(test_data,'GCD','GeoCalibDetectorStatus_2013.56429_V1.i3.gz')
outfile = 'out.i3.gz'

tray = I3Tray()

tray.Add('I3Reader',Filename=infile)

tray.Add(compress,
         base_filename=base)

tray.Add('I3Writer',Filename=outfile,
         Streams=[icetray.I3Frame.Geometry,
                  icetray.I3Frame.Calibration,
                  icetray.I3Frame.DetectorStatus])

tray.Execute()

