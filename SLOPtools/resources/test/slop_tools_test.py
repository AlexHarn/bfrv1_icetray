#!/usr/bin/env python

# $Id: slop_tools_test.py 136765 2015-08-24 08:15:15Z jacobi $

"""
This script generates some fake events with a SLOP like topology
and then applies TupleTagger and MPCleaner to those events.
"""

from os.path import expandvars

i3testdata=expandvars("$I3_TESTDATA")
gcd=i3testdata + "/sim/GeoCalibDetectorStatus_2012.56063_V0.i3.gz"


from I3Tray import I3Tray
from icecube import icetray, dataclasses, dataio
from icecube.SLOPtools import TupleTagger, MPCleaner

from generate_slop import slopporator, sloppozela, slopunion

tray=I3Tray()

tray.Add("I3InfiniteSource", "infinite", Stream=icetray.I3Frame.Physics, Prefix=gcd)
tray.Add(slopporator, "Hobo SLOP")
tray.Add(sloppozela, "Hobo Noise")
tray.Add(slopunion, "Gewerk")
tray.Add(TupleTagger, "WholeCar", PulseMapName="SLOP_Pulses", RunOnPulses=True)
tray.Add(MPCleaner, "Tide", PulseMapName="SLOP_Pulses")
tray.Add("Dump", "Dumb")
#tray.Add("I3Writer","writer", filename="SLOPtools_test.i3", Streams=map(icetray.I3Frame.Stream, 'GP'))


tray.Execute(10+3)
