#!/usr/bin/env python
 
"""
UNITTEST for CoincSuiteHelpers: Create a artificial series of testframes and check if the processing goes through
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
    self.AddOutBox("OutBox")
  def Configure(self):
    pass
  def Process(self):
    """ deliver frames QP with only a bit of rudamentary information """
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
    Qframe.Put(SplitName+"SplitCount", icetray.I3Int(1))
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
    self.PushFrame(P1frame)
    
    self.RequestSuspension()


class TestDaqSequence(unittest.TestCase):
  """ define what the test-case should actually see """
  def testSequence(self):
    if (self.frame.Stop == icetray.I3Frame.DAQ):
      self.assert_(self.frame.Has(SplitName+"SplitCount"))
      self.assert_(self.frame["Fake"+"SplitCount"].value == 0)
      self.assert_(self.frame.Has(SplitName+"ReducedCount"))
      self.assert_(self.frame[SplitName+"ReducedCount"].value == 0)
#__________________________________________________
class TestPhysicsSequence(unittest.TestCase):
  """ define what the test-case should actually see """
  def __init__(self, methodName='runTest'):
    super(TestPhysicsSequence,self).__init__(methodName)
  def testSequence(self):
    if (self.frame.Stop == icetray.I3Frame.Physics): #just to be sure
      self.assert_(self.frame.Has(SplitPulses+"TimeRange"))
      tw = self.frame[SplitPulses+"TimeRange"]
      self.assert_(tw.start==0)
      self.assert_(tw.stop==1)

#================ TRAY ==============
from I3Tray import *
tray = I3Tray()

tray.AddModule(ScenarioSource, "ScenarioSource")

from icecube import CoincSuite
tray.AddModule(CoincSuite.SplitCountMaker, "SplitCount",
               SplitName = "Fake")
tray.AddModule(CoincSuite.ReducedCountMaker, "ReducedCount",
               SplitName = SplitName)
tray.AddModule(CoincSuite.createTimeWindow, "TimeWindow",
               InputPulses = SplitPulses)

tray.AddModule(icetray.I3TestModuleFactory(TestDaqSequence), "TestDaqSequence",
                  Streams=[icetray.I3Frame.DAQ])
  
tray.AddModule(icetray.I3TestModuleFactory(TestPhysicsSequence), "TestPhysicsSequence",
                  Streams=[icetray.I3Frame.Physics])


tray.Execute()



