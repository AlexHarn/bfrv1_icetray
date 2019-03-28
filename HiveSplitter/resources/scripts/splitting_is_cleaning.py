#!/usr/bin/env python

###
# A Script that runs on a dataset of single-track events, splits them, recombines them and than plots some nice propperties
# USAGE: python [this_script.py] --help
###
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
  parser.add_option("-n", "--nevents", action="store", type="int", default=0, dest="NEVENTS", help="Number of Events to process")
  parser.add_option("--i3file", action="store_true", default=False, dest="I3FILE", help="write I3File")
  parser.add_option("--root", action="store_true", default=False, dest="ROOTFILE", help="write rootfile")
  (options,args) = parser.parse_args()
  params.Infile = options.INPUT 
  params.Outfile = options.OUTPUT
  params.GCDfile = options.GCD
  params.NEvents = options.NEVENTS
  params.I3File = options.I3FILE
  params.RootFile = options.ROOTFILE
#===============================================

@icetray.traysegment
def SplitterTray(tray, name, options):
  def Stepper(frame):
    print "RUN: "+str(frame["I3EventHeader"].run_id)+" EVENT: "+str(frame["I3EventHeader"].event_id)
  tray.AddModule(Stepper, "Stepper")

  #separate the Pulses into subseries for the particles
  from icecube import MCHitSeparator
  tray.AddModule("MCHitSeparator", "MCHitSerperator",
    MCTreeName = "I3MCTree",
    MCHitMapName = "MCHitSeriesMap",
    RecoMapName = "OfflinePulses",
    OutputName = "particle_",
    TrueMultiplicity = True,)

  #Here is the possibility to cut on [SimMult] or [TrueMult]
    
  tray.AddModule("I3HiveSplitter","OriginalHiveSplitter",
    InputName="OfflinePulses",
    OutputName="MaskedOfflinePulses",
    Multiplicity=4,
    TimeWindow=2000*I3Units.ns,
    TimeConeMinus=1000*I3Units.ns,
    TimeConePlus=1000*I3Units.ns,
    ICRingLimits=[300., 300., 272.7, 272.7, 165.8, 165.8], #I3Units.m
    DCRingLimits=[150., 150., 131.5, 131.5, 40.8, 40.8], #I3Units.m
    PinguRingLimits=[150., 150.0, 144.1, 144.1, 124.7, 124.7, 82.8, 82.8], #I3Units.m
    SaveSplitCount=True)
	
  tray.AddModule("MCHitIdentifier", "MCHitIdentifier",
    SubEventStream = "toposplit",
    RecoMapName = "MaskedOfflinePulses",
    ParticlePrefix = "particle_",
    AdditionalInfo = True)
	      
  from icecube import MCHitSeparator
  tray.AddModule(MCHitSeparator.MCRecombiner, "MCRecombiner",
      PulsesName = "MaskedOfflinePulses",
      SplitName = "toposplit",
      RemoveNoise = True)

  tray.AddModule("Delete","deleteMCHitIdentifierKeys2", #recomined frames and all others must be reevaluated
	  Keys=["AssociatedPrimary",
	  "FractionNCh",
	  "FractionNCh_MC",
	  "FractionNCh_Matched",
	  "FractionNCh_Noise",
	  "PrimaryContributionNCh"])

  tray.AddModule("MCHitIdentifier", "MCHitIdentifierAGAIN",
    SubEventStream = "toposplit",
    RecoMapName = "MaskedOfflinePulses",
    ParticlePrefix = "particle_",
    AdditionalInfo = True)

  if options.RootFile:
    from histocounter import histocounter #this file is local in HiveSplitter/resources/scripts
    tray.AddModule(histocounter, "histocounter",
                    OutfileName = options.Outfile+".root",
                    SplitName = "toposplit",
                    Source = options.Source)
  
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

  tray.AddSegment(SplitterTray, "SplittingIsCleaning", params)
  
  if params.I3File:
    tray.AddModule("I3Writer","writer",
                   streams = [icetray.I3Frame.DAQ, icetray.I3Frame.Physics, icetray.I3Frame.Geometry], #DANGER
                   filename = params.Outfile + ".i3",)

  
    
  if (params.NEvents==0):
    tray.Execute()
  else:
    tray.Execute(params.NEvents)
    
  
  del tray
