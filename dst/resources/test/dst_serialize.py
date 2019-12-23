#!/usr/bin/env python
#
"""
Test for conservation of frames (Make sure DSTExtractor is not eating up frames)
"""

import os,sys
from I3Tray import *
import os.path 
from os.path import expandvars

from icecube import (icetray,dataclasses,dst,phys_services)
from icecube.dst.dsttest import CheckFrameIndex
from icecube import filter_tools

dstfile = "dsttest.hdf5"

i3_ports = expandvars("$I3_TESTDATA")
gcdfile = i3_ports + "/GCD/GeoCalibDetectorStatus_IC86.55697_corrected_V2.i3.gz"

if not os.path.exists(gcdfile):
   raise Exception("No GCD file found in %s!" % gcdfile)

tray = I3Tray()

rand = phys_services.I3GSLRandomService(123)
tray.context["I3RandomService"]= rand

tray.AddModule("I3InfiniteSource", "TMA-2",
   Stream = icetray.I3Frame.DAQ,
   Prefix = gcdfile)


tray.AddModule("DSTTestGenerator","generatedst",
   SubEventStreamName="dst_stream",
   NEvents = 10
)

tray.AddModule(CheckFrameIndex)

tray.AddModule("I3DSTModule16", "dst", 
   DSTName="I3DST",
   DSTHeaderName="I3DSTHeader",
   IgnoreDirectHits=True)

tray.AddModule("KeepFromSubstream","dst_stream",
      StreamName = "dst_stream",
      KeepKeys = ["I3DST"],
      )

# You can write the dst to .root files
tray.AddSegment(dst.ExtractDST,"extractdst",
            dst_output_filename = dstfile,
            extract_to_frame = True,
            remove_filter_stream = False,
            dstname = "I3DST")

tray.AddModule("I3Writer","write_extracted", 
	       filename=dstfile.replace(".hdf5",".i3"),
	       streams=[icetray.I3Frame.DAQ,icetray.I3Frame.Physics]
               )

tray.Execute()

import os
os.unlink(dstfile)
os.unlink(dstfile.replace(".hdf5",".i3"))

