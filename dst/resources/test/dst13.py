#!/usr/bin/env python
#
"""
Test for conservation of frames (Make sure DSTExtractor is not eating up frames)
"""

import os,sys
from I3Tray import *
import os.path 
from os.path import expandvars
from os import unlink

from icecube import (icetray,dataclasses,dst,phys_services)
from icecube.icetray import I3Int
from icecube.dst.dsttest import CheckFrameIndex
from icecube import filter_tools

dstfile = "dsttest.root"

i3_ports = expandvars("$I3_TESTDATA")
gcdfile = i3_ports + "/sim/GeoCalibDetectorStatus_IC80_DC6.54655.i3.gz"

if not os.path.exists(gcdfile):
   raise Exception("No GCD file found in %s!" % gcdfile)


tray = I3Tray()

rand = phys_services.I3GSLRandomService(123)
tray.context["I3RandomService"]= rand

tray.AddModule("I3InfiniteSource", "TMA-2",
   Stream = icetray.I3Frame.DAQ,
   Prefix = gcdfile)

tray.AddModule("DSTTestGenerator","generatedst",
   SubEventStreamName="dst_stream"
)


# You can write the dst to .root files
#tray.AddModule("QConverter","qify-dst")(
#    ("WritePFrame", True)
#    )

tray.AddModule(CheckFrameIndex)

tray.AddModule("CountFrames",DAQ=10,Physics=10,Calibration=1,DetectorStatus=1,Geometry=1)

tray.AddModule("I3DSTDAQModule13", "dstq",
  DSTHeaderName="I3DST13Header" ,
  DSTName="I3DST13" 
)
tray.AddModule("I3DSTModule13", "dst",IgnoreDirectHits=True)

tray.AddModule("KeepFromSubstream","dst_stream",
      StreamName = "dst_stream",
      KeepKeys = ["I3DSTReco13","Pindex"],
      )


tray.AddSegment(dst.ExtractDST13,"extractdst",
            dst_output_filename = dstfile,
            extract_to_frame = True,
            dstname = "I3DST13")

tray.AddModule("Rename","pcounter_fix",
    Keys = ["Pindex_dst_stream0","Pindex"]
    )

#tray.AddModule("CountFrames",DAQ=10,Physics=10,Calibration=1,DetectorStatus=1,Geometry=1)

tray.AddModule(CheckFrameIndex,check=True)

# The usual trashcan-at-the-end idiom.
#
tray.AddModule("Dump")


tray.Execute(14)

unlink(dstfile)

