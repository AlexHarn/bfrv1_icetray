#!/usr/bin/env python

"""
This script demonstrates running HiveSplitter to split a
RecoPulseSeries (located in the Q frame) into separate pulse series
for each subevent, each in its own P frame.
Make some cleaning on the pulse-series and male an reconstruction
"""

import sys, os
from I3Tray import *
from icecube import icetray, dataio, dataclasses
from icecube.dataclasses import make_pair as mp

from icecube.icetray import I3Units

i3_testdata = os.path.expandvars("$I3_TESTDATA")

print ("Output will be written to "+os.path.join(os.path.expandvars("$I3_BUILD")+"/IceHive/resources","example.i3.bz2"))

icetray.set_log_level(icetray.I3LogLevel.LOG_INFO)
#icetray.set_log_level_for_unit("IceHive", icetray.I3LogLevel.LOG_DEBUG)
#icetray.set_log_level_for_unit("I3IceHive", icetray.I3LogLevel.LOG_DEBUG)
#icetray.set_log_level_for_unit("HiveSplitter", icetray.I3LogLevel.LOG_DEBUG)
#icetray.set_log_level_for_unit("I3HiveCleaning", icetray.I3LogLevel.LOG_DEBUG)
#icetray.set_log_level_for_unit("HiveCleaning", icetray.I3LogLevel.LOG_DEBUG)

tray = I3Tray()

#read some file
tray.AddModule("I3Reader","Reader",
        FilenameList=[os.path.join(i3_testdata+"/IceHive","example_scenario.i3.bz2")])
#remove all P frames and thereby reduce it
tray.AddModule(lambda f: not f.Stop == icetray.I3Frame.Physics)

#remove keys which might possibly collide
tray.AddModule("Delete", "delete_keys_in_use",
               Keys = ["MaskedOfflinePulses"+"_Physics",
                       "MaskedOfflinePulses"+"_Noise",
                       "MaskedOfflinePulses"+"_Noised",
                       "IceHiveSplit"+"SplitCount"])

#tray.AddModule("Dump", "dump")

#split the Q frame into P-frames
from icecube import IceHive

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

cleanVicinity = IceHive.RingLimits()
cleanVicinity.AddLimitPair(IceHive.LimitPair(-70., 70.))
cleanVicinity.AddLimitPair(IceHive.LimitPair(-70., 70.))

#clean the output pulses in the P-frame
tray.AddModule("I3HiveCleaning<I3RecoPulse>","HiveClean",
  InputName = "MaskedOfflinePulses",
  OutputName = "HC"+"MaskedOfflinePulses",
  TimeStaticMinus= 600.*I3Units.ns,
  TimeStaticPlus= 600.*I3Units.ns,
  SingleDenseRingVicinity=cleanVicinity,
  DoubleDenseRingVicinity=cleanVicinity,
  TripleDenseRingVicinity=cleanVicinity,
  Stream = icetray.I3Frame.Physics,
  If = lambda f: True)

#make a reconstruction in the clean Pulses in the P frame
from icecube import linefit
tray.AddSegment(linefit.simple, 'LineFit_HC',
  inputResponse = "HCMaskedOfflinePulses",
  fitName = "LineFit",
  If = lambda f: True)

tray.AddModule("I3Writer","Writer",
  Filename=os.path.join(os.path.expandvars("$I3_BUILD")+"/IceHive/resources","example.i3.bz2"),
  Streams=[icetray.I3Frame.Geometry, icetray.I3Frame.Physics, icetray.I3Frame.DAQ])


tray.Execute()

