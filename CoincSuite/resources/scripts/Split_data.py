#!/usr/bin/env python

"""
Run a splitter of the users choice on data
This should work more or less out of the box on everything newer than IC79 and on higher levels from L2a
"""

import os, sys, glob
from os.path import expandvars
from optparse import OptionParser
from icecube import icetray, dataclasses

class RunParameters:
  def __init__(self):
    self.Infile = '' 
    self.Outfile = ''
    self.GCDfile = ''
    self.NEvents = 0
    self.Splitter =0

@icetray.traysegment
def Process(tray, name, 
            Params = RunParameters()):

  def p_away(frame):
    return False
  tray.AddModule(p_away, "p_away", Streams = [icetray.I3Frame.Physics])

  SplitName = "split"
  InputPulses = "OfflinePulses"
  OutputPulses = "MaskedOfflinePulses"

  #=========================
  if (Params.Splitter==1):
  #=========================
    from icecube import phys_services
    tray.AddModule("I3NullSplitter", SplitName,
                   MaskedPulsesName = OutputPulses)

  #=========================
  elif (Params.Splitter==1):
  #=========================
    from icecube import trigger_splitter
    tray.AddModule("I3TriggerSplitter", SplitName,
      InputResponses = [InputPulses],
      OutputResponses = [OutputPulses],
      TrigHierName= "I3TriggerHierarchy",
      TriggerConfigIDs= [1006, 1007, 1011, 21001], #[(SMT8),(string),(SMT3),(volume)]
      NoSplitDt = 10000,
      ReadoutWindowMinus = 4000,
      ReadoutWindowPlus = 6000)

  #=========================
  elif (Params.Splitter==2):
  #=========================
    from icecube import SeededRTCleaning
    tray.AddModule( 'I3SeededRTHitMaskingModule','SeededRTCleaning',
                      InputResponse = InputPulses,
                      OutputResponse = "SRTOfflinePulses",
                      RTRadius = 150,
                      RTTime = 1000,
                      MaxIterations = 3,
                      Seeds = 'HLCcore',
                      HLCCoreThreshold = 2,
                      Stream=icetray.I3Frame.DAQ)
    
    from icecube import TopologicalSplitter                                                                                   
    tray.AddModule("I3TopologicalSplitter", SplitName,
                    InputName   = "SRTOfflinePulses",
                    OutputName  = OutputPulses,
                    Multiplicity = 4, #Default=4 //Solarwimp-IC79 setting 4 (minimal nch in order to do SPEFit2)
                    TimeWindow  = 4000, #Default=4000 ns
                    XYDist = 300, #Default=500 m
                    ZDomDist = 15,
                    TimeCone = 1000, #Default=1000 ns
                    SaveSplitCount = True)

  #=========================
  elif (Params.Splitter==3):
  #=========================
    from icecube import TopologicalSplitterMaxDist
    tray.AddModule("I3TopologicalSplitterMaxDist", SplitName,
      InputName = InputPulses,
      OutputName = OutputPulses,
      Multiplicity = 4,
      TimeWindow = 4000,
      XYDist = 300,
      ZDist = 300, 
      ZDomDist = 15,
      XYDCDist = 150,
      ZDCDist = 150,
      ZDCDomDist = 10,
      TimeConeMinus = 1000,
      TimeConePlus = 1000,
      SaveSplitCount = True)

  #=========================
  elif (Params.Splitter==4):
  #=========================
    from icecube import IceHive
    tray.AddModule("I3IceHive", SplitName,
        InputName = InputPulses,
        OutputName = OutputPulses,
        Multiplicity=4,
        TimeWindow=2000.*I3Units.ns,
        TimeCVMinus=300.*I3Units.ns,
        TimeCVPlus=300.*I3Units.ns,
        TimeCNMinus=200.*I3Units.ns,
        TimeCNPlus=200.*I3Units.ns,
        TimeStatic=300.*I3Units.ns,
        SingleDenseRingLimits=[255., 255., 272.7, 272.7, 165.8, 165.8], #I3Units.m
        DoubleDenseRingLimits=[70., 70., 131.5, 131.5, 40.8, 40.8], #I3Units.m
        TrippleDenseRingLimits=[70., 70., 144.1, 144.1, 124.7, 124.7, 82.8, 82.8], #I3Units.m
        SingleDenseRingVicinity=[100.,100.,100.,100.], #I3Units.m
        DoubleDenseRingVicinity=[100.,100.,100.,100.,100.,100.], #I3Units.m
        TrippleDenseRingVicinity=[100.,100.,100.,100.,100.,100.,100.,100.], #I3Units.m
        #Mode = 4,
        SaveSplitCount=True,)
        #NoSplitMode = False,
        #TrigHierName= "QTriggerHierarchy",
        #TriggerConfigIDs= [1006, 1007, 1011, 21001], #[(SMT8),(string),(SMT3),(volume)]
        #NoSplitDt = 10000,
        #ReadoutWindowMinus = 4000,
        #ReadoutWindowPlus = 6000)
        
  #=========================
  elif (Params.Splitter==5):
  #=========================
    from icecube import IceHive
    tray.AddModule("I3IceHive<I3RecoPulse>",SplitName,
      InputName = InputPulses,
      OutputName = OutputPulses,
      Multiplicity=4,
      TimeWindow=2000*I3Units.ns,
      TimeCVMinus=300*I3Units.ns,
      TimeCVPlus=300*I3Units.ns,
      TimeCNMinus=200*I3Units.ns,
      TimeCNPlus=200*I3Units.ns,
      TimeStatic=200*I3Units.ns,
      SingleDenseRingLimits=[255., 255., 272.7, 272.7, 165.8, 165.8], #I3Units.m
      DoubleDenseRingLimits=[70., 70., 131.5, 131.5, 40.8, 40.8], #I3Units.m
      TrippleDenseRingLimits=[70., 70., 144.1, 144.1, 124.7, 124.7, 82.8, 82.8], #I3Units.m
      SingleDenseRingVicinity=[100.,100.,100.,100.], #I3Units.m
      DoubleDenseRingVicinity=[100.,100.,100.,100.,100.,100.], #I3Units.m
      TrippleDenseRingVicinity=[100.,100.,100.,100.,100.,100.,100.,100.], #I3Units.m
      SaveSplitCount=True,)
      #NoSplitMode = False,
      #TrigHierName= "QTriggerHierarchy",
      #TriggerConfigIDs= [1006, 1007, 1011, 21001], #[(SMT8),(string),(SMT3),(volume)]
      #NoSplitDt = 10000,
      #ReadoutWindowMinus = 4000,
      #ReadoutWindowPlus = 6000)

    #from icecube import IceHive
    #tray.AddModule("I3HiveCleaning<I3RecoPulse>","HiveClean",
      #InputName = "SplitPulses",
      #OutputName = "HC"+SplitPulses,
      #TimeStaticMinus=600*I3Units.ns,
      #TimeStaticPlus=600*I3Units.ns,
      #SingleDenseRingVicinity=[70.,70.,70.,70.,], #I3Units.m
      #DoubleDenseRingVicinity=[70.,70.,70.,70.], #I3Units.m
      #TrippleDenseRingVicinity=[70.,70.,70.,70.,], #I3Units.m
      #If = lambda f: True)#which_split(f, SplitName) )
   
  #=========================
  else:
  #=========================
    raise ValueError("have not configured such splitter")
  
  #A LineFit and a single SPEfit for free
  from icecube import linefit
  tray.AddSegment( linefit.simple,'LineFit',
    inputResponse = OutputPulses,
    fitName = 'LineFit',
    If = lambda f: True)

  from icecube import lilliput
  import icecube.lilliput.segments
  #single iteration fit
  tray.AddSegment( lilliput.segments.I3SinglePandelFitter,'SPEFit',
    pulses = OutputPulses,
    seeds = ['LineFit'],
    If = lambda f: True)

