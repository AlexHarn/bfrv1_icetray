#!/usr/bin/python

from icecube import icetray, dataclasses
from icecube.icetray import I3Units
#from icecube.CoincSuite import which_split
from icecube.phys_services.which_split import which_split
from icecube.icetray import pypick


#==============================================================================
class SplitCountMaker(icetray.I3PacketModule):
  """
  A helper to make the SplitCount available as an I3Int in the Q-frames
  Puts a key <I3Int>(SplitName+"ReducedCount")==0 into the frame
  """
  def __init__(self, ctx):
    icetray.I3PacketModule.__init__(self, ctx, icetray.I3Frame.DAQ)
    self.AddParameter("SplitName","Name of the Splitter and its SubEventStream","toposplit")
    self.AddOutBox("OutBox")
  def Configure(self):
    icetray.I3PacketModule.Configure(self)
    self.splitname = self.GetParameter("SplitName")
  def FramePacket(self,frames):
    count =0
    for frame in frames:
      if (frame.Stop == icetray.I3Frame.Physics):
        if (frame.Has("I3EventHeader")):
          if (frame["I3EventHeader"].sub_event_stream == self.splitname):
            count +=1
    frames[0].Put(self.splitname+"SplitCount", icetray.I3Int(count))
    for frame in frames:
      self.PushFrame(frame)
    return

#==============================================================================
class ReducedCountMaker(icetray.I3PacketModule):
  """
  A helper to make the ReducedCount available as an I3Int in the Q-frames.
  Does put a key <I3Int>(SplitName+"ReducedCount")==0 into the frame.
  """
  def __init__(self, ctx):
    icetray.I3PacketModule.__init__(self, ctx, icetray.I3Frame.DAQ)
    self.AddParameter("SplitName","Name of the Splitter and its SubEventStream","toposplit")
    self.AddOutBox("OutBox")
  def Configure(self):
    icetray.I3PacketModule.Configure(self)
    self.splitName = self.GetParameter("SplitName")
  def FramePacket(self,frames):
    reducedValue = 0
    qframe = frames[0]
    if (qframe.Has(self.splitName+"SplitCount")):
      splits = qframe[self.splitName+"SplitCount"].value
      nsplitframes = 0 
      for frame in frames:
        if (frame.Has("I3EventHeader")):
          if (frame["I3EventHeader"].sub_event_stream== self.splitName):
            nsplitframes+=1
      reducedValue = splits-nsplitframes
      if (reducedValue<0):
        icetray.logging.log_fatal("Something is wrong with your SplitCount-variable")
    qframe.Put(self.splitName+"ReducedCount", icetray.I3Int(reducedValue))
    for frame in frames:
      self.PushFrame(frame)
    return
  
