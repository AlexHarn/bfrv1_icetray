#!/usr/bin/env python

"""
A script demonstrating the procedure, if it should be attempted to apply a
event-splitter during any later stage of data processing.

NOTE: This is intended as a oientation help, code might probably be outdated
"""


from icecube import icetray, dataclasses

#uncomment; plug in your most favourite, but very noise module for debugging
#icetray.set_log_level_for_unit('I3IceHive', icetray.I3LogLevel.LOG_DEBUG)

@icetray.traysegment
def resplit(tray, name,
  pulses= "OfflinePulses",
  splitpulses= "MaskedOfflinePulses",
  org_split_name = "toposplit"):
  """
  It works like this:
  (Splitter has to be rerun in front)
  tag the frame that corresponds to the split that was remaining in the old split-stream
  then do the recombination
  see if new frames have been recombined into the frame in question.
  If so move the new recombined frame at the place of the original frame in question and substitute it

  :param pulses: source of all pulses
  :param splitpulses: the split and to split pulses
  :param org_split_name: name of the split-stream that shouldbe evaluated
  """
  #tray.AddModule("Delete", "del", Keys=["ResplitSplitCount", "ResplitReducedCount", "ThisOne"])

  #A very noisy module, kill on sight
  from icecube.CoincSuite import Stepper
  tray.AddModule(Stepper, "stepper")

  class Loop(icetray.I3PacketModule):
    def __init__(self, ctx):
      icetray.I3PacketModule.__init__(self, ctx, icetray.I3Frame.DAQ)
      self.AddOutBox("OutBox")
    def Configure(self):
      icetray.I3PacketModule.Configure(self)
    def Finish(self):
      icetray.I3PacketModule.Finish(self)
    def FramePacket(self,frames):
      for frame in frames:
          #if not frame["I3EventHeader"].sub_event_stream=="redo":
          self.PushFrame(frame)
  #tray.AddModule(Loop, "loop") #enable this module in case something was wrong with the stream of frame that we got

  SplitName = "Resplit"
  from icecube import IceHive
  tray.AddModule("I3IceHive",SplitName,
    InputName = pulses,
    OutputName = splitpulses,
    Multiplicity=4,
    TimeWindow=2000*I3Units.ns,
    TimeCVMinus=300*I3Units.ns, #DANGER
    TimeCVPlus=300*I3Units.ns, #DANGER
    TimeCNMinus=200*I3Units.ns,
    TimeCNPlus=200*I3Units.ns,
    TimeStaticMinus=200*I3Units.ns,
    TimeStaticPlus=200*I3Units.ns,
    SingleDenseRingLimits=[255., 255., 272.7, 272.7, 165.8, 165.8], #I3Units.m
    DoubleDenseRingLimits=[70., 70., 131.5, 131.5, 40.8, 40.8], #I3Units.m
    TrippleDenseRingLimits=[70., 70., 144.1, 144.1, 124.7, 124.7, 82.8, 82.8], #I3Units.m
    SingleDenseRingVicinity=[100.,100.,100.,100.], #I3Units.m
    DoubleDenseRingVicinity=[100.,100.,100.,100.,100.,100.], #I3Units.m
    TrippleDenseRingVicinity=[100.,100.,100.,100.,100.,100.,100.,100.], #I3Units.m
    Mode = 4,
    SaveSplitCount=True,)
    #TrigHierName= "QTriggerHierarchy",
    #TriggerConfigIDs= [1006, 1007, 1011, 21001], #[(SMT8),(string),(SMT3),(volume)]
    #NoSplitDt = 10000,
    #ReadoutWindowMinus = 4000,
    #ReadoutWindowPlus = 6000)

  from icecube.CoincSuite import which_split
  from icecube import IceHive
  tray.AddModule("I3HiveCleaning","HiveClean",
    InputName = splitpulses,
    OutputName = "HC"+splitpulses,
    TimeStaticMinus=600*I3Units.ns,
    TimeStaticPlus=600*I3Units.ns,
    SingleDenseRingVicinity=[70.,70.,70.,70.,], #I3Units.m
    DoubleDenseRingVicinity=[70.,70.,70.,70.], #, 72., 72.], #I3Units.m
    TrippleDenseRingVicinity=[70.,70.,70.,70.,], #52.,52.], #I3Units.m
    If = lambda f: True)#which_split(f, SplitName) )

  #clip away the first and the last hit DOM, this is for quality
  #NOTE you can a static cleaning for this for example IceHive/HiveCleaning
  from icecube import PartialCOG
  tray.AddModule("I3PulseClipper","I3PulseClipper",
    InputName = splitpulses,
    OutputName = "Clipped"+splitpulses,
    ClipHead = 1,
    ClipTail = 1,
    TimeLimit = 1000, #600
    Mode = 2,
    If = lambda f: which_split(f, SplitName))

  from icecube.CoincSuite import resplit
  tray.AddModule(resplit.moveObjects, "MoveObjects",
    OriginalStream = org_split_name,
    TargetStream = SplitName,
    MoveObjects = [], #NOTE put your favourite fits here
    FlagName = "ThisOne")

  #==============================
  # A round of CoincSuite
  #==============================
  from icecube import CoincSuite
  from icecube.CoincSuite import which_split
  tray.AddModule(lambda f: f.Put(SplitName+"ReducedCount", icetray.I3Int(0)), "ReducedCountMaker",
    Streams = [icetray.I3Frame.DAQ])

  tray.AddModule("HypoFrameCreator", "SHypoFrameCreator",
    SplitName = SplitName,
    HypoName = "hypoframe",
    RecoMapName = 'HC'+splitpulses, #DANGER these are no motherpulses, aka the lowest unaltered map/mask of pulses
    MaxTimeSeparation = 3000.*I3Units.ns,
    FlagName = "ThisOne")

  from icecube import linefit, lilliput
  tray.AddSegment( linefit.simple,'LineFit_Masked',
    inputResponse = "HC"+splitpulses,
    fitName = 'LineFit_Masked',
    If = lambda f: which_split(f, split_name=SplitName) or which_split(f, split_name='hypoframe'))

  tray.AddModule(CoincSuite.Stepper, "Stepper")

  tray.AddModule("CylinderPulsesTester", "TestMutualCylinderPulses",
    SplitName = SplitName,
    HypoName = "hypoframe",
    RecoMapName = 'HC'+splitpulses,
    HypoFitName = "LineFit_Masked",
    RecoFitName = "LineFit_Masked",
    CriticalRatio = 0.7, #0.8
    CylinderRadius = 300.*I3Units.meter, #previously 150.
    MutualCompare = True)

  tray.AddModule("SpeedTester","TestSpeed",
    SplitName = SplitName,
    HypoName = "hypoframe",
    HypoFitName = "LineFit_Masked",
    SpeedUpperCut = 0.35*I3Units.m/I3Units.ns,
    SpeedLowerCut = 0.15*I3Units.m/I3Units.ns)

  tray.AddModule("cogCausalConnectTester", "TestcogCausalConnect",
    SplitName = SplitName,
    HypoName = "hypoframe",
    RecoMapName = 'HC'+splitpulses,
    HypoFitName = "LineFit_Masked",
    TimeConeMinus = 1000*I3Units.ns,
    TimeConePlus = 1000*I3Units.ns,
    WallTime = 3000*I3Units.ns,
    MaxVerticalDist = 800*I3Units.m, #previously 700
    MaxHorizontalDist = 800*I3Units.m) #previously 700

  from icecube import lilliput, gulliver
  tray.AddService("I3GulliverIPDFPandelFactory","SPE1stLLh",
    InputReadout = 'HC'+splitpulses,
    Likelihood = "SPE1st",
    PEProb = "GaussConvoluted",
    NoiseProbability = 1.e4*I3Units.hertz,
    IceModel = 2,
    AbsorptionLength = 98.0*I3Units.m,
    JitterTime = 4.0*I3Units.ns)

  tray.AddModule("TrackLikelihoodTester", "TestHypoTrackLikelihood",
    SplitName = SplitName,
    HypoName = "hypoframe",
    LogLikelihoodService = "SPE1stLLh",
    FitName = "LineFit_Masked")

  tray.AddModule("TrackLikelihoodTester", "TestMutualTrackLikelihood",
    SplitName = SplitName,
    HypoName = "hypoframe",
    LogLikelihoodService = "SPE1stLLh",
    FitName = "LineFit_Masked",
    MutualCompare = True)

  tray.AddModule("DecisionMaker", "FinalDecision",
    SplitName = SplitName,
    HypoName = "hypoframe",
    RecoMapName = "HC"+splitpulses,
    LikeNameList = ["TestMutualCylinderPulses",
      "TestHypoTrackLikelihood",
      "TestMutualTrackLikelihood",
      "TestcogCausalConnect"],
    VetoNameList = ["TestcogCausalConnect",
                    "TestSpeed",])

  #tray.AddModule(resplit.integrateResplit, "MoveObjectsBack",
    #OriginalStream = org_split_name,
    #TargetStream = SplitName,
    #MoveObjects = [], #NOTE
    #FlagName = "ThisOne")

  tray.AddModule(lambda f: not(which_split(f,split_name=SplitName) and f.Has("FinalDecision")), "KillRecombinedFrame")

  tray.AddModule(lambda f: not(which_split(f,split_name="hypoframe")), "KillHypoFrame")

  tray.AddSegment(linefit.simple,'LineFit_AGAIN',
    inputResponse = "HC"+splitpulses,
    fitName = 'LineFit_Masked',
    If = lambda f: which_split(f, split_name=SplitName) and not f.Has("LineFit_Masked"))

#___________________IF STANDALONE__________________________
if (__name__=='__main__'):
  from optparse import OptionParser

  usage = 'usage: %prog [options]'
  parser = OptionParser(usage)

  parser.add_option("-i", "--input", action="store", type="string", default="", dest="INPUT", help="Input i3 file to process")
  parser.add_option("-o", "--output", action="store", type="string", default="", dest="OUTPUT", help="Output i3 file")
  parser.add_option("-g", "--gcd", action="store", type="string", default="", dest="GCD", help="GCD file for input i3 file")
  parser.add_option("-n", "--nevents", action="store", type="int", default=0, dest="NEVENTS", help="Number of Events to process")

  (options,args) = parser.parse_args()

  from icecube import icetray, dataio
  from I3Tray import *
  tray = I3Tray()

  tray.AddModule("I3Reader","reader",
    filenamelist = [options.GCD, options.INPUT],)

  tray.AddSegment(resplit, "resplit")

  tray.AddModule("I3Writer","writer",
    streams = [icetray.I3Frame.DAQ,icetray.I3Frame.Physics],
    filename = options.OUTPUT)

  

  if (options.NEVENTS==0):
    tray.Execute()
  else:
    tray.Execute(options.NEVENTS)

  
