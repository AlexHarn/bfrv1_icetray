#!/usr/bin/env python
 
"""
UNITTEST for AfterpulseTester: Create a artificial series of testframes and check if the processing goes through
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
    recopulse1.time = 0.
    recopulse1.charge = 1.
    recopulse2 = dataclasses.I3RecoPulse()
    recopulse2.time = 100.
    recopulse2.charge = 1.
    Qrecomap[icetray.OMKey(1,1)] = [recopulse1, recopulse2]
    Qrecomap[icetray.OMKey(2,2)] = [recopulse1]
    Qrecomap[icetray.OMKey(3,3)] = [recopulse1, recopulse2]
    Qrecomap[icetray.OMKey(4,4)] = [recopulse1]
    Qrecomap[icetray.OMKey(5,5)] = [recopulse1, recopulse2]
    Qrecomap[icetray.OMKey(6,6)] = [recopulse1]
    Qrecomap[icetray.OMKey(7,7)] = [recopulse1, recopulse2]
    Qrecomap[icetray.OMKey(8,8)] = [recopulse1]
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
    P1eh.sub_event_stream = "split"
    P1eh.sub_event_id = 0
    P1frame.Put("I3EventHeader", P1eh)
    P1recomap = dataclasses.I3RecoPulseSeriesMap()
    P1recomap[icetray.OMKey(1,1)] = [recopulse1]
    P1recomap[icetray.OMKey(2,2)] = [recopulse1]
    P1recomap[icetray.OMKey(3,3)] = [recopulse1]
    P1recomap[icetray.OMKey(4,4)] = [recopulse1]
    P1recomap[icetray.OMKey(5,5)] = [recopulse1]
    P1recomap[icetray.OMKey(6,6)] = [recopulse1]
    P1recomap[icetray.OMKey(7,7)] = [recopulse1]
    P1recomap[icetray.OMKey(8,8)] = [recopulse1]
    P1recomask = dataclasses.I3RecoPulseSeriesMapMask(Qframe, OrgPulses, P1recomap)
    P1frame.Put(SplitPulses, P1recomask)
    self.PushFrame(P1frame)
    #now make the second p-frame containing one I3RecoPulse
    P2frame = icetray.I3Frame(icetray.I3Frame.Physics)
    P2eh = dataclasses.I3EventHeader()
    P2eh.start_time = (dataclasses.I3Time(2011, 1))
    P2eh.end_time = (dataclasses.I3Time(2011, 2))
    P2eh.run_id = 1
    P2eh.event_id = 1
    P2eh.sub_event_stream = "split"
    P2eh.sub_event_id = 1
    P2frame.Put("I3EventHeader", P2eh)
    P2recomap = dataclasses.I3RecoPulseSeriesMap()
    P2recomap[icetray.OMKey(1,1)] = [recopulse2]
    P2recomap[icetray.OMKey(3,3)] = [recopulse2]
    P2recomap[icetray.OMKey(5,5)] = [recopulse2]
    P2recomap[icetray.OMKey(7,7)] = [recopulse2]
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
    Hrecomap[icetray.OMKey(1,1)] = [recopulse1, recopulse2]
    Hrecomap[icetray.OMKey(2,2)] = [recopulse1]
    Hrecomap[icetray.OMKey(3,3)] = [recopulse1, recopulse2]
    Hrecomap[icetray.OMKey(4,4)] = [recopulse1]
    Hrecomap[icetray.OMKey(5,5)] = [recopulse1, recopulse2]
    Hrecomap[icetray.OMKey(6,6)] = [recopulse1]
    Hrecomap[icetray.OMKey(7,7)] = [recopulse1, recopulse2]
    Hrecomap[icetray.OMKey(8,8)] = [recopulse1]
    Hrecomask = dataclasses.I3RecoPulseSeriesMapMask(Qframe, OrgPulses, Hrecomap)
    Hframe.Put(SplitPulses, Hrecomask)
    Hcf = dataclasses.I3MapStringVectorDouble()
    Hcf["split"]=[0,1]
    Hframe.Put("CS_CreatedFrom", Hcf)
    Hfit = dataclasses.I3Particle()
    Hfit.time= 0
    Hfit.pos= dataclasses.I3Position(0.,0.,0.)
    Hfit.dir= dataclasses.I3Direction(0., 0.)
    Hfit.fit_status = dataclasses.I3Particle.OK
    Hframe.Put("HFit", Hfit)
    self.PushFrame(Hframe)
    
    self.RequestSuspension()


class TestDaqSequence(unittest.TestCase):
  """ define what the test-case should actually see """
  def testSequence(self):
    if (self.frame.Stop == icetray.I3Frame.DAQ):
      self.assert_(self.frame.Has(SplitName+"RecombAttempts"))
      ra = self.frame[SplitName+"RecombAttempts"]
      self.assert_(ra[0] == "AfterpulseTester")
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
        self.assert_(eh.sub_event_stream=="split" and eh.sub_event_id==0)
      elif (self.pframes == 1):
        eh = self.frame["I3EventHeader"]
        self.assert_(eh.sub_event_stream=="split" and eh.sub_event_id==1)
      elif (self.pframes == 2):
        eh = self.frame["I3EventHeader"]
        self.assert_(eh.sub_event_stream=="hypoframe" and eh.sub_event_id==0)
        self.assert_(self.frame.Has("CS_RecombSuccess"))
        rs = self.frame["CS_RecombSuccess"]
        self.assert_(rs.keys()==["AfterpulseTester"])
        self.assert_(rs["AfterpulseTester"]==True)
      else:
        log_fatal("to many frames")

      self.pframes +=1

#================ TRAY ==============
from I3Tray import *
tray = I3Tray()

tray.AddModule(ScenarioSource, "ScenarioSource",
                GCDfile = GCDfile)

tray.AddModule("AfterpulseTester","AfterpulseTester",
  SplitName = SplitName,
  HypoName = "hypoframe",
  RecoMapName = "MaskedOfflinePulses",
  QTotFraction = 0.51,
  TimeOffset = 100.*I3Units.ns,
  OverlapFraction = 0.5)

tray.AddModule(icetray.I3TestModuleFactory(TestDaqSequence), "TestDaqSequence",
                  Streams=[icetray.I3Frame.DAQ])
  
tray.AddModule(icetray.I3TestModuleFactory(TestPhysicsSequence), "TestPhysicsSequence",
                  Streams=[icetray.I3Frame.Physics])


tray.Execute()