#___________________PARSER__________________________
def parseOptions(parser, params):
  parser.add_option("-g", "--gcd", action="store", type="string", default="", dest="GCD", help="GCD file for input i3 file")
  parser.add_option("-i", "--input", action="store", type="string", default="", dest="INPUT", help="Input i3 file to process")
  parser.add_option("-o", "--output", action="store", type="string", default="", dest="OUTPUT", help="Output i3 file")
  parser.add_option("-n", "--nevents", action="store", type="int", default=0, dest="NEVENTS", help="Number of Events to process")
  parser.add_option("--splitter", action="store", type="int", dest="SPLITER", help="whichSplitter to use: (0)NullSplitter (1)TriggerSplitter (2)TopologicalSplitter, (3)MaxDist, (4)HiveSplitter, (5)IceHive")
  (options,args) = parser.parse_args()
  params.GCDfile = options.GCD
  params.Infile = options.INPUT 
  params.Outfile = options.OUTPUT
  params.NEvents = options.NEVENTS
  params.Splitter = options.SPLITTER

#___________________IF STANDALONE__________________________
if (__name__=='__main__'):
  from optparse import OptionParser
  
  params = RunParameters()
  usage = 'usage: %prog [options]'
  parser = OptionParser(usage)
  parseOptions(parser, params)
  
  Infile_List = glob.glob(params.Infile)
  Infile_List.sort()

  from icecube import icetray, dataio
  from I3Tray import *
  tray = I3Tray()
  
  tray.AddModule("I3Reader", "reader",
    filenamelist=[params.GCDfile]+Infile_List)
    
  tray.AddSegment(Process, "Split",
                  Params = params)

  #tray.AddModule("Dump", "dump")
  
  tray.AddModule("I3Writer","writer",
    streams = [icetray.I3Frame.DAQ, icetray.I3Frame.Physics],
    filename = params.Outfile)

  
    
  if (params.NEvents==0):
    tray.Execute()
  else:
    tray.Execute(params.NEvents)
    
  
  del tray
  
