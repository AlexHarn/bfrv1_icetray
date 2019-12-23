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
from icecube import filter_tools, recclasses
import healpy, numpy

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

tray.AddModule("I3DSTModule16", "dst_NSIDE_1024", 
   DSTName="I3DST_NSIDE_1024",
   DSTHeaderName="I3DSTHeader_NSIDE_1024",
   HealPixNSide=1024,
   IgnoreDirectHits=True)

tray.AddModule("I3DSTModule16", "dst_NSIDE_516", 
   DSTName="I3DST_NSIDE_516",
   DSTHeaderName="I3DSTHeader_NSIDE_516",
   HealPixNSide=516,
   IgnoreDirectHits=True)


def check_pix(f,header_name="I3DSTHeader"):
    if header_name in f:
        h = f[header_name]
        nside  = h.heal_pix_n_side
        coords = dst.HealPixCoordinate()
        coords.ComputeBins(nside)
        print (healpy.nside2npix(nside), coords.bins())
        assert( healpy.nside2npix(nside) == coords.bins())
    return True


tray.AddModule(check_pix,"check1",
        header_name="I3DSTHeader_NSIDE_516")
tray.AddModule(check_pix,"check2",
        header_name="I3DSTHeader_NSIDE_1024")

# You can write the dst to .root files
tray.Execute()


