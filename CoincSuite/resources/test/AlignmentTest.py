#!/usr/bin/env python
 
"""
UNITTEST for Alignment Test: Create a artificial series of testframes and check if the processing goes through
"""

import os, sys
from I3Tray import *
from icecube import icetray, dataio, dataclasses
from icecube import CoincSuite

import unittest

#=== Some Globals ===
from icecube.icetray import I3Units
GCDfile = os.path.expandvars("$I3_TESTDATA/sim/GeoCalibDetectorStatus_IC86.55697_corrected_V2.i3.gz")
CriticalAngle = 25.*I3Units.deg
CriticalDistance = 20.*I3Units.m
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
    P1eh.sub_event_stream = "split"
    P1eh.sub_event_id = 0
    P1frame.Put("I3EventHeader", P1eh)
    P1recomap = dataclasses.I3RecoPulseSeriesMap()
    P1recomap[icetray.OMKey(1,1)] = [recopulse1]
    P1recomask = dataclasses.I3RecoPulseSeriesMapMask(Qframe, OrgPulses, Qrecomap)
    P1frame.Put(SplitPulses, P1recomask)
    P1fit = dataclasses.I3Particle()
    P1fit.pos= dataclasses.I3Position(0.,0., CriticalDistance)
    P1fit.dir= dataclasses.I3Direction(0., CriticalAngle)
    P1fit.fit_status = dataclasses.I3Particle.OK
    P1frame.Put("Fit", P1fit)
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
    P2recomap[icetray.OMKey(2,2)] = [recopulse2]
    P2recomask = dataclasses.I3RecoPulseSeriesMapMask(Qframe, OrgPulses, P2recomap)
    P2frame.Put(SplitPulses, P2recomask)
    P2fit = dataclasses.I3Particle()
    P2fit.pos= dataclasses.I3Position(0.,0., CriticalDistance)
    P2fit.dir= dataclasses.I3Direction(0., -CriticalAngle)
    P2fit.fit_status = dataclasses.I3Particle.OK
    P2frame.Put("Fit", P2fit)
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
      self.assert_(ra[0] == "MutualAlignmentTester")
      self.assert_(ra[1] == "HypoAlignmentTester")
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
        self.assert_(rs.keys()==["HypoAlignmentTester", "MutualAlignmentTester"])
        self.assert_(rs["MutualAlignmentTester"]==True)
        self.assert_(rs["HypoAlignmentTester"]==True)
      else:
        log_fatal("to many frames")

      self.pframes +=1

#================ TRAY ==============
from I3Tray import *
tray = I3Tray()

tray.AddModule(ScenarioSource, "ScenarioSource",
                GCDfile = GCDfile)

from icecube import CoincSuite
tray.AddModule("AlignmentTester", "MutualAlignmentTester",
  SplitName = SplitName,
  HypoName = HypoName,
  HypoFitName = "HFit",
  RecoFitName = "Fit",
  CriticalAngle = 2*CriticalAngle,
  CriticalDistance = CriticalDistance,
  MutualCompare=True)

tray.AddModule("AlignmentTester", "HypoAlignmentTester",
  SplitName = SplitName,
  HypoName = HypoName,
  HypoFitName = "HFit",
  RecoFitName = "Fit",
  CriticalAngle = CriticalAngle,
  CriticalDistance = CriticalDistance,
  MutualCompare=False)

tray.AddModule(icetray.I3TestModuleFactory(TestDaqSequence), "TestDaqSequence",
                  Streams=[icetray.I3Frame.DAQ])
  
tray.AddModule(icetray.I3TestModuleFactory(TestPhysicsSequence), "TestPhysicsSequence",
                  Streams=[icetray.I3Frame.Physics])


tray.Execute()



