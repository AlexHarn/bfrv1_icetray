"""
These are modules which can be used in processing and quantification of MC-datasets

helpful and sometimes needed are the projects MCHitSeparator and MCPulseSeparator in this context
"""

from icecube import icetray, dataclasses

class Counter(icetray.I3PacketModule):
  """
  A counter function of wrong splits:
  can also select only these events that were wrong to be pushed

  works intelligent with weighting according to specified source and useage of MCHitSeparator keys if available

  :param PushWrongPlus: If specified push only those FramePackets that contain more splits than the expectation
  :param PushWrongMinus: If specified push only those FramePackets that contain less splits than the expectation
  :param SplitName: name of the subevent-stream, identical to SplitterModule
  :param Source: Source of the input data: choose from ["DATA", "CORSIKA", "NUGEN", "COINCUGEN", "E2NUGEN", "WIMPSUN", "WIMPEARTH"]
  """
  def __init__(self, ctx):
    icetray.I3PacketModule.__init__(self, ctx, icetray.I3Frame.DAQ)
    self.AddOutBox("OutBox")
    #bookkeeping
    self.wrongminus=0
    self.right=0
    self.wrongplus=0
    self.wrongminus_weighted=0.
    self.right_weighted =0.
    self.wrongplus_weighted =0.
    #parameters
    self.AddParameter("PushWrongPlus","Push only those frames which were counted WrongPlus", False )
    self.AddParameter("PushWrongMinus","Push only those frames which were counted WrongMinus", False )
    self.AddParameter("SplitName", "Name of the split", "toposplit")
    self.AddParameter("WeightKey", "Where to find the key storing th event wheight", "")
  def Configure(self):
    icetray.I3PacketModule.Configure(self)
    self.pushWrongPlus = self.GetParameter("PushWrongPlus")
    self.pushWrongMinus = self.GetParameter("PushWrongMinus")
    self.splitname = self.GetParameter("SplitName")
    self.weightKey = self.GetParameter("WeightKey")
    if self.weightKey=="":
      icetray.logging.log_fatal("Specify 'WeightKey'")
  def Finish(self):
    icetray.I3PacketModule.Finish(self)
    icetray.logging.log_notice("right (weighted): %d (%.3e)" % (self.right, self.right_weighted))
    icetray.logging.log_notice("wrongplus (weighted): %d (%.3e)" % (self.wrongplus, self.wrongplus_weighted))
    icetray.logging.log_notice("wrongminus (weighted): %d (%.3e)" % (self.wrongminus, self.wrongminus_weighted))
  def FramePacket(self,frames):
    multi=0
    select= False
    nfiles = 1
    for frame in frames:
      nframes=0
      if frame.Stop == icetray.I3Frame.DAQ: #found DAQ
        #Get Weight
        weight = frame[self.weightKey].value

        #Get Multiplicity
        if frame.Has("TrueMultiplicity"):
          multi = frame['TrueMultiplicity'].value
        elif frame.Has("CorsikaWeightMap") and frame.Has("I3MCWeightDict"): #Fallbacks
          multi = frame["CorsikaWeightMap"]['Multiplicity'] + 1
        elif frame.Has("CorsikaWeightMap"):
          multi = frame["CorsikaWeightMap"]['Multiplicity']
        elif frame.Has("I3MCWeightDict"):
          multi = 1
        else:
          multi = 0
        #the number of events split-off
        if frame.Has(self.splitname+"SplitCount") and frame.Has(self.splitname+"ReducedCount"):
          splitmulti =frame[self.splitname+'SplitCount'].value-frame[self.splitname+'ReducedCount'].value
        elif frame.Has(self.splitname+"SplitCount"):
          splitmulti =frame[self.splitname+'SplitCount'].value
        else:
          icetray.logging.log_fatal("CoincSuite.Counter::Could not find splitcount '%s' in the Qframe" % (self.splitname+"SplitCount"))
        #do count
        if splitmulti == multi:
          self.right += 1
          self.right_weighted+=weight
        elif splitmulti < multi:
          self.wrongminus += 1
          self.wrongminus_weighted+=weight
          if self.pushWrongMinus:
            select = True
        elif splitmulti > multi:
          self.wrongplus += 1
          self.wrongplus_weighted+=weight
          if self.pushWrongPlus:
            select = True
    #treat pushing of frames
    if not (self.pushWrongPlus or self.pushWrongMinus):
      for frame in frames:
        self.PushFrame(frame)
    else:
      if select:
        for frame in frames:
          self.PushFrame(frame)
    return
