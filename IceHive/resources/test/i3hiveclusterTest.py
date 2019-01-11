#!/usr/bin/env python 

"""
This is really not a unit test but a regression test, ensuring that this project's splitter continues
to produce exactly the same output as the original HiveSplitter. If any change is made to the
algorithm this test will fail, and if the change is intentional the test data will need to be regenerated
to put this test back on track.
This script can also be used to generate the testcase itself by specifying the number of output FramePackets as second argument,
 i.e. $ ./i3hiveclusterTest.py 100
"""
from icecube import icetray, dataclasses

class I3HiveClusterVerifier(icetray.I3PacketModule):
  """An I3Module which compares that two different cleanings produce exactly the same results"""
  def __init__(self, context):
    super(I3HiveClusterVerifier, self).__init__(context, icetray.I3Frame.DAQ)
    self.frameCount = 0
    self.orgName = "org"
    self.redoName = "redo"
    self.pulsesName = "MaskedOfflinePulses"
    self.AddParameter("OriginalName", "Name of the original sub-eventstream", self.orgName)
    self.AddParameter("RedoName", "Name of the reprocessed sub-eventstream", self.redoName)
    self.AddOutBox("OutBox")
  def Configure(self):
    self.orgName = self.GetParameter("OriginalName")
    self.redoName = self.GetParameter("RedoName")
  def MarkAndPush(self, frame):
    frame.Put("DIFF", icetray.I3Bool(True))
    self.PushFrame(frame)
  def Physics(self, frame):
    self.frameCount+=1
    
    #next, step through the split frames in parallel and make sure that the the split pulse series are identical
    #compare these objects:
    # - RecoPulseSeries

    orgMap = dataclasses.I3RecoPulseSeriesMap.from_frame(frame, self.orgName)
    redoMap = dataclasses.I3RecoPulseSeriesMap.from_frame(frame, self.redoName)
  
    #should be the same number of DOMs present in both maps
    if (len(orgMap) != len(redoMap)):
      icetray.logging.log_error("Regression: Different numbers of hit DOMs in frame "+str(self.frameCount)+", split frame "+str(frameCount)+":\n"\
        +" original= "+str(len(orgMap))+", current="+str(len(redoMap)))
      self.MarkAndPush(frames)
      return
    
    #iterate over all of the DOMs and ensure that the same DOMs are hit, and have the same pulses
    for k in range(len(orgMap)):
      #check that both pulse series are for the same DOM
      orgOMKey = orgMap.keys()[k]
      redoOMKey = redoMap.keys()[k]
      if (orgOMKey != redoOMKey):
        icetray.logging.log_error("Regression: Missmatched DOMs in frame "+str(self.frameCount)+", split frame "+str(frameCount)+":\n"\
          +" original= "+str(orgOMKey)+", current="+str(redoOMKey))
        self.MarkAndPush(frames)
        return
      #check that the DOM has the same number of pulses in both series
      orgPulses = orgMap.values()[k]
      redoPulses = redoMap.values()[k]
      if (len(orgPulses) != len(redoPulses)):
        icetray.logging.log_error("Regression: Different numbers of hits on DOM "+str(orgOMKey)+" in frame "+str(self.frameCount)+", split frame "+str(frameCount)+":\n"\
          +" original= "+str(len(orgPulses))+", current="+str(len(redoPulses)))
        self.MarkAndPush(frames)
        return
      
      #iterate over all of the pulses and check that they are the same
      for p in range(len(orgPulses)):
        orgPulse = orgPulses[p]
        redoPulse = redoPulses[p]
        if (len(orgPulses) != len(redoPulses)):
          icetray.logging.log_error("Regression: Different pulse "+str(p)+" on DOM "+str(orgOMKey)+" in frame "+str(self.frameCount)+", split frame "+str(frameCount)+"!")
          self.MarkAndPush(frames)
          return
    self.PushFrame(frame)

class FrameDelivery(icetray.I3PacketModule):
  """A module that just deliveres a specified number of packages"""
  def __init__(self, context):
    super(FrameDelivery, self).__init__(context, icetray.I3Frame.DAQ)
    self.AddParameter("NRequested", "Deliver that many frame-packets", 100)
    self.AddOutBox("OutBox")
  def Configure(self):
    self.nrequested = self.GetParameter("NRequested")
    self.ndelivered = 0
  def FramePacket(self, frames):
    if (self.ndelivered<self.nrequested):
      for frame in frames:
        self.PushFrame(frame)
      self.ndelivered+=1
    else:
      self.RequestSuspension()
    
import unittest
#define when there should a loud screem
class TestEqual(unittest.TestCase):
  """ define what the test-case should actually see """
  def testSequence(self):
    if (self.frame.Stop == icetray.I3Frame.Physics):
      self.assert_(not self.frame.Has("DIFF"))

#tray
import sys
from icecube import dataio
from I3Tray import *

from icecube import IceHive

#running this with with a number
if (len(sys.argv) ==1):
  test = True
elif (len(sys.argv) ==2):
  test = False
else:
  raise RuntimeError("specify either a single number or no option at all")
#find testdata
if (os.environ.get("I3_TESTDATA")):
  i3_testdata = (os.path.expandvars("$I3_TESTDATA"))
else:
  i3_testdata = os.path.expandvars("$I3_TESTDATA")


tray = I3Tray()

if (test):
  tray.AddModule("I3Reader","Reader",
    Filename = os.path.join(i3_testdata,"IceHive/hivecluster_testcase.i3.bz2"))
else:
  tray.AddModule("I3Reader","Reader",
    filenamelist = ["/data/exp/IceCube/2011/filtered/level2/0513/Level2_IC86.2011_data_Run00118178_0513_GCD.i3.gz",
                    "/data/exp/IceCube/2011/filtered/level2/0513/Level2_IC86.2011_data_Run00118178_Part00000000.i3.bz2"])
if (not test):
  tray.AddModule(lambda f: (f.Stop==icetray.I3Frame.Physics and f["I3EventHeader"].sub_event_stream=="in_ice"))
  
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


#allow a unique name to the modules
if (test):
  redo_org = 'redo'
else:
  redo_org = 'org'

tray.AddModule("I3HiveCluster<I3RecoPulse>", redo_org,
  InputName = "MaskedOfflinePulses",
  OutputName = redo_org,
  Stream = icetray.I3Frame.Physics)
  
if (test):
  tray.AddModule(I3HiveClusterVerifier,"CleaningVerifier")

  tray.AddModule(icetray.I3TestModuleFactory(TestEqual), "TestEqual",
                Streams=[icetray.I3Frame.DAQ])

if (not test):
  tray.AddModule("Delete", "del",
                 Keys = ["MaskedOfflinePulses_Physics",
                         "MaskedOfflinePulses_Noise"])
  
  
  tray.AddModule(FrameDelivery, "deliver",
                 NRequested = int(sys.argv[1]))
  
  tray.AddModule("I3Writer", "Writer",
                  Filename= os.path.join(os.path.expandvars("$I3_BUILD"),"IceHive/resources/hivecluster_testcase.i3.bz2"),
                  Streams = [icetray.I3Frame.Geometry,
                            #icetray.I3Frame.Calibration,
                            #icetray.I3Frame.DetectorStatus,
                            icetray.I3Frame.DAQ,
                            icetray.I3Frame.Physics])



tray.Execute()


