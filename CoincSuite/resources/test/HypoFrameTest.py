#!/usr/bin/env python
 
"""
UNITTEST for HypoFrameCreator
"""

import os, sys
from I3Tray import *
from icecube import icetray, dataio, dataclasses
from icecube import CoincSuite

import unittest

#=== Some Globals ===
from icecube.icetray import I3Units
GCDfile = os.path.expandvars("$I3_TESTDATA/sim/GeoCalibDetectorStatus_IC86.55697_corrected_V2.i3.gz")
SplitName = "split"
HypoName = "hypoframe"
OrgPulses = "OfflinePulses"
SplitPulses = "MaskedOfflinePulses"

class ScenarioSource(icetray.I3Module):
  """ Create a ypoframe combinable Scenario : GCDQPP, where GCD are read from a file"""
  def __init__(self, context):
    super(ScenarioSource,self).__init__(context)
    self.gcdfile = self.AddParameter("GCDfile", "A GCD file to push in front", "")
    self.AddOutBox("OutBox")
  def Configure(self):
    self.gcdfile = self.GetParameter("GCDfile")
  def Process(self):
    gcd = dataio.I3File(self.gcdfile, 'R')
    while (gcd.more()):
      frame = gcd.pop_frame()
      self.PushFrame(frame)
    gcd.close()
    #now deliver artificial testcase
    #make a Q-frame#make a Q-frame
    Qframe = icetray.I3Frame(icetray.I3Frame.DAQ)
    Qeh = dataclasses.I3EventHeader()
    Qeh.start_time = (dataclasses.I3Time(2011, 0))
    Qeh.end_time = (dataclasses.I3Time(2011, 2))
    Qeh.run_id = 1
    Qeh.event_id = 1
    Qframe.Put("I3EventHeader", Qeh)
    Qrecomap = dataclasses.I3RecoPulseSeriesMap()
    recopulse1 = dataclasses.I3RecoPulse()
    recopulse1.time = 0
    recopulse1.charge = 1
    recopulse2 = dataclasses.I3RecoPulse()
    recopulse2.time = 1
    recopulse2.charge = 2
    Qrecomap[icetray.OMKey(1,1)] = [recopulse1]
    Qrecomap[icetray.OMKey(2,2)] = [recopulse2]
    Qframe.Put(OrgPulses, Qrecomap)
    Qframe.Put(SplitName+"SplitCount", icetray.I3Int(2))
    Qframe.Put(SplitName+"ReducedCount", icetray.I3Int(0))
    self.PushFrame(Qframe)
    #now make the first p-frame containing one I3RecoPulse
    P1frame = icetray.I3Frame(icetray.I3Frame.Physics)
    P1eh = dataclasses.I3EventHeader()
    P1eh.start_time = (dataclasses.I3Time(2011, 0))
    P1eh.end_time = (dataclasses.I3Time(2011, 1))
    P1eh.run_id = 1
    P1eh.event_id = 1
    P1eh.sub_event_stream = SplitName
    P1eh.sub_event_id = 0
    P1frame.Put("I3EventHeader", P1eh)
    P1recomap = dataclasses.I3RecoPulseSeriesMap()
    P1recomap[icetray.OMKey(1,1)] = [recopulse1]
    P1recomask = dataclasses.I3RecoPulseSeriesMapMask(Qframe, OrgPulses, Qrecomap)
    P1frame.Put(SplitPulses, P1recomask)
    self.PushFrame(P1frame)
    #now make the second p-frame containing one I3RecoPulse
    P2frame = icetray.I3Frame(icetray.I3Frame.Physics)
    P2eh = dataclasses.I3EventHeader()
    P2eh.start_time = (dataclasses.I3Time(2011, 1))
    P2eh.end_time = (dataclasses.I3Time(2011, 2))
    P2eh.run_id = 1
    P2eh.event_id = 1
    P2eh.sub_event_stream = SplitName
    P2eh.sub_event_id = 1
    P2frame.Put("I3EventHeader", P2eh)
    P2recomap = dataclasses.I3RecoPulseSeriesMap()
    P2recomap[icetray.OMKey(2,2)] = [recopulse2]
    P2recomask = dataclasses.I3RecoPulseSeriesMapMask(Qframe, OrgPulses, P2recomap)
    P2frame.Put(SplitPulses, P2recomask)
    self.PushFrame(P2frame)
    
    self.RequestSuspension()


#__________________________________________________
class TestPhysicsSequence(unittest.TestCase):
  """ define what the test-case should actually see """
  def __init__(self, methodName='runTest'):
    super(TestPhysicsSequence,self).__init__(methodName)
    self.pframes = 0
  
  def testSequence(self):
    if (self.frame.Stop == icetray.I3Frame.Physics): #just to be sure
      if (self.pframes == 0):
        eh = self.frame["I3EventHeader"]
        self.assert_(eh.sub_event_stream==SplitName and eh.sub_event_id==0)
      elif (self.pframes == 1):
        eh = self.frame["I3EventHeader"]
        self.assert_(eh.sub_event_stream==SplitName and eh.sub_event_id==1)
      elif (self.pframes == 2):
        self.assert_(self.frame.Has("I3EventHeader"))
        eh = self.frame["I3EventHeader"]
        self.assert_(eh.sub_event_stream==HypoName and eh.sub_event_id==0)
        self.assert_(self.frame.Has("MaskedOfflinePulses"))
        recomask = self.frame["MaskedOfflinePulses"]
        self.assert_(isinstance(recomask, dataclasses.I3RecoPulseSeriesMapMask))
        self.assert_(recomask.source == "OfflinePulses")
        recomap = dataclasses.I3RecoPulseSeriesMap.from_frame(self.frame, "MaskedOfflinePulses")
        self.assert_(len(recomap)==2)
        self.assert_(recomap.keys()[0]==OMKey(1,1))
        self.assert_(recomap.keys()[1]==OMKey(2,2))
        self.assert_(self.frame.Has("CS_CreatedFrom"))
        cf = self.frame["CS_CreatedFrom"]
        self.assert_(len(cf)==1)
        self.assert_(cf.keys()[0]==SplitName)
        self.assert_(len(cf[SplitName])==2)
        self.assert_(cf[SplitName][0]==0)
        self.assert_(cf[SplitName][1]==1)
      else:
        log_fatal("to many frames")

      self.pframes +=1

#=== TRAY ===
from icecube import icetray, dataio
from I3Tray import *
tray = I3Tray()

tray.AddModule(ScenarioSource, "ScenarioSource",
                GCDfile = GCDfile)

tray.AddModule("HypoFrameCreator", "HypoFrameCreator",
  SplitName = SplitName,
  HypoName = HypoName,
  RecoMapName = SplitPulses,
  MaxTimeSeparation = 3000.*I3Units.ns)

tray.AddModule(icetray.I3TestModuleFactory(TestPhysicsSequence), "TestPhysicsSequence",
                Streams=[icetray.I3Frame.Physics])
  

tray.Execute()

