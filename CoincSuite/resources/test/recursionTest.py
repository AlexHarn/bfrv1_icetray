#!/usr/bin/env python 

"""
This is really not a unit test but a regression test, ensuring that this project's splitter continues
to produce exactly the same output as the original HiveSplitter. If any change is made to the
algorithm this test will fail, and if the change is intentional the test data will need to be regenerated
to put this test back on track.
This script can also be used to generate the testcase itself by specifying the number of output FramePackets as second argument,
 i.e. $ ./recursionTest.py 100
"""
from icecube import icetray, dataclasses, lilliput
from icecube.icetray import I3Units
from icecube.dataclasses import I3Constants
import icecube.lilliput.segments
from icecube.phys_services.which_split import which_split

SplitPulses = "MaskedOfflinePulses"
SplitName = "split"
FitName = "LFit"

import unittest
tester_list= ["Afterpulse",
              "Speed",
              "HypoAlignment",
              "MutualAlignment",
              "HypoTrackSystem",
              "MutualTrackSystem",
              "HypoReducingLikelihood",
              "MutualReducingLikelihood",
              "impCausalConnect",
              "cogCausalConnect"]

#__________________________________________________
class TestDaqSequence(unittest.TestCase):
  """ define what the test-case should actually see in teh DAQ-frame"""
  def testSequence(self):
    if (self.frame.Stop == icetray.I3Frame.DAQ):
      self.assert_(self.frame.Has(SplitName+"RecombAttempts"))
      ra = self.frame[SplitName+"RecombAttempts"]

      for tester in tester_list:
        self.assert_("org"+tester in ra, "the original module "+"org"+tester+" has not been run on this frame")
        self.assert_("redo"+tester in ra, "the redo module "+"redo"+tester+" has not been run on this frame")
#__________________________________________________
class TestPhysicsSequence(unittest.TestCase):
  """ define what the test-case should actually see """
  def __init__(self, methodName='runTest'):
    super(TestPhysicsSequence,self).__init__(methodName) 
  def testSequence(self):
    if (self.frame.Stop == icetray.I3Frame.Physics): #just to be sure
      eh = self.frame["I3EventHeader"]
      if (eh.sub_event_stream=='hypoframe'):
        self.assert_(self.frame.Has("CS_RecombSuccess"))
        rs = self.frame["CS_RecombSuccess"]

        for tester in tester_list:
          both_found=((("org"+tester) in rs) and (("redo"+tester) in rs)) 
          none_found = ((("org"+tester) not in rs) and (("redo"+tester) not in rs))
          self.assert_(none_found or both_found, "for tester "+tester+" not both, original and redo Module, have been run")
          if (both_found):
            if not rs["org"+tester]==rs["redo"+tester] :
              print(rs["org"+tester])
              print(rs["redo"+tester])
            self.assert_(rs["org"+tester]==rs["redo"+tester], "the decision by the original and redo Module for "+tester+" differ")

#=============== TRAY ===========================
import sys
from icecube import dataio
from I3Tray import *

from icecube import CoincSuite

#running this with with a number
if (len(sys.argv) ==1):
  test = True
elif (len(sys.argv) ==2):
  test = False
else:
  raise RuntimeError("specify either a single number or no option at all")

tray = I3Tray()

if (test):
  tray.AddModule("I3Reader","Reader",
    Filename = os.path.expandvars("$I3_TESTDATA") + "/CoincSuite/coincsuite_testcase.i3.bz2" )
