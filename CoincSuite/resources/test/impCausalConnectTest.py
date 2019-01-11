#!/usr/bin/env python
 
"""
UNITTEST for impCausalConnectTester: Create a artificial series of testframes and check if the processing goes through
"""

import os, sys
from I3Tray import *
from icecube import icetray, dataio, dataclasses
from icecube import CoincSuite

import unittest

#=== Some Globals ===
from icecube.icetray import I3Units
from icecube.dataclasses import I3Constants
GCDfile = os.path.expandvars("$I3_TESTDATA/sim/GeoCalibDetectorStatus_IC86.55697_corrected_V2.i3.gz")
CriticalAngle = 25.*I3Units.deg
CriticalDistance = 20.*I3Units.m
SplitName = "split"
HypoName = "hypoframe"
OrgPulses = "OfflinePulses"
SplitPulses = "MaskedOfflinePulses"

class ScenarioSource(icetray.I3Module):
  """ Create a ypoframe combinable Scenario : GCDQPP, where GCD are read from a file
  create a testcase, where the pulses are distributed on the inner ring of the detector center
  arround string 36, that are strings 26, 27, 25, 27, 45, 46
  make one cluster at the top (OM 1) and one at the bottom (OM 60) of the detector
  """
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
      if (frame.Stop==icetray.I3Frame.Geometry):
        geo = frame["I3Geometry"]
      self.PushFrame(frame)
    gcd.close()
    #now deliver artificial testcase
    #make a Q-frame
    Qrecomap = dataclasses.I3RecoPulseSeriesMap()
    recopulse1 = dataclasses.I3RecoPulse()
    recopulse1.time = 0.
    recopulse1.charge = 1.
    recopulse2 = dataclasses.I3RecoPulse()
    recopulse2.time = 1000./I3Constants.c #=3335.ns
    recopulse2.charge = 2.
    Qrecomap[icetray.OMKey(26,1)] = [recopulse1]
    Qrecomap[icetray.OMKey(27,1)] = [recopulse1]
    Qrecomap[icetray.OMKey(35,1)] = [recopulse1]
    Qrecomap[icetray.OMKey(37,1)] = [recopulse1]
    Qrecomap[icetray.OMKey(45,1)] = [recopulse1]
    Qrecomap[icetray.OMKey(46,1)] = [recopulse1]
    Qrecomap[icetray.OMKey(26,60)] = [recopulse2]
    Qrecomap[icetray.OMKey(27,60)] = [recopulse2]
    Qrecomap[icetray.OMKey(35,60)] = [recopulse2]
    Qrecomap[icetray.OMKey(37,60)] = [recopulse2]
    Qrecomap[icetray.OMKey(45,60)] = [recopulse2]
    Qrecomap[icetray.OMKey(46,60)] = [recopulse2]
    
    Qframe = icetray.I3Frame(icetray.I3Frame.DAQ)
    Qeh = dataclasses.I3EventHeader()
    Qeh.start_time = (dataclasses.I3Time(2011, 0))
    Qeh.end_time = (dataclasses.I3Time(2011, 2))
    Qeh.run_id = 1
    Qeh.event_id = 1
    Qframe.Put("I3EventHeader", Qeh)

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
    
    P1recomap[icetray.OMKey(26,1)] = [recopulse1]
    P1recomap[icetray.OMKey(27,1)] = [recopulse1]
    P1recomap[icetray.OMKey(35,1)] = [recopulse1]
    P1recomap[icetray.OMKey(37,1)] = [recopulse1]
    P1recomap[icetray.OMKey(45,1)] = [recopulse1]
    P1recomap[icetray.OMKey(46,1)] = [recopulse1]

    P1recomask = dataclasses.I3RecoPulseSeriesMapMask(Qframe, OrgPulses, P1recomap)
    P1frame.Put(SplitPulses, P1recomask)
    P1fit = dataclasses.I3Particle(dataclasses.I3Particle.ParticleShape.InfiniteTrack)
    P1fit.time= 0 -455.*I3Units.ns
    P1fit.pos=geo.omgeo[icetray.OMKey(36,1)].position
    P1fit.dir= dataclasses.I3Direction(0., 0.) #straight down
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
    P2eh.sub_event_stream = SplitName
    P2eh.sub_event_id = 1
    P2frame.Put("I3EventHeader", P2eh)
    P2recomap = dataclasses.I3RecoPulseSeriesMap()
    
    P2recomap[icetray.OMKey(26,60)] = [recopulse2]
    P2recomap[icetray.OMKey(27,60)] = [recopulse2]
    P2recomap[icetray.OMKey(35,60)] = [recopulse2]
    P2recomap[icetray.OMKey(37,60)] = [recopulse2]
    P2recomap[icetray.OMKey(45,60)] = [recopulse2]
    P2recomap[icetray.OMKey(46,60)] = [recopulse2]

    P2recomask = dataclasses.I3RecoPulseSeriesMapMask(Qframe, OrgPulses, P2recomap)
    P2frame.Put(SplitPulses, P2recomask)
    
    P2fit = dataclasses.I3Particle(dataclasses.I3Particle.ParticleShape.InfiniteTrack)
    P2fit.time =  1000./I3Constants.c -455.*I3Units.ns
    P2fit.pos=geo.omgeo[icetray.OMKey(36,60)].position
    P2fit.dir= dataclasses.I3Direction(0., 0.) #straight up
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

    Hrecomap[icetray.OMKey(26,1)] = [recopulse1] #ring top
    Hrecomap[icetray.OMKey(27,1)] = [recopulse1]
    Hrecomap[icetray.OMKey(35,1)] = [recopulse1]
    Hrecomap[icetray.OMKey(37,1)] = [recopulse1]
    Hrecomap[icetray.OMKey(45,1)] = [recopulse1]
    Hrecomap[icetray.OMKey(46,1)] = [recopulse1]
    Hrecomap[icetray.OMKey(26,60)] = [recopulse2] #ring bottom
    Hrecomap[icetray.OMKey(27,60)] = [recopulse2]
    Hrecomap[icetray.OMKey(35,60)] = [recopulse2]
    Hrecomap[icetray.OMKey(37,60)] = [recopulse2]
    Hrecomap[icetray.OMKey(45,60)] = [recopulse2]
    Hrecomap[icetray.OMKey(46,60)] = [recopulse2]

    Hrecomask = dataclasses.I3RecoPulseSeriesMapMask(Qframe, OrgPulses, Hrecomap)
    Hframe.Put(SplitPulses, Hrecomask)
    Hcf = dataclasses.I3MapStringVectorDouble()
    Hcf["split"]=[0,1]
    Hframe.Put("CS_CreatedFrom", Hcf)
    Hfit = dataclasses.I3Particle(dataclasses.I3Particle.ParticleShape.InfiniteTrack)
    Hfit.time = -454.*I3Units.ns
    Hfit.pos=geo.omgeo[icetray.OMKey(36,1)].position
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
      self.assert_(ra[0] == "impCausalConnectTester")
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
        self.assert_(rs.keys()==["impCausalConnectTester"])
        self.assert_(rs["impCausalConnectTester"]==True)
      else:
        log_fatal("to many frames")

      self.pframes +=1

#================ TRAY ==============
from I3Tray import *
tray = I3Tray()

icetray.logging.set_level(icetray.I3LogLevel.LOG_INFO)

tray.AddModule(ScenarioSource, "ScenarioSource",
                GCDfile = GCDfile)

tray.AddModule("impCausalConnectTester", "impCausalConnectTester",
  SplitName = SplitName,
  HypoName = "hypoframe",
  RecoMapName = SplitPulses,
  HypoFitName = "HFit",
  TravelTimeResidual = dataclasses.make_pair(-20.*I3Units.ns,20.*I3Units.ns),
  WallTime = 1005.*I3Units.m/I3Constants.c,
  MaxVerticalDist = 1005.*I3Units.m,
  MaxHorizontalDist = 1.*I3Units.m)

tray.AddModule(icetray.I3TestModuleFactory(TestDaqSequence), "TestDaqSequence",
                Streams=[icetray.I3Frame.DAQ])
 
tray.AddModule(icetray.I3TestModuleFactory(TestPhysicsSequence), "TestPhysicsSequence",
                Streams=[icetray.I3Frame.Physics])


tray.Execute()



