#!/usr/bin/env python

"""
This demonstrates how data can be treatment with event splitting and recombinations:
script runs an generic event splitter (TopologicalSplitter) and recombinations (CoincSuite).
can be used in order to generate a testcase.
"""

import os, sys
from optparse import OptionParser

from I3Tray import *
from icecube import icetray, dataio, dataclasses

from icecube.phys_services.which_split import which_split
from icecube.icetray import pypick

#uncomment; plug in your most favourite, but very noise module for debugging
#icetray.set_log_level_for_unit('cogCausalConnectTester', icetray.I3LogLevel.LOG_DEBUG)

class RunParameters:
  def __init__(self):
    self.Infile = ''
    self.Outfile = ''
    self.GCDfile = ''
    self.NEvents = 0

@icetray.traysegment
def Split_Recombine(tray, name,
                    OrgPulses = "", # NOTE Define the name of the unaltered Pulses in the Q-frame
                    SplitName = "", # NOTE Define the name of the subevent stream that you like to create
                    SplitPulses = ""): # NOTE Define how the split Pulses should be named

  # NOTE Use a generic event Splitter which works on Q-frames and writes P frames with the output being the SplitPulses
  # NOTE Do not forget to save the SplitCount to the Q-frame in the format <I3Int>(SplitName+"SplitCount")
#  from icecube import IceHive
#  tray.AddSegment(IceHive.Split_and_Recombine,"IceHive_Splitting")

  from icecube import CoincSuite
  # NOTE Create a complement object <I3Int>(SplitName+"ReducedName") in the Q frame keeping trace of the number of frames which have been recombined
  tray.AddModule(lambda f: f.Put(SplitName+"ReducedCount", icetray.I3Int(0)), "ReducedCountMaker",
    Streams = [icetray.I3Frame.DAQ])

  # NOTE Discard frames that you do not want to concider for recombinations early on.
  # NOTE every discarded frame is accounted for by increasing the 'ReducedCount'
  tray.AddModule("AfterpulseDiscard", "AfterpulseDiscard",
    SplitName = SplitName, #NOTE Run on the SplitFrames; this parameter should be set for all modules
    RecoMapName = SplitPulses, #NOTE use the SplitPulses; this parameter should be set for all modules
    QTotFraction = .1,
    TimeOffset = 3.E3*I3Units.ns,
    OverlapFraction = 0.75,
    Discard = True)

  # NOTE create the HypoFrames from all remaining SplitFrames
  tray.AddModule("HypoFrameCreator", "ForceHypoFrames",
    SplitName = SplitName,
    HypoName = "hypoframe", # NOTE give the stream of Hypoframes a unique name
    RecoMapName = SplitPulses,
    MaxTimeSeparation = 3000.*I3Units.ns) # NOTE specify that frames timely spaced too far apart should not be recombined

  from icecube import linefit, lilliput
  # NOTE Create variables and objects that you want to base your recombination decisions on; e.g. here a simple reconstruction
  tray.AddSegment( linefit.simple,'LineFit_Masked',
    inputResponse = SplitPulses,
    fitName = 'LineFit_Masked',
    If = which_split(SplitName) | which_split('hypoframe') ) # NOTE do this only on the SplitFrames and the HypoFrames; you can use the which_split-function to select the streams to execute on

  # NOTE Run all your Tester-modules that you like. No recombination will be performed at this step yet
  tray.AddModule("CylinderPulsesTester", "TestHypoCylinderPulses",
    SplitName = SplitName,
    HypoName = "hypoframe",
    RecoMapName = SplitPulses,
    HypoFitName = "LineFit_Masked",
    CriticalRatio = 0.7, #0.8
    CylinderRadius = 150*I3Units.meter)

  # NOTE Final Judgement on the recombination is passed here, by evaluation of the decisions of the Tester-modules,
  # NOTE recombined ComboFrames will be inlined at the appropriate place in the stream of SplitFrames
  tray.AddModule("DecisionMaker", "FinalDecision",
    SplitName = SplitName,
    HypoName = "hypoframe",
    RecoMapName = SplitPulses,
    LikeNameList = ["TestHypoCylinderPulses"], # NOTE if any of these Testers favours recombination frames will be recombined
    VetoNameList = [], # NOTE if any of these modules disfavours recombination frames will not be recombined
    TrueNameList = [],
    Discard = True) # NOTE if any of these module favours recombination frames will be recombined regardless

  # NOTE remove all HypoFrames because we do not need any longer
  tray.AddModule( lambda f: f["I3EventHeader"].sub_event_stream!="hypoframe", "KillHypoFrame")

  # NOTE rerun the variables and objects on the newly created ComboFrames
  tray.AddSegment(linefit.simple,'LineFit_AGAIN',
    inputResponse = SplitPulses,
    fitName = 'LineFit_Masked',
    If = which_split(SplitName) & pypick(lambda f: not f.Has("LineFit_Masked")) )

#___________________PARSER__________________________
def parseOptions(parser, params):
  i3_testdata = os.path.expandvars("$I3_TESTDATA")
  parser.add_option("-i","--input", 
                    action="store", 
                    type="string", 
                    default=os.path.join(i3_testdata,"IceHive/icehive_testcase.i3.bz2"), 
                    dest="INPUT", 
                    help="Input i3 file to process")
  parser.add_option("-o", "--output", 
                    action="store", 
                    type="string", 
                    default=os.path.expandvars("$I3_BUILD/CoincSuite/resources/recombined_testcase.i3.bz2"), 
                    dest="OUTPUT", 
                    help="Output i3 file")
  parser.add_option("-g", "--gcd", 
                    action="store",
                    type="string", 
                    default=os.path.join(i3_testdata,"IceHive/IdealGeometry.i3.bz2"),
                    dest="GCD", 
                    help="GCD file for input .i3-file")
  parser.add_option("-n", "--nevents", 
                    action="store", 
                    type="int", 
                    default=0, 
                    dest="NEVENTS", 
                    help="Number of Events to process")
  (options,args) = parser.parse_args()
  params.Infile = options.INPUT
  params.Outfile = options.OUTPUT
  params.GCDfile = options.GCD
  params.NEvents = options.NEVENTS

#___________________IF STANDALONE__________________________
if (__name__=='__main__'):
  params = RunParameters()

  usage = 'usage: %prog [options]'
  parser = OptionParser(usage)

  parseOptions(parser, params)

  icetray.logging.log_notice("output is written to "+params.Outfile)

  tray = I3Tray()

  tray.AddModule("I3Reader", "reader",
    FilenameList=[params.GCDfile, params.Infile])

  #tray.AddModule("Dump", "dump")

  tray.AddSegment(Split_Recombine, "Split_Recombine",
                  OrgPulses = "OfflinePulses",
                  SplitName = "org",
                  SplitPulses = "org")

  tray.AddModule("I3Writer","writer",
    streams = [icetray.I3Frame.Geometry,
              icetray.I3Frame.DAQ,
              icetray.I3Frame.Physics],
    filename = params.Outfile)

  

  if (params.NEvents==0):
    tray.Execute()
  else:
    tray.Execute(params.NEvents)

  
  del tray
