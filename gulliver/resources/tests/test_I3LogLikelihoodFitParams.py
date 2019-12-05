#!/usr/bin/env python

import unittest
import math

from I3Tray import *
from icecube import gulliver

class TestI3LogLikelihoodFitParams(unittest.TestCase):

  def test_reset(self):
    params = gulliver.I3LogLikelihoodFitParams()
    params.logl = 33.
    params.rlogl = 42.
    params.ndof = 55
    params.nmini = 36

    params.reset()

    self.assertTrue(math.isnan(params.logl))
    self.assertTrue(math.isnan(params.rlogl))
    self.assertEqual(params.ndof, -1)
    self.assertEqual(params.nmini, -1)

  def test_print(self):
    params = gulliver.I3LogLikelihoodFitParams()
    params.logl = 33.
    params.rlogl = 42.
    params.ndof = 55
    params.nmini = 36

    print(params)

unittest.main()