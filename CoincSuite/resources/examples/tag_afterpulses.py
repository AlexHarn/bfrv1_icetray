#!/usr/bin/env python

"""
Try to tag afterpulse events by running the AfterpulseDiscard Module in CoincSuite:
Read events with an already applied eventsplitter, run the module
"""

import os, sys
from optparse import OptionParser

from I3Tray import *
from icecube import icetray, dataio, dataclasses

#uncomment; plug in your most favourite, but very noise module for debugging
icetray.set_log_level(icetray.I3LogLevel.LOG_INFO)
icetray.set_log_level_for_unit('AfterpulseDiscard', icetray.I3LogLevel.LOG_TRACE)

class RunParameters:
  def __init__(self):
    self.Infile = ''
    self.Outfile = ''

    self.NEvents = 0
    #___________________PARSER__________________________
def parseOptions(parser, params):
  i3_testdata = os.path.expandvars("$I3_TESTDATA")
  parser.add_option("-i", "--input",
                    action="store",
                    type="string",
                    default=os.path.join(i3_testdata,"IceHive/icehive_testcase.i3.bz2"),
                    dest="INPUT",
                    help="Input i3 file to process")
  parser.add_option("-o", "--output",
                    action="store",
                    type="string",
                    default = "",
                    dest="OUTPUT", help="Output i3 file")
  parser.add_option("-n", "--nevents",
                    action="store", 
                    type="int", 
                    default=0,
                    dest="NEVENTS",
                    help="Number of Events to process")
  (options,args) = parser.parse_args()
  params.Infile = options.INPUT
  params.Outfile = options.OUTPUT
  params.NEvents = options.NEVENTS

@icetray.traysegment
def Split_Recombine(tray, name,
                    Params = RunParameters()):
  #tray.AddModule("Keep","HouseCleaning",
          #Keys = ["I3Geometry",
                  #"I3Calibration",
                  #"I3DetectorStatus",
                  #"I3DetectorConfiguration",
                  #"I3EventHeader",
                  #"OfflinePulses",
                  #"I3MCTree"])

  # NOTE Define the name of the unultered Pulses in the Q-frame
  OrgPulses = "OfflinePulses"
  # NOTE Define the name of the subevent stream that you like to create
  SplitName = "org"
  # NOTE Define how the split Pulses should be named
  SplitPulses = "org"

  from icecube import CoincSuite
  # NOTE Create a complement object <I3Int>(SplitName+"ReducedName") in the Q frame keeping trace of the number of frames which have been recombined
  tray.AddModule(lambda f: f.Put(SplitName+"ReducedCount", icetray.I3Int(0)), "ReducedCountMaker",
    Streams = [icetray.I3Frame.DAQ])

  # NOTE Discard frames that you do not want to concider for recombinations early on.
  # NOTE every discarded frame should be accounted for by increasing the 'ReducedCount'
  tray.AddModule("AfterpulseDiscard", "AfterpulseDiscard",
    SplitName = SplitName, #NOTE Run on the splitframes; this paramater should be set for all modules
    RecoMapName = SplitPulses, #NOTE use the SplitPulses; this paramater should be set for all modules
    QTotFraction = .1,
    TimeOffset = 3.E3*I3Units.ns,
    OverlapFraction = 0.75)

  # NOTE remove the frames the you do not want to use
#  tray.AddModule(lambda f: which_split(f,split_name=SplitName) and f.Has("Afterpulse"), "KillAfterpulseFrame")

  # NOTE create the HypoFrames from all remaining SplitFrames
  tray.AddModule("HypoFrameCreator", "ForceHypoFrames",
    SplitName = SplitName,
    HypoName = "hypoframe", # NOTE give the stream of Hypoframes a unique name
    RecoMapName = SplitPulses,
    MaxTimeSeparation = 3000.*I3Units.ns) # NOTE specify that frames timely spaced too far apart should not be recombined

#___________________IF STANDALONE__________________________
if (__name__=='__main__'):
  params = RunParameters()

  usage = 'usage: %prog [options]'
  parser = OptionParser(usage)

  parseOptions(parser, params)

  tray = I3Tray()

  tray.AddModule("I3Reader", "reader",
    FilenameList=[params.Infile])

  #tray.AddModule("Dump", "dump")

  tray.AddSegment(Split_Recombine, "Split_Recombine",
    Params = params)

  if params.Outfile!="":
    tray.AddModule("I3Writer","writer",
      #streams = [icetray.I3Frame.TrayInfo,
      #          icetray.I3Frame.Geometry,
      #          icetray.I3Frame.DAQ,
      #          icetray.I3Frame.Physics],
      filename = params.Outfile)

  

  if (params.NEvents==0):
    tray.Execute()
  else:
    tray.Execute(params.NEvents)

  