#    Filename = os.path.expandvars("$I3_SRC") + "/CoincSuite/resources/coincsuite_testcase.i3.bz2")
else:
  tray.AddModule("I3Reader","Reader",
    filenamelist = ["/data/exp/IceCube/2011/filtered/level2/0513/Level2_IC86.2011_data_Run00118178_0513_GCD.i3.gz",
                    "/data/exp/IceCube/2011/filtered/level2/0513/Level2_IC86.2011_data_Run00118178_Part00000000.i3.bz2"])

  tray.AddModule(lambda f: not (f.Stop==icetray.I3Frame.Physics and (f["I3EventHeader"].sub_event_stream != SplitName) ))
  
  tray.AddModule("Keep", "keep",
                 Keys = ["I3Geometry",
                         #"I3Calibration",
                         #"I3DetectorStatus",
                         "I3TriggerHierarchy",
                         "CleanTriggerHierarchy",
                         "FilterMask",
                         "I3EventHeader",
                         "I3DST11",
                         "I3SuperDST",
                         "OfflinePulses",
                         "MaskedOfflinePulses"])

  from icecube import IceHive
  tray.AddModule("I3IceHive<I3RecoPulse>", SplitName,
    InputName="OfflinePulses",
    OutputName=SplitPulses,
    SaveSplitCount=True)
  
  tray.AddModule(lambda f: f.Put(SplitName+"ReducedCount", icetray.I3Int(0)), "ReducedCountMaker",
    Streams = [icetray.I3Frame.DAQ])

  tray.AddModule("HypoFrameCreator", "HypoFrameCreator",
    SplitName = SplitName,
    HypoName = "hypoframe",
    RecoMapName = SplitPulses,
    MaxTimeSeparation = 3000.*I3Units.ns)

  from icecube import linefit, lilliput
  tray.AddSegment( linefit.simple, 'LineFit',
    inputResponse = SplitPulses,
    fitName = FitName,
    If = which_split(SplitName) | which_split('hypoframe'))


#=== A bunch of tester modules ===
#allow a unique name to the modules
if (test):
  redo_org = 'redo'
else:
  redo_org = 'org'

tray.AddModule("AfterpulseTester", redo_org+"Afterpulse",
  SplitName = SplitName,
  HypoName = "hypoframe",
  RecoMapName = SplitPulses,
  QTotFraction = .1,
  TimeOffset = 3.E3*I3Units.ns,
  OverlapFraction = 0.75)
  
tray.AddModule("SpeedTester", redo_org+"Speed",
  SplitName = SplitName,
  HypoName = "hypoframe",
  HypoFitName = FitName,
  SpeedUpperCut = 1.17*I3Constants.c,
  SpeedLowerCut = 0.5*I3Constants.c)

tray.AddModule("AlignmentTester", redo_org+"HypoAlignment",
  SplitName = SplitName,
  HypoName = "hypoframe",
  HypoFitName = FitName,
  RecoFitName = FitName,
  CriticalAngle = 25*I3Units.degree,
  CriticalDistance = 20*I3Units.meter)

tray.AddModule("AlignmentTester", redo_org+"MutualAlignment",
  SplitName = SplitName,
  HypoName = "hypoframe",
  HypoFitName = FitName,
  RecoFitName = FitName,
  CriticalAngle = 25*I3Units.degree,
  CriticalDistance = 20*I3Units.meter,
  MutualCompare = True)

mininame = lilliput.segments.add_minuit_simplex_minimizer_service(tray)
paraname = lilliput.segments.add_simple_track_parametrization_service(tray)
llhname  = lilliput.segments.add_pandel_likelihood_service(tray,'MaskedOfflinePulses','SPE1st',10.*I3Units.hertz)

tray.AddModule("ReducingLikelihoodTester", redo_org+"HypoReducingLikelihood",
  SplitName = SplitName,
  HypoName = "hypoframe",
  HypoFitName = FitName,
  RecoFitName = FitName,
  LlhName = llhname,
  MiniName = mininame,
  ParaName = paraname,
  ReductionFactor = 0.8,
  Refit = False)

tray.AddModule("ReducingLikelihoodTester", redo_org+"MutualReducingLikelihood",
  SplitName = SplitName,
  HypoName = "hypoframe",
  HypoFitName = FitName,
  RecoFitName = FitName,
  LlhName = llhname,
  MiniName = mininame,
  ParaName = paraname,
  ReductionFactor = 0.8,
  MutualCompare = True,
  Refit = False)

