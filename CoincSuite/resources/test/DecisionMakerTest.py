#!/usr/bin/env python
 
"""
UNITTEST for DecisionMaker
"""

import os, sys
from I3Tray import *
from icecube import icetray, dataio, dataclasses
from icecube import CoincSuite

import unittest

#=== Some Globals ===
from icecube.icetray import I3Units
GCDfile = os.path.expandvars("$I3_TESTDATA/GCD/GeoCalibDetectorStatus_IC86.55697_corrected_V2.i3.gz")
SplitName = "split"
HypoName = "hypoframe"
OrgPulses = "OfflinePulses"
SplitPulses = "MaskedOfflinePulses"

class ScenarioSource(icetray.I3Module):
  """ Create a hypoframe combinable Scenario : GCDQPP, where GCD are read from a file"""
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
    #make a Q-frame
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
    Hframe = icetray.I3Frame(icetray.I3Frame.Physics)
    Heh = dataclasses.I3EventHeader()
    Heh.start_time = (dataclasses.I3Time(2011, 0))
    Heh.end_time = (dataclasses.I3Time(2011, 2))
    Heh.run_id = 1
    Heh.event_id = 1
    Heh.sub_event_stream = HypoName
    Heh.sub_event_id = 0
    Hframe.Put("I3EventHeader", Heh)
    Hrecomap = dataclasses.I3RecoPulseSeriesMap()
    Hrecomap[icetray.OMKey(1,1)] = [recopulse1]
    Hrecomap[icetray.OMKey(2,2)] = [recopulse2]
    Hrecomask = dataclasses.I3RecoPulseSeriesMapMask(Qframe, OrgPulses, Hrecomap)
    Hframe.Put(SplitPulses, Hrecomask)
    Hcf = dataclasses.I3MapStringVectorDouble()
    Hcf[SplitName]=[0,1]
    Hframe.Put("CS_CreatedFrom", Hcf)
    Hrs = dataclasses.I3MapStringBool()
    Hrs["XTest"]=True
    Hframe.Put("CS_RecombSuccess", Hrs)
    self.PushFrame(Hframe)
    
    self.RequestSuspension()


#__________________________________________________
class TestDaqSequence(unittest.TestCase):
  """ define what the test-case should actually see """
  def testSequence(self):
    if (self.frame.Stop == icetray.I3Frame.DAQ): #just to be sure
      self.assert_(self.frame[SplitName+"ReducedCount"].value==1)

#__________________________________________________
pframes = 0
class TestPhysicsSequence(unittest.TestCase):
  """ define what the test-case should actually see """
  def __init__(self, methodName='runTest'):
    super(TestPhysicsSequence,self).__init__(methodName)
    
  def testSequence(self):
    global pframes
    """ the physics frame arrive in the sequence split(0), split(1), hypo(0), split(2) """
    if (self.frame.Stop == icetray.I3Frame.Physics): #just to be sure
      if (pframes == 0): #the first split-frame that has been recombined and is redundant
        eh = self.frame["I3EventHeader"]
        self.assert_(eh.sub_event_stream==SplitName and eh.sub_event_id==0)
        self.assert_(self.frame.Has("CS_ReducedBy"))
        rb = self.frame["CS_ReducedBy"]
        self.assert_(rb[HypoName][0]==0)
        self.assert_(self.frame.Has("CS_ReducedWith"))
        rw = self.frame["CS_ReducedWith"]
        self.assert_(rw[SplitName][0]==1.)
      elif (pframes == 1): #the second split-frame that has been recombined and is redundant
        eh = self.frame["I3EventHeader"]
        self.assert_(eh.sub_event_stream==SplitName and eh.sub_event_id==1)
        self.assert_(self.frame.Has("CS_ReducedBy"))
        rb = self.frame["CS_ReducedBy"]
        self.assert_(rb[HypoName][0]==0)
        self.assert_(self.frame.Has("CS_ReducedWith"))
        rw = self.frame["CS_ReducedWith"]
        self.assert_(rw[SplitName][0]==1)
      elif (pframes == 2): #the hypoframe which is found to be the right recombination hypothesis
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
        self.assert_(self.frame.Has("CS_Reducing"))
        red = self.frame["CS_Reducing"]
        self.assert_(red[SplitName][0]==0)
        self.assert_(red[SplitName][1]==1)
      elif (pframes == 3): #the recombined event from the split-frames
        self.assert_(self.frame.Has("I3EventHeader"))
        eh = self.frame["I3EventHeader"]
        self.assert_(eh.sub_event_stream==SplitName and eh.sub_event_id==2)
        self.assert_(self.frame.Has(SplitPulses))
        recomask = self.frame[SplitPulses]
        self.assert_(isinstance(recomask, dataclasses.I3RecoPulseSeriesMapMask))
        self.assert_(recomask.source == OrgPulses)
        recomap = dataclasses.I3RecoPulseSeriesMap.from_frame(self.frame, SplitPulses)
        self.assert_(len(recomap)==2)
        self.assert_(recomap.keys()[0]==OMKey(1,1))
        self.assert_(recomap.keys()[1]==OMKey(2,2))
        self.assert_(self.frame.Has("CS_ComboFrom"))
        cf = self.frame["CS_ComboFrom"]
        self.assert_(len(cf[SplitName])==2)
        self.assert_(cf.keys()[0]==SplitName)
        self.assert_(len(cf[SplitName])==2)
        self.assert_(cf[SplitName][0]==0)
        self.assert_(cf[SplitName][1]==1)
      else:
        icetray.logging.log_fatal("to many frames")

      pframes +=1


#=== TRAY ===
from icecube import icetray, dataio
from I3Tray import *
tray = I3Tray()

tray.AddModule(ScenarioSource, "ScenarioSource",
                GCDfile = GCDfile)

tray.AddModule("DecisionMaker", "FinalDecision",
  SplitName = SplitName,
  HypoName = HypoName,
  LikeNameList = [],
  VetoNameList = [],
  TrueNameList = ["XTest"],
  Discard = False)

tray.AddModule(icetray.I3TestModuleFactory(TestPhysicsSequence), "TestDaqSequence",
                Streams=[icetray.I3Frame.DAQ])
tray.AddModule(icetray.I3TestModuleFactory(TestPhysicsSequence), "TestPhysicsSequence",
                Streams=[icetray.I3Frame.Physics])


tray.Execute()

