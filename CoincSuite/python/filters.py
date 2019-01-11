#!/usr/bin/env python

"""
A helper library that provides various filter options to shape event-selections
"""
from icecube import icetray, dataclasses
from icecube.icetray import I3Units

#===============================================================================
class KeySelector(icetray.I3PacketModule):
  """
  Select only framePackets that have these keys present in the Q-frame
  
  :param Keys: A list of keys that have to be present in the frame
  """
  def __init__(self, ctx):
    icetray.I3PacketModule.__init__(self, ctx, icetray.I3Frame.DAQ)
    self.AddParameter("Keys", "A list of keys that have to be present in the frame", [])
    self.AddOutBox("OutBox")
  def Configure(self):
    icetray.I3PacketModule.Configure(self)
    self.requiredKeys = self.GetParameter("Keys")
  def FramePacket(self,frames):
    for key in self.requiredKeys:
      if not frames[0].Has(key):
        return
    for frame in frames:
      self.PushFrame(frame)
    return

#===============================================================================
class OnlyNPrimaries(icetray.I3PacketModule):
  """
  A function to select only events with a given number of primaries by checking in the I3MCTree
  
  :param NPrimaries: Select only frames with that many primaries present
  """
  def __init__(self, ctx):
    icetray.I3PacketModule.__init__(self, ctx, icetray.I3Frame.DAQ)
    self.AddParameter("NPrimaries", "Select only MC frames with that many primaries present", 1)
    self.AddOutBox("OutBox")
  def Configure(self):
    icetray.I3PacketModule.Configure(self)
    self.nmany = self.GetParameter("NPrimaries")
  def FramePacket(self,frames): #ACTION goes down here
    if frames[0].Stop == icetray.I3Frame.DAQ:
      if frames[0].Has("I3MCTree"):
        tree = frames[0]["I3MCTree"]
        prim = tree.primaries
        if len(prim)==self.nmany:
          if prim[0].is_neutrino:
            for frame in frames:
              self.PushFrame(frame)

#===============================================================================
class FilterSelector(icetray.I3PacketModule):
  """
  Selects filters from the FilterMask and issues a given number of events
  
  param FilterList: Select only events from that Filters
  param NEvents: That many Events are to be delivered
  """
  def __init__(self, ctx):
    icetray.I3PacketModule.__init__(self, ctx, icetray.I3Frame.DAQ)
    self.AddParameter("FilterList", "Select only events from that Filters", ["MuonFilter_11"])
    self.AddParameter("NEvents", "That many events are to be delivered", float('inf'))
    self.AddOutBox("OutBox")
  def Configure(self):
    icetray.I3PacketModule.Configure(self)
    self.filternamelist=self.GetParameter("FilterList")
    self.deliverReq=self.GetParameter("NEvents")
    self.delivered=0
  def FramePacket(self,frames):
    pushthese = False
    if frames[0].Stop == icetray.I3Frame.DAQ:
      if frames[0].Has("FilterMask"):
        filter_selected = False
        for filter_key in self.filternamelist:
          if filter_key in frames[0]["FilterMask"].keys():
            if frames[0]["FilterMask"][filter_key].condition_passed and frames[0]["FilterMask"][filter_key].prescale_passed:
              filter_selected = True
        if filter_selected:
          self.delivered+=1
          pushthese=True
    if pushthese:
      for frame in frames:
        self.PushFrame(frame)
    if self.delivered>=self.deliverReq:
      self.RequestSuspension()
    return

#===============================================================================
class UpDownCounter(icetray.I3PacketModule):
  """
  Counts if tracks are up or down-going; runs on P-frames of a given SplitName
  
  :param SplitName: name of the sub_event_stream to run on
  :param FitName: Name of the Fit in the P-Frame for the evaluation
  :param NCh_Cut: A nch treshhold below which no decision is taken
  """
  def __init__(self, ctx):
    icetray.I3PacketModule.__init__(self, ctx, icetray.I3Frame.DAQ)
    self.AddOutBox("OutBox")
    self.AddParameter("SplitName", "Name of the split", "toposplit")
    self.AddParameter("FitName", "Name of the Fit to use", "LineFit_Masked")
    self.AddParameter("NCh_Cut", "Only take events with at least that nch", 10)
  def Configure(self):
    icetray.I3PacketModule.Configure(self)
    self.splitname = self.GetParameter("SplitName")
    self.fitname = self.GetParameter("FitName")
    self.nch_cut = self.GetParameter("NCh_Cut")
    self.Up_brightest=0
    self.Down_brightest=0
    self.Up_all=0
    self.Down_all=0
    self.allevents =0
    self.allsubevents =0
  def Finish(self):
    icetray.I3PacketModule.Finish(self)
    icetray.logging.log_notice("UP/DOWN COUNTER reporting: \n  up_brightest: %d\n  down_brightest: %d \n  up_all: %d \n  down_all: %d \n  all seen events: %d \n  all seen subevents: %d \n" % (self.Up_brightest,self.Down_brightest,self.Up_all,self.Down_all,self.allevents,self.allsubevents))
  def FramePacket(self,frames):
    self.allevents+=1
    nch_these =[]
    all_these=[]
    nch_max = 0
    for frame in frames:
      if frame.Stop == icetray.I3Frame.Physics: #found DAQ
        if frame.Has("I3EventHeader"):
          if frame["I3EventHeader"].sub_event_stream == self.splitname:
            self.allsubevents+=1;
            nch = len(frame["MaskedOfflinePulses"].apply(frame))
            if nch_max < nch:
              nch_max = nch
              frame_max = frame
            if nch>= self.nch_cut:
              if frame[self.fitname].fit_status == dataclasses.I3Particle.OK:
                zenith = frame[self.fitname].dir.zenith
                if (zenith>90*I3Units.degree):
                  self.Up_all += 1
                else:
                  self.Down_all +=1
    if nch_max>=self.nch_cut:
      frame = frame_max
      if frame.Has("LineFit_Masked"):
        if frame["LineFit_Masked"].fit_status == dataclasses.I3Particle.OK:
          zenith = frame["LineFit_Masked"].dir.zenith
          if (zenith>90*I3Units.degree):
            self.Up_brightest += 1
          else:
            self.Down_brightest +=1
    for frame in frames:
      self.PushFrame(frame)
    return
