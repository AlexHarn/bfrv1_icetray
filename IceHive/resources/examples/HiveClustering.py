#!/usr/bin/env python

"""
This script demonstrates running HiveSplitter to split a
RecoPulseSeries (located in the Q frame) into separate pulse series
for each subevent, each in its own P frame.
USAGE: python [this_script.py] [infile] [outfile]
"""

import sys
from I3Tray import *
from icecube import icetray, dataio, dataclasses
from icecube import IceHive

from icecube.icetray import I3Units

icetray.set_log_level(icetray.I3LogLevel.LOG_INFO)
icetray.set_log_level_for_unit("I3IceHive", icetray.I3LogLevel.LOG_DEBUG)

tray = I3Tray()

tray.AddModule("I3Reader","Reader",
        FilenameList=sys.argv[1:-1])

tray.AddModule("Delete", "delete_keys_in_use",
               Keys = ["MaskedOfflinePulses"+"_Physics",
                       "MaskedOfflinePulses"+"_Noise",
                       "MaskedOfflinePulses"+"_Noised",])

#tray.AddModule("Dump", "dump")

singleRings = IceHive.RingLimits()
singleRings.AddLimitPair(IceHive.LimitPair(-255., 255.))
singleRings.AddLimitPair(IceHive.LimitPair(-272.7, 272.7))
singleRings.AddLimitPair(IceHive.LimitPair(-165.8, 165.8))
doubleRings = IceHive.RingLimits()
doubleRings.AddLimitPair(IceHive.LimitPair(-70., 70.))
doubleRings.AddLimitPair(IceHive.LimitPair(-131.5, 131.5))
doubleRings.AddLimitPair(IceHive.LimitPair(-40.8, 40.8))
tripleRings = IceHive.RingLimits()
tripleRings.AddLimitPair(IceHive.LimitPair(-70., 70.))
tripleRings.AddLimitPair(IceHive.LimitPair(-144.1, 144.1))
tripleRings.AddLimitPair(IceHive.LimitPair(-124.7, 124.7))
tripleRings.AddLimitPair(IceHive.LimitPair(-82.8, 82.8))
singleVicinity = IceHive.RingLimits()
singleVicinity.AddLimitPair(IceHive.LimitPair(-100.,100.))
singleVicinity.AddLimitPair(IceHive.LimitPair(-100.,100.))
doubleVicinity = IceHive.RingLimits()
doubleVicinity.AddLimitPair(IceHive.LimitPair(-100.,100.))
doubleVicinity.AddLimitPair(IceHive.LimitPair(-100.,100.))
doubleVicinity.AddLimitPair(IceHive.LimitPair(-100.,100.))
tripleVicinity = IceHive.RingLimits()
tripleVicinity.AddLimitPair(IceHive.LimitPair(-100.,100.))
tripleVicinity.AddLimitPair(IceHive.LimitPair(-100.,100.))
tripleVicinity.AddLimitPair(IceHive.LimitPair(-100.,100.))
tripleVicinity.AddLimitPair(IceHive.LimitPair(-100.,100.))

tray.AddModule("I3IceHive<I3RecoPulse>","IceHiveSplit",
  InputName="OfflinePulses",
  OutputName="MaskedOfflinePulses",
  Multiplicity=4,
  TimeWindow=2000.*I3Units.ns,
  TimeCVMinus=300.*I3Units.ns,
  TimeCVPlus=300.*I3Units.ns,
  TimeCNMinus=200.*I3Units.ns,
  TimeCNPlus=200.*I3Units.ns,
  TimeStatic=200.*I3Units.ns,
  SingleDenseRingLimits=singleRings,
  DoubleDenseRingLimits=doubleRings,
  TripleDenseRingLimits=tripleRings,
  SingleDenseRingVicinity=singleVicinity,
  DoubleDenseRingVicinity=doubleVicinity,
  TripleDenseRingVicinity=tripleVicinity,
  SaveSplitCount=True)

tray.AddModule("I3Writer","Writer",
  Filename=sys.argv[-1],
  Streams=[icetray.I3Frame.Physics, icetray.I3Frame.DAQ])


tray.Execute()

