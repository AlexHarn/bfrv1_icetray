"""
Segments are guide-lines no absolutes;
Parameter settings for the contained modules might not be optimal for your specific analysis
please review the the parameter settings, before use
If not sure, look at the documentation or contact me: marcel.zoll@fysik.su.se
"""

from icecube import icetray

@icetray.traysegment
def Split_and_Recombine(tray, name,
                        LineFitName = "LineFit",
                        SplitName = "IceHiveSplit",
                        OriginalPulses = "InIcePulses",
                        SplitPulses = "SplitInIcePulses",
                        CleanedSplitPulsesName = "CleanSplitInIcePulses",
                        DesignatedPulsesName = "RecoInIcePulses", 
                        TriggerHierarchyName = "QTriggerHierarchy",
                        InIceTriggerIDs = [1006, 1007, 1011, 21001], #[(SMT8),(string),(SMT3),(volume)]
                        AddTimeWindow = True,
                        Clean = True,
                        LineFitReco = True,
                        Recombine = True,
                        Recover = False,
                        ):
  """
  A tray-segment to perform event-splitting and event-recombination at low-level in data processing
  
  :param DesignatedPulsesName: The end product pulses will be written to this key
  :param LineFitName: The end produce LineFit will be written to this key
  :param OriginalPulses: Name of the original pulse in the Q-frame that will be split
  :param SplitName: Name of the splitter and therefore the subevent-stream that is delivered
  :param SplitPulses: Name of the SplitPulses in the P-frame
  :param TriggerHierarchyName: Name of the TriggerHierarchy in the Q-frame
  :param LineFitName: Name of that the LineFit reconstruction will be given
  :param AddTimeWindow: Set to True if a time window should be added (needed for EHE filter, I3TriggerSplitter does this too)
  :param CleanedSplitPulsesName: Name of the Cleaned Pulses if option 'Clean' is enabled
  :param Clean: Choose if a round of HiveCleaning (similar to RT-Cleaning) should commence on the output-pulses; improves reconstructability 
  :param LineFitReco: make a importvedLineFit reconstruction; needed for Recombine and Recover option
  :param Recombine: recombine wrongly split events with CoincSuite
  :param Recover: try to recover lost pulses, by tracing back the track
  
  The Output are Q-frames containing:
  -[SplitNameSplitCount]: The number of created P-frames (SplitFrames) by the splitter
  -[SplitNameReducedCount]: The number of removed splits by means of recombinations; [SplitNameSplitCount]-[SplitNameReducedCount] is the effective remaining number of SplitFrames
  -[SplitPulses_Physics]: All clustered Pulses in the OriginalPulseSeries; identical to the union of all SplitPulses in the SubFrames
  -[SplitPulses_Noise]: All not clustered Pulses in the OriginalPulseSeries; identical to the inversion of SplitPulses_Physics
  and P-frames containing:
  -[SplitPulses]: The split PulseSeriesMapMask, which contains the clustered Pulses as selected by the Splitter
  -[HCSplitPulses]: The split PulseSeriesMapMask after the application of HiveCleaning, if the 'Clean'-option has been chosen
  -[SplitPulsesTimeRange]: The time range of the PulseSeriesMapMask equal to [first-pulse.time, last-pulse.time]
  -[SplitPulses_Noised]: The SplitPulses plus all noise in the feasible time-range; might be helpful for native reconstructions which do not depend on hit/pulse-cleaning or which explicitly require noised pulses, because they assume a noise-probability
  -[I3TriggerHierarchy]: The SubTrigger Hierarchy containing only Triggers participating in this event
  -[LineFitName]: A LineFit reconstruction
  """
  
  from icecube import icetray, dataclasses, phys_services, IceHive
  from I3Tray import I3Units
  from icecube.icetray import pypick
  from icecube.phys_services.which_split import which_split
  
  if LineFitReco:
    try:
      from icecube import linefit
    except:
      raise RuntimeError("for the option LineFitReco you need to have LineFit installed")
  
  if Recombine:
    try:
      from icecube import CoincSuite
    except:
      raise RuntimeError("for the options 'Recombine' you need to have CoincSuite installed")
  
  if AddTimeWindow:
    try:
      from icecube import CoincSuite
    except:
      raise RuntimeError("for the options 'AddTimeWindow' you need to have CoincSuite installed")

  if Recover:
    try:
      from icecube import PulseRecover
    except:
      raise RuntimeError("for the option 'Clean'  you need to have PulseRecover installed.\nYou can fin it in SVN/sandbox/mzoll/PulseRecover/branches/LineFit.")
  

  #=====================
  # IceHive Event Splitting
  #======================
  #Run the HiveSplitter: create P-frames containing its splits

  from icecube import IceHive
  singleRings = IceHive.RingLimits()
  singleRings.AddLimitPair(IceHive.LimitPair(-255., 255.))
  singleRings.AddLimitPair(IceHive.LimitPair(-272.7, 272.7))
  singleRings.AddLimitPair(IceHive.LimitPair(-165.8, 165.8))
  doubleRings = IceHive.RingLimits()
  doubleRings.AddLimitPair(IceHive.LimitPair(-130., 70.))
  doubleRings.AddLimitPair(IceHive.LimitPair(-131.5, 131.5))
  doubleRings.AddLimitPair(IceHive.LimitPair(-40.8, 40.8))
  tripleRings = IceHive.RingLimits()
  tripleRings.AddLimitPair(IceHive.LimitPair(-130., 70.))
  tripleRings.AddLimitPair(IceHive.LimitPair(-144.1, 144.1))
  tripleRings.AddLimitPair(IceHive.LimitPair(-124.7, 124.7))
  tripleRings.AddLimitPair(IceHive.LimitPair(-82.8, 82.8))
  singleVicinity = singleRings
  doubleVicinity = doubleRings
  tripleVicinity = tripleRings
  #NOTE FUTURE a more stringend set of limits
  #singleVicinity = IceHive.RingLimits()
  #singleVicinity.AddLimitPair(IceHive.LimitPair(-100., 100.))
  #singleVicinity.AddLimitPair(IceHive.LimitPair(-100., 100.))
  #doubleVicinity = IceHive.RingLimits()
  #doubleVicinity.AddLimitPair(IceHive.LimitPair(-100., 100.))
  #doubleVicinity.AddLimitPair(IceHive.LimitPair(-100., 100.))
  #doubleVicinity.AddLimitPair(IceHive.LimitPair(-100., 100.))
  #tripleVicinity = IceHive.RingLimits()
  #tripleVicinity.AddLimitPair(IceHive.LimitPair(-100., 100.))
  #tripleVicinity.AddLimitPair(IceHive.LimitPair(-100., 100.))
  #tripleVicinity.AddLimitPair(IceHive.LimitPair(-100., 100.))
  #tripleVicinity.AddLimitPair(IceHive.LimitPair(-100., 100.))
  
  tray.AddModule("I3IceHive<I3RecoPulse>", SplitName,
    InputName = OriginalPulses,
    OutputName = SplitPulses,
    Multiplicity=3,
    TimeWindow=2000.*I3Units.ns,
    TimeCVMinus=300.*I3Units.ns,
    TimeCVPlus=300.*I3Units.ns,
    TimeCNMinus=200.*I3Units.ns,
    TimeCNPlus=200.*I3Units.ns,
    TimeStatic=200.*I3Units.ns,
    SingleDenseRingLimits=singleRings,
    DoubleDenseRingLimits=doubleRings,
    TripleDenseRingLimits=tripleRings,
    SingleDenseRingVicinity=singleVicinity,
    DoubleDenseRingVicinity=doubleVicinity,
    TripleDenseRingVicinity=tripleVicinity,
    SaveSplitCount=True,
    UpdateTriggerHierarchy = True,
    TrigHierName= TriggerHierarchyName,
    #TriggerConfigIDs= InIceTriggerIDs,
    #NoSplitDt = 10000,
    ReadoutWindowMinus = 4000.*I3Units.ns,
    ReadoutWindowPlus = 6000.*I3Units.ns)

  RecoPulses = SplitPulses #NOTE Make an alias on which pulses Reconstructions should commence

  if (Clean):
    cleanVicinity = IceHive.RingLimits()
    cleanVicinity.AddLimitPair(IceHive.LimitPair(-70., 70.))
    cleanVicinity.AddLimitPair(IceHive.LimitPair(-70., 70.))
    
    tray.AddModule("I3HiveCleaning<I3RecoPulse>","HiveClean",
      InputName = SplitPulses,
      OutputName = CleanedSplitPulsesName,
      TimeStaticMinus=600.*I3Units.ns,#NOTE default is 200.
      TimeStaticPlus=600.*I3Units.ns, #NOTE default is 200.
      SingleDenseRingVicinity=cleanVicinity,
      DoubleDenseRingVicinity=cleanVicinity,
      TripleDenseRingVicinity=cleanVicinity,
      If = which_split(SplitName) )
    
    RecoPulses = CleanedSplitPulsesName
    
  if Recombine:
    tray.AddModule(lambda f: f.Put(SplitName+"ReducedCount", icetray.I3Int(0)), "ReducedCountMaker",
                   Streams = [icetray.I3Frame.DAQ])

  #=========================
  # CoincSuite Recombinations
  #=========================
  if (Clean and Recombine):
    tray.AddModule(CoincSuite.discardEmptySplits,"removeEmptySplit",
                  SplitName = SplitName,
                  PulsesName = RecoPulses)

  if Recombine:
    tray.AddModule("AfterpulseDiscard", "AfterpulseDiscard",
      SplitName = SplitName,
      RecoMapName = SplitPulses,
      QTotFraction = .1,
      TimeOffset = 3000.*I3Units.ns,
      OverlapFraction = 0.5,
      Discard = True)

    tray.AddModule("HypoFrameCreator", "HypoFrameCreator",
      SplitName = SplitName,
      HypoName = "hypoframe",
      RecoMapName = RecoPulses,
      MaxTimeSeparation = 3000.*I3Units.ns)

    ### LineFit, a fast and simple reconstruction
    tray.AddSegment( linefit.simple,'LineFit',
      inputResponse = RecoPulses,
      fitName = LineFitName,
      If = (which_split(split_name=SplitName) | which_split(split_name='hypoframe')))

    tray.AddModule("TrackSystemTester", "TestHypoTrackSystem",
      SplitName = SplitName,
      HypoName = "hypoframe",
      RecoMapName = SplitPulses,
      HypoFitName = LineFitName,
      CriticalRatio = 0.7, #0.8
      CylinderRadius = 150.*I3Units.meter,
      ResTimeWindow = dataclasses.make_pair(-float("inf"),float("inf")), #FUTURE dataclasses.make_pair(-200*I3Units.ns,200*I3Units.ns),
      ParticleSpeed = float("nan")) #FUTURE dataclasses.I3Constants.c,
    
    tray.AddModule("TrackSystemTester", "TestMutualTrackSystem",
      SplitName = SplitName,
      HypoName = "hypoframe",
      RecoMapName = SplitPulses,
      RecoFitName = LineFitName,
      CriticalRatio = 0.7, #0.8
      CylinderRadius = 150.*I3Units.meter,
      ResTimeWindow = dataclasses.make_pair(-float("inf"),float("inf")), #FUTURE dataclasses.make_pair(-200*I3Units.ns,200*I3Units.ns),
      ParticleSpeed = float("nan"), #FUTURE dataclasses.I3Constants.c,
      MutualCompare=True)

    tray.AddModule("AlignmentTester", "TestHypoAlignment",
      SplitName = SplitName,
      HypoName = "hypoframe",
      HypoFitName = LineFitName,
      RecoFitName = LineFitName,
      CriticalAngle = 25.*I3Units.degree,
      CriticalDistance = 20.*I3Units.meter)

    tray.AddModule("AlignmentTester", "TestMutualAlignment",
      SplitName = SplitName,
      HypoName = "hypoframe",
      HypoFitName = LineFitName,
      RecoFitName = LineFitName,
      CriticalAngle = 25.*I3Units.degree,
      CriticalDistance = 20.*I3Units.meter,
      MutualCompare=True)

    tray.AddModule("SpeedTester","TestSpeed",
      SplitName = SplitName,
      HypoName = "hypoframe",
      HypoFitName = LineFitName,
      SpeedUpperCut = 0.35*I3Units.m/I3Units.ns,
      SpeedLowerCut = 0.15*I3Units.m/I3Units.ns)

    tray.AddModule("cogCausalConnectTester", "TestcogCausalConnect",
      SplitName = SplitName,
      HypoName = "hypoframe",
      RecoMapName = RecoPulses,
      HypoFitName = LineFitName,
      TravelTimeResidual = dataclasses.make_pair(-1000.*I3Units.ns, 1000.*I3Units.ns),
      WallTime = 3000.*I3Units.ns,
      MaxVerticalDist = 700.*I3Units.m,
      MaxHorizontalDist = 700.*I3Units.m)

    #where recombinations happen, for real
    RecombineKeys =[SplitPulses+"_Noised"]
    if (Clean): 
      RecombineKeys.append(CleanedSplitPulsesName)

    tray.AddModule("DecisionMaker", "FinalDecision",
      SplitName = SplitName,
      HypoName = "hypoframe",
      RecoMapName = SplitPulses,
      LikeNameList = ["TestHypoTrackSystem",
        "TestMutualTrackSystem",
        "TestHypoAlignment",
        "TestMutualAlignment"],
      VetoNameList = ["TestcogCausalConnect","TestSpeed"],
      TrueNameList = [],
      RecombineRecoMaps = RecombineKeys,
      Discard = True)

    #discard the hypoframes, as they are of no use anymore
    tray.AddModule( lambda f: f['I3EventHeader'].sub_event_stream!="hypoframe", "KillHypoFrame")

  #=================
  # REPEAT and uniformize
  #=================
  #Here procedures need to be repeated, which have not yet been performed on the recombined frames
