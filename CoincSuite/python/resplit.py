#!/usr/bin/env python

"""
Helpers to help you to resplit and reassamble your already split frames:
Concider the scenario where you have already applied an event-splitter 
and the data has been processed to further, involving cuts, cleanings, whatever.
Through this the purity of data (pulses/ reconstructability/ event-classes) has become much more homogenious.
Certain processings from previous stages, like the splitting and recombination, might be desired to be repeated
with harder parameter settings adapted to the more homogenious dataset.
This collection of I3Modules allows exactly this.
"""
from icecube import icetray, dataclasses

class moveObjects(icetray.I3PacketModule):
  """
  Move the specified objects between the frames of the Originial and the Target stream;
  if the flag is specified only flaged frames from the original stream are considered
  
  :param OriginalStream: default="toposplit" ,Name of the Original SubEventStream
  :param TargetStream: default="resplit" ,Name of the Target SubEventStream
  :param MoveObjects: default=["LineFit_Masked"], "These Objects shall be moved to the new stream
  :param FlagName: default="ThisOne" ,Name of the Flag that should be placed
  """
  def __init__(self, ctx):
    icetray.I3PacketModule.__init__(self, ctx, icetray.I3Frame.DAQ)
    self.AddOutBox("OutBox")
    self.AddParameter("OriginalStream", "Name of the Original SubEventStream", "")
    self.AddParameter("TargetStream", "Name of the Target SubEventStream", "")
    self.AddParameter("MoveObjects", "These Objects shall be moved to the new stream", ["LineFit_Masked"])
    self.AddParameter("FlagName", "Name of the Flag that should be placed", "ThisOne")
  def Configure(self):
    icetray.I3PacketModule.Configure(self)
    self.originalStream = self.GetParameter("OriginalStream")
    self.targetStream = self.GetParameter("TargetStream")
    self.moveObjects = self.GetParameter("MoveObjects")
    self.flagName = self.GetParameter("FlagName")

    if self.originalStream=="" or self.targetStream=="":
      exit("Configure OriginalStream and TargetStream")
  def Finish(self):
    icetray.I3PacketModule.Finish(self)
  def FramePacket(self,frames):
    originalFrame = icetray.I3Frame() #keep this pointer alive
    targetFrame = icetray.I3Frame()
    for oframe in frames:
      if oframe.Has("I3EventHeader") and oframe["I3EventHeader"].sub_event_stream==self.originalStream:
        originalFrame=oframe
        for tframe in frames:
          if tframe.Has("I3EventHeader") and tframe["I3EventHeader"].sub_event_stream==self.targetStream and tframe["I3EventHeader"].sub_event_id==oframe["I3EventHeader"].sub_event_id:
            targetFrame=tframe
            for key in self.moveObjects:
              targetFrame[key]=originalFrame[key]
            targetFrame.Put(self.flagName, icetray.I3Bool(True))
    for frame in frames:
      self.PushFrame(frame)
