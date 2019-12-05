#!/usr/bin/env python

import os
import unittest

from I3Tray import *
from icecube import icetray, dataio, dataclasses
from icecube import gulliver

theFileName = "TheFile.i3.gz"

theI3File = dataio.I3File(theFileName, 'w')

def PutStuffInTheFrame(i3File):

  params = gulliver.I3LogLikelihoodFitParams()
  params.logl = 33.
  params.rlogl = 42.
  params.ndof = 55
  params.nmini = 36

  frame = icetray.I3Frame(icetray.I3Frame.DAQ)
  frame["params"] = params
  i3File.push(frame)

PutStuffInTheFrame(theI3File)
theI3File.close()

#Open it back up and get stuff out
newI3File = dataio.I3File(theFileName, 'r')
testFrame = newI3File.pop_daq()

class TestSerialize(unittest.TestCase):

  def test_check_values_from_frame(self):
    self.assertEqual(testFrame.Has("params"), True)
    params = testFrame["params"]
    self.assertEqual(params.logl, 33, "We don't get out of the frame what we put in")
    self.assertEqual(params.rlogl, 42, "We don't get out of the frame what we put in")
    self.assertEqual(params.ndof, 55, "We don't get out of the frame what we put in")
    self.assertEqual(params.nmini, 36, "We don't get out of the frame what we put in")

  def tearDown(self):
    if os.path.exists(theFileName):
      os.remove(theFileName)
    self.assertEqual(os.path.exists(theFileName), False, "Warning, the file did not get removed properly")                     

newI3File.close()

unittest.main()