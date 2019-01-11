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


class RunParameters:
  def __init__(self):
    self.Infile = ''
    self.Outfile = ''
    self.GCDfile = ''
    self.NEvents = 0
    #___________________PARSER__________________________

import sys, os
from I3Tray import *
from icecube import icetray, dataio, dataclasses

from icecube.icetray import I3Units

print ("Output will be written to "+os.path.join(os.path.expandvars("$I3_BUILD")+"/CoincSuite/resources","example.i3.bz2"))


#uncomment; plug in your most favourite, but very noise module for debugging
#icetray.set_log_level_for_unit('cogCausalConnectTester', icetray.I3LogLevel.LOG_DEBUG)

tray = I3Tray()

tray.AddModule("I3Reader", "reader",
  FilenameList=[os.path.join(os.path.expandvars("$I3_BUILD")+"/CoincSuite/resources/data","example_scenario.i3.bz2")])           

#tray.AddModule("Dump", "dump")

from icecube import CoincSuite
tray.AddSegment(CoincSuite.Complete, "CoincSuiteComplete",
  suffix = '',
  SplitName = 'toposplit',
  SplitPulses = "MaskedOfflinePulses",
  FitName = 'LFit')

tray.AddModule("I3Writer","writer",
  streams = [#icetray.I3Frame.TrayInfo,
            icetray.I3Frame.Geometry,
            icetray.I3Frame.DAQ,
            icetray.I3Frame.Physics],
  Filename=os.path.join(os.path.expandvars("$I3_BUILD")+"/CoincSuite/resources","example.i3.bz2"))



tray.Execute()