#  if (Clean):
#    from icecube import IceHive
#    tray.AddModule("I3HiveCleaning<I3RecoPulse>","HiveClean_AGAIN",
#      InputName = SplitPulses,
#      OutputName = CleanedSplitPulsesName,
#      TimeStaticMinus=600.*I3Units.ns,#NOTE default is 200.
#      TimeStaticPlus=600.*I3Units.ns, #NOTE default is 200.
#      SingleDenseRingVicinity=cleanVicinity,
#      DoubleDenseRingVicinity=cleanVicinity,
#      TripleDenseRingVicinity=cleanVicinity,
#      If = which_split(SplitName) & pypick(lambda f: not f.Has(CleanedSplitPulsesName)))

  if (AddTimeWindow):
    tray.AddModule(CoincSuite.createTimeWindow,"HiveTimeWindow",
      InputPulses=SplitPulses,
      Output="TriggerSplitterLaunchWindow",
      If = which_split(SplitName))
    
    #NOTE Drop-in fix for TriggerHierarchies, which need to be present in the frame for certain filters
    def ClipTriggerHierarchy(frame):
      """ if frames do not have I3TriggerHierarchy put it there by clipping in time"""
      qth = frame[TriggerHierarchyName]
      tw = frame["TriggerSplitterLaunchWindow"]
      th = IceHive.clip_TriggerHierarchy(qth, tw, [1011, 1006,1007,21001])  #SMT8, SMT3, String, Volume-trigger
      frame.Put("I3TriggerHierarchy", th)
    tray.AddModule(ClipTriggerHierarchy, "ClipTriggers",
      If = which_split(SplitName) & pypick(lambda f: not f.Has("I3TriggerHierarchy")))
  
  if Recombine:
    tray.AddSegment( linefit.simple,'LineFit_AGAIN',
      inputResponse = RecoPulses,
      fitName = LineFitName,
      If = which_split(SplitName) & pypick(lambda f: not f.Has(LineFitName)))

  ### copy the key at 'filter_globals.SplitRTCleanedInIcePulses' to 'pulses'
  tray.AddModule("Copy", "copy_AGAIN",
                Keys =[RecoPulses, DesignatedPulsesName],
                If = which_split(SplitName)
                )
    
  ###
  # option to recover crutial pulses, which have been lost
  ###
  if (Recover):
    tray.AddModule("I3SimplePulseRecover", "SimplePulseRecover", #TODO options switch to IterativePulseRecover here!
                   PhysicsPulsesName = RecoPulses,
                   TrackName = LineFitName,
                   DiscoverablePulsesName = SplitPulses+"_Noised",
                   OutputPulsesName = RecoPulses+"Plus",
                   OutputTrackName = LineFitName+"Plus",
                   TimeResidualWindow = dataclasses.make_pair(-500.*I3Units.ns, +500.*I3Units.ns),
                   MaxDistance = 150.*I3Units.m,
                   ApproxTrack=True,
                   TrimTrack = True,
                   BackwardSearch= True,
                   ForwardSearch = False,
                   Deadtime = 0.0,
                   If = which_split(SplitName))

    def pick_recover():
      def f(frame):
        if (frame.Stop==icetray.I3Frame.Physics):
          return (frame[RecoPulses+'Plus'+"_discNCh"].value >= 2 and frame[RecoPulses+'Plus'+"_discQ"].value >= 1.5)
        return False
      return icetray.pypick(f)

    tray.AddModule("Delete", "delete_TRICE",
                   Keys = [DesignatedPulsesName, LineFitName, LineFitName+"Params"],
                   If = which_split(SplitName) & pick_recover()
                   )

    tray.AddModule("Copy", "copy_TRICE",
                   Keys =[RecoPulses+'Plus', DesignatedPulsesName],
                   If = which_split(SplitName) & pick_recover()
                   )

    tray.AddSegment( linefit.simple,'LineFit_TRICE',
                     inputResponse = DesignatedPulsesName,
                     fitName = LineFitName,
                     If = which_split(SplitName) & pick_recover()
                     )
    


