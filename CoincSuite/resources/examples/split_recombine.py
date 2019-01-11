#!/usr/bin/env python

"""
This demonstrates how data can be treatment with event splitting and recombinations:
an i3file is read which contains already split frames, then the CoincSuite recombinations
are applied as they are contained in the CoincSuite.complete tray-segment
In case you want an outfile, specify the '-o' option
"""

import os, sys
from optparse import OptionParser

from I3Tray import *
from icecube import icetray, dataio, dataclasses

#uncomment; plug in your most favourite, but very noise module for debugging
#icetray.set_log_level_for_unit('cogCausalConnectTester', icetray.I3LogLevel.LOG_DEBUG)

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
                    default="",
                    dest="OUTPUT",
                    help="Output i3 file")
  parser.add_option("-n", "--nevents",
                    action="store",
                    type="int",
                    default=0,
                    dest="NEVENTS",
                    help="Number of Events to process")
  (options,args) = parser.parse_args()
  if options.OUTPUT=="":
    print("specify an output file")

  params.Infile = options.INPUT
  params.Outfile = options.OUTPUT
  params.NEvents = options.NEVENTS

@icetray.traysegment
def Split_Recombine(tray, name,
                    params):

  from icecube import CoincSuite
  tray.AddSegment(CoincSuite.Complete, "CoincSuiteComplete",
    suffix = '',
    SplitName = 'org',
    SplitPulses = "org")


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

  Split_Recombine( tray, "Split_Recombine", params)

  if params.Outfile!="":
    tray.AddModule("I3Writer","writer",
    #streams = [icetray.I3Frame.TrayInfo,
              #icetray.I3Frame.Geometry,
              #icetray.I3Frame.DAQ,
              #icetray.I3Frame.Physics],
      filename = params.Outfile)
    
  

  if (params.NEvents==0):
    tray.Execute()
  else:
    tray.Execute(params.NEvents)

  
  del tray
