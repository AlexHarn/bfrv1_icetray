#!/usr/bin/env python

import os
import unittest

from I3Tray import *
from icecube import icetray, dataio, dataclasses
from icecube import gulliver_bootstrap

theFileName = "TheFile.i3.gz"

theI3File = dataio.I3File(theFileName, 'w')


def PutStuffInTheFrame(i3File):
  frame = icetray.I3Frame(icetray.I3Frame.DAQ)

  params = gulliver_bootstrap.BootstrapParams()
  params.status = gulliver_bootstrap.BootstrapParams.OK
  params.successfulFits = 33
  params.totalFits = 42

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
    self.assertEqual(params.status, gulliver_bootstrap.BootstrapParams.OK, "these should be the same.")
    self.assertEqual(params.successfulFits, 33, "these should be the same.")
    self.assertEqual(params.totalFits, 42, "these should be the same.")

  def tearDown(self):
    if os.path.exists(theFileName):
      os.remove(theFileName)
    self.assertEqual(os.path.exists(theFileName), False)                     

newI3File.close()

unittest.main()