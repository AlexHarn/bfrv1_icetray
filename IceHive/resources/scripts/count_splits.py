#!/usr/bin/env python

# Script to test the power of the Splitters in seperating coincident events from each other and leaving single track even intact
# The Multiplicity of the event will be extracted from the dataset either by having MCHitSeperator runnning up front or, as fall back options, the Multiplicity of CorsikaWeightMap (CORSIKA) or MCWeightDict (NUGEN)
# USAGE: see python [this_script.py] --help
#
#

import os, sys, getopt, glob
from os.path import expandvars
from icecube import icetray, dataio, dataclasses
from optparse import OptionParser

from I3Tray import *

### --------- CONFIGURATION and basic run information -----------------
class RunParameters:
  def __init__(self):
    self.Infile = ''
    self.Outfile = ''
    self.GCDfile = ''
    self.NEvents = 0
    self.Multiplicity = 0
    self.I3File = False
    self.RootFile = False

#___________________PARSER__________________________
def parseOptions(parser, params):
  parser.add_option("-i", "--input", action="store", type="string", default="", dest="INPUT", help="Input i3 file to process")
  parser.add_option("-o", "--output", action="store", type="string", default="", dest="OUTPUT", help="Output i3 file")
  parser.add_option("-g", "--gcd", action="store", type="string", default="", dest="GCD", help="GCD file for input i3 file")
  parser.add_option("-n", "--nevents", action="store", type="int", default=0, dest="NEVENTS", help="Number of events to process")
  parser.add_option("--i3file", action="store_true", default=False, dest="I3FILE", help="write I3File")
  (options,args) = parser.parse_args()
  params.Infile = options.INPUT
  params.Outfile = options.OUTPUT
  params.GCDfile = options.GCD
  params.NEvents = options.NEVENTS
  params.I3File = options.I3FILE
#===============================================

class Counter(icetray.I3PacketModule):
  """A Module that compares the number of written out splits to the multiplicity as stated by the MCTruth"""
  def __init__(self, ctx):
    icetray.I3PacketModule.__init__(self, ctx, icetray.I3Frame.DAQ)
    self.AddOutBox("OutBox")
    self.wrongminus=0
    self.right=0
    self.wrongplus=0
    self.wrongminus_weight=0
    self.right_weight=0
    self.wrongplus_weight=0
    self.AddParameter("OnlyWrongs","Push only those frames which where wrong", False )
  def Configure(self):
    icetray.I3PacketModule.Configure(self)
    self.onlywrongs = self.GetParameter("OnlyWrongs")
  def Finish(self):
    icetray.I3PacketModule.Finish(self)
    print "right (weight): %d, %g" % (self.right, self.right_weight)
    print "wrong_plus (weight): %d, %g" % (self.wrongplus, self.wrongplus_weight)
    print "wrong_minus (weight): %d, %g" % (self.wrongminus, self.wrongminus_weight)
  def FramePacket(self,frames):
    multi=0
    select= False
    for frame in frames:
      nframes=0
      if frame.Stop == icetray.I3Frame.DAQ: #found DAQ
	n_files = 100
	weight =1 # frame["I3MCWeightDict"]["OneWeight"] / (n_files)

	if frame.Has("TrueMultiplicity"):
	  multi = frame['TrueMultiplicity'].value
	#Fallback
	elif frame.Has("CorsikaWeightMap") and frame.Has("I3MCWeightDict"):
	  multi = frame["CorsikaWeightMap"]['Multiplicity'] + 1
	elif frame.Has("CorsikaWeightMap"):
	  multi = frame["CorsikaWeightMap"]['Multiplicity']
	elif frame.Has("I3MCWeightDict"):
	  multi = 1
	else:
	  multi = 0
	#the number of events split-off
	if frame.Has("toposplitSplitCount") and frame.Has("toposplitReducedCount"):
	  splitmulti =frame['toposplitSplitCount'].value-frame['toposplitReducedCount'].value
	elif frame.Has("toposplitSplitCount"):
	  splitmulti =frame['toposplitSplitCount'].value
	if splitmulti == multi:
	  self.right_weight += weight
	  self.right += 1
	elif splitmulti < multi:
	  self.wrongminus_weight += weight
	  self.wrongminus += 1
	  select = True
	elif splitmulti > multi:
	  self.wrongplus_weight += weight
	  self.wrongplus += 1
	  select = True
      if not self.onlywrongs:
	self.PushFrame(frame)
      else:
	if select:
	  self.PushFrame(frame)
    return


@icetray.traysegment
def Count(tray, name, options):
  def Stepper(frame):
    print "RUN: "+str(frame["I3EventHeader"].run_id)+" EVENT: "+str(frame["I3EventHeader"].event_id)
  tray.AddModule(Stepper, "Stepper")

  tray.AddModule("I3HiveSplitter","HiveSplit",
    InputName="OfflinePulses",
    OutputName="MaskedOfflinePulses",
    Multiplicity=4,
    TimeWindow=2000*I3Units.ns,
    TimeConeMinus=1000*I3Units.ns,
    TimeConePlus=1000*I3Units.ns,
    SingleDenseRingLimits=[255., 255., 272.7, 272.7, 165.8, 165.8], #I3Units.m
    DoubleDenseRingLimits=[70., 70., 131.5, 131.5, 40.8, 40.8], #I3Units.m
    TrippleDenseRingLimits=[70., 70., 144.1, 144.1, 124.7, 124.7, 82.8, 82.8], #I3Units.m
    Mode =1,
    SaveSplitCount=True,
    TrigHierName= "I3TriggerHierarchy",
    TriggerConfigIDs= [1006, 1007, 1011, 21001], #[(SMT8),(string),(SMT3),(volume)]
    NoSplitDt = 10000, #seems
    ReadoutWindowMinus = 4000,
    ReadoutWindowPlus = 6000)

  tray.AddModule(Counter, "Counter",
    OnlyWrongs = options.WriteOnlyWrongs)

#___________________IF STANDALONE__________________________
if (__name__=='__main__'):
  from optparse import OptionParser

  params = RunParameters()

  usage = 'usage: %prog [options]'
  parser = OptionParser(usage)

  parseOptions(parser, params)

  Infile_List = glob.glob(params.Infile)

  from icecube import icetray, dataio

  tray = I3Tray()

  tray.AddModule("I3Reader", "reader",
    filenamelist=[params.GCDfile]+Infile_List)

  tray.AddSegment(Count, "Count", params)

  if params.I3File:
    tray.AddModule("I3Writer","writer",
                   streams = [icetray.I3Frame.DAQ, icetray.I3Frame.Physics, icetray.I3Frame.Geometry], #DANGER
                   filename = params.Outfile + ".i3",)

  

  if (params.NEvents==0):
    tray.Execute()
  else:
    tray.Execute(params.NEvents)

  
  del tray