tray.AddModule("cogCausalConnectTester", redo_org+"cogCausalConnect",
  SplitName = SplitName,
  HypoName = "hypoframe",
  RecoMapName = SplitPulses,
  HypoFitName = FitName,
  TravelTimeResidual = dataclasses.make_pair(-1000.*I3Units.ns,1000.*I3Units.ns),
  WallTime = 3000.*I3Units.ns,
  MaxVerticalDist = 700.*I3Units.m,
  MaxHorizontalDist = 700.*I3Units.m,
  MaxTrackDist = 200.*I3Units.m,
  MaxFurthestDist = 600.*I3Units.m)

tray.AddModule("impCausalConnectTester", redo_org+"impCausalConnect",
  SplitName = SplitName,
  HypoName = "hypoframe",
  RecoMapName = SplitPulses,
  HypoFitName = FitName,
  TravelTimeResidual = dataclasses.make_pair(-1000.*I3Units.ns,1000.*I3Units.ns),
  WallTime = 3000.*I3Units.ns,
  MaxVerticalDist = 700.*I3Units.m,
  MaxHorizontalDist = 700.*I3Units.m)
  
tray.AddModule("TrackSystemTester", redo_org+"HypoTrackSystem",
  SplitName = SplitName,
  HypoName = "hypoframe",
  RecoMapName = SplitPulses,
  HypoFitName = FitName,
  CriticalRatio = 0.7, #0.8
  CylinderRadius = 150*I3Units.meter,
  ResTimeWindow = dataclasses.make_pair(-200,200),
  ParticleSpeed = dataclasses.I3Constants.c)

tray.AddModule("TrackSystemTester", redo_org+"MutualTrackSystem",
  SplitName = SplitName,
  HypoName = "hypoframe",
  RecoMapName = SplitPulses,
  RecoFitName = FitName,
  CriticalRatio = 0.7, #0.8
  CylinderRadius = 150*I3Units.meter,
  ResTimeWindow = dataclasses.make_pair(-200*I3Units.ns,200*I3Units.ns),
  ParticleSpeed = dataclasses.I3Constants.c,
  MutualCompare=True)  
  
if (test):
  tray.AddModule(icetray.I3TestModuleFactory(TestDaqSequence), "TestDaqSequence",
                    Streams=[icetray.I3Frame.DAQ])
    
  tray.AddModule(icetray.I3TestModuleFactory(TestPhysicsSequence), "TestPhysicsSequence",
                    Streams=[icetray.I3Frame.Physics])

else:  
  
  class DropNoCoincSuitePackets(icetray.I3PacketModule):
    """ drop every frame-packet that is not a good testcase """
    def __init__(self, context):
      icetray.I3PacketModule.__init__(self, context, icetray.I3Frame.DAQ)
      self.AddParameter('NRequested', "Deliver that many frame-packets", 1000)
      self.AddOutBox("OutBox")
    def Configure(self):
      self.nrequested = self.GetParameter("NRequested")
      self.ndelivered = 0
    def FramePacket(self, frames):
      if (self.ndelivered<self.nrequested):
        if (frames[0].Has(SplitName+"RecombAttempts")):
          for frame in frames:
            self.PushFrame(frame)
          self.ndelivered+=1
      else:
        self.RequestSuspension()

  tray.AddModule(DropNoCoincSuitePackets, "DropUNwantedStuff",
                 NRequested = int(sys.argv[1]))

  tray.AddModule("I3Writer", "Writer",
                  Filename= os.path.join(os.path.expandvars("$I3_BUILD"),"CoincSuite/resources/coincsuite_testcase.i3.bz2"),
                  Streams = [icetray.I3Frame.Geometry,
                            #icetray.I3Frame.Calibration,
                            #icetray.I3Frame.DetectorStatus,
                            icetray.I3Frame.DAQ,
                            icetray.I3Frame.Physics])
                  


tray.Execute()