#==============================================================================
@icetray.traysegment
def Complete(tray, name,
  suffix = '',
  SplitName = '',
  SplitPulses = '',
  FitName = 'LineFit'):
  """
  A segment doing everything that CoincSuite possibly provide.
  Run after your splitter, which has written the SplitFrames and SplitCount

  :param suffix: right now an unused option FIXME appendable suffix
  :param SplitName: name of the subevent-stream, identical to SplitterModule
  :param SplitPulses: name of the split PulseSeries in the subevent-stream
  :param FitName: Name of that will be given to the LineFit
  """
  icetray.logging.log_info("Using CoincSuite Recombinations")

  if (SplitName==''):
    icetray.logging.log_fatal("Configure CSComplete Traysegment Parameter 'Splitname'")
  if (SplitPulses==''):
    icetray.logging.log_fatal("Configure CSComplete Traysegment Parameter 'Pulses'")

  tray.AddModule(lambda f: f.Put(SplitName+"ReducedCount", icetray.I3Int(0)), "ReducedCountMaker",
    Streams = [icetray.I3Frame.DAQ])

  tray.AddModule("AfterpulseDiscard", "AfterpulseDiscard",
    SplitName = SplitName,
    RecoMapName = SplitPulses,
    QTotFraction = .1,
    TimeOffset = 3.E3*I3Units.ns,
    OverlapFraction = 0.75,
    Discard = True)

  tray.AddModule("HypoFrameCreator", "HypoFrameCreator",
    SplitName = SplitName,
    HypoName = "hypoframe",
    RecoMapName = SplitPulses,
    MaxTimeSeparation = 3000.*I3Units.ns)

  from icecube import linefit
  tray.AddSegment( linefit.simple, 'LineFit',
    inputResponse = SplitPulses,
    fitName = FitName,
    If = which_split(SplitName) | which_split('hypoframe') )

  tray.AddModule("TrackSystemTester", "TestHypoTrackSystem",
    SplitName = SplitName,
    HypoName = "hypoframe",
    RecoMapName = SplitPulses,
    HypoFitName = FitName,
    CriticalRatio = 0.7, #0.8
    CylinderRadius = 150*I3Units.meter,
    ResTimeWindow = dataclasses.make_pair(-200,200),
    ParticleSpeed = dataclasses.I3Constants.c)
  
  tray.AddModule("TrackSystemTester", "TestMutualTrackSystem",
    SplitName = SplitName,
    HypoName = "hypoframe",
    RecoMapName = SplitPulses,
    RecoFitName = FitName,
    CriticalRatio = 0.7, #0.8
    CylinderRadius = 150*I3Units.meter,
    ResTimeWindow = dataclasses.make_pair(-200*I3Units.ns,200*I3Units.ns),
    ParticleSpeed = dataclasses.I3Constants.c,
    MutualCompare=True)

  tray.AddModule("AlignmentTester", "TestHypoAlignment",
    SplitName = SplitName,
    HypoName = "hypoframe",
    HypoFitName = FitName,
    RecoFitName = FitName,
    CriticalAngle = 25*I3Units.degree,
    CriticalDistance = 20*I3Units.meter)

  tray.AddModule("AlignmentTester", "TestMutualAlignment",
    SplitName = SplitName,
    HypoName = "hypoframe",
    HypoFitName = FitName,
    RecoFitName = FitName,
    CriticalAngle = 25*I3Units.degree,
    CriticalDistance = 20*I3Units.meter,
    MutualCompare=True)

  tray.AddModule("SpeedTester","TestSpeed",
    SplitName = SplitName,
    HypoName = "hypoframe",
    HypoFitName = FitName,
    SpeedUpperCut = 0.35*I3Units.m/I3Units.ns, #1.17*c
    SpeedLowerCut = 0.15*I3Units.m/I3Units.ns) #0.5*c

  tray.AddModule("cogCausalConnectTester", "TestCogCausalConnect",
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

  from icecube import lilliput
  import icecube.lilliput.segments
  mininame = lilliput.segments.add_minuit_simplex_minimizer_service(tray)
  paraname = lilliput.segments.add_simple_track_parametrization_service(tray)
  llhname  = lilliput.segments.add_pandel_likelihood_service(tray,SplitPulses,'SPE1st',10.*I3Units.hertz)

  tray.AddModule("ReducingLikelihoodTester", "TestHypoReducingLikelihood",
    SplitName = SplitName,
    HypoName = "hypoframe",
    HypoFitName = FitName,
    RecoFitName = FitName,
    LlhName = llhname,
    MiniName = mininame,
    ParaName = paraname,
    ReductionFactor = 0.8,
    Refit = False)

  tray.AddModule("ReducingLikelihoodTester", "TestMutualReducingLikelihood",
    SplitName = SplitName,
    HypoName = "hypoframe",
    HypoFitName = FitName,
    RecoFitName = FitName,
    LlhName = llhname,
    MiniName = mininame,
    ParaName = paraname,
    ReductionFactor = 0.8,
    Refit = False,
    MutualCompare = True)

  tray.AddModule("DecisionMaker", "FinalDecision",
    SplitName = SplitName,
    HypoName = "hypoframe",
    RecoMapName = SplitPulses,
    LikeNameList = ["TestHypoTrackSystem",
      "TestMutualTrackSystem",
      "TestHypoAlignment",
      "TestMutualAlignment",
      "TestHypoReducingLikelihood",
      "TestMutualReducingLikelihood"],
    VetoNameList = ["TestCogCausalConnect","TestSpeed"],
    TrueNameList = [],
    Discard = True)
    
  tray.AddModule( lambda f: f["I3EventHeader"].sub_event_stream!="hypoframe", "KillHypoFrame")

  tray.AddSegment(linefit.simple,'LineFit_repeat',
    inputResponse = SplitPulses,
    fitName = FitName,
    If = which_split(SplitName) & pypick(lambda f: not f.Has(FitName)) )

  tray.AddModule("Delete", "Delete",
    Keys=[])# NOTE MAYBE remove unused keys here


#==============================================================================
class createTimeWindow(icetray.I3ConditionalModule):
  """
  Determine the time window defined by first and last pulse in 'InputPulses' 
  and extend it by 'offest' ns at the beginning and end
  """
  def __init__(self, context):
    icetray.I3ConditionalModule.__init__(self, context)
    self.AddParameter("InputPulses", "Name of InputPulses", "")
    self.AddParameter("Offset", "Extend the TimeRange by this much at the start and end time", 0.)
    self.AddParameter("Output", "Output; left unconfigured output is '[InputPulses]+TimeRange'", "")
    self.AddOutBox('OutBox')
  def Configure(self):
    icetray.I3ConditionalModule.Configure(self)
    self.inputPulses = self.GetParameter("InputPulses")
    self.offset = self.GetParameter("Offset")
    self.output = self.GetParameter("Output")
    if (self.output==""):
      self.output=self.inputPulses+"TimeRange"
  def Physics(self, frame):
    time_pulses = []
    pulses = dataclasses.I3RecoPulseSeriesMap.from_frame(frame, self.inputPulses)
    maxTime=-1.e100
    minTime=1.e100
    for item in pulses:
      for pulse in item[1]:
        if pulse.time<minTime:
          minTime=pulse.time
        if pulse.time>maxTime:
          maxTime=pulse.time
    if maxTime<minTime:
      frame.Put(self.output, dataclasses.I3TimeWindow(-self.offset, self.offset))
    else:
      frame.Put(self.output, dataclasses.I3TimeWindow(minTime-self.offset, maxTime+self.offset))
    self.PushFrame(frame)


#=========================================
class discardEmptySplits(icetray.I3PacketModule):
      """
      Discard Splits there the frame is effectively discardEmptySplits
      """
      def __init__(self, ctx):
        icetray.I3PacketModule.__init__(self, ctx, icetray.I3Frame.DAQ)
        self.AddOutBox("OutBox")
        self.AddParameter("SplitName", "Name of the SubEventStream to process on", "")
        self.AddParameter("PulsesName", "Name of the Pulses to operate on", "")
      def Configure(self):
        icetray.I3PacketModule.Configure(self)
        self.splitName = self.GetParameter("SplitName")
        self.pulsesName = self.GetParameter("PulsesName")
      def Finish(self):
        icetray.I3PacketModule.Finish(self)
      def FramePacket(self,frames):
        n_reduced = 0
        oframes = []
        for frame in frames:
          if (frame.Has("I3EventHeader") and frame["I3EventHeader"].sub_event_stream==self.splitName):
            r = dataclasses.I3RecoPulseSeriesMap.from_frame(frame, self.pulsesName)
            keep = True
            if (len(r)==0):
              keep = False
            else:
              for item in r:
                keep = False
                if (len(item[1])):
                  keep = True
                  break

            if (keep):
              oframes.append(frame)
            else:
              n_reduced += 1
          else:
            oframes.append(frame)

        for frame in oframes:
          if (frame.Stop == icetray.I3Frame.DAQ):
            red_value = frame[self.splitName+"ReducedCount"].value
            frame.Delete(self.splitName+"ReducedCount")
            frame.Put(self.splitName+"ReducedCount", icetray.I3Int((red_value+n_reduced) ) )
          self.PushFrame(frame)
