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

  def test_comparison(self):
    params1 = gulliver.I3LogLikelihoodFitParams()
    params1.logl = 33.
    params1.rlogl = 42.
    params1.ndof = 55
    params1.nmini = 36

    params2 = gulliver.I3LogLikelihoodFitParams()
    params2.logl = 33.
    params2.rlogl = 42.
    params2.ndof = 55
    params2.nmini = 36

    self.assertTrue(params1 == params2)

  def test_print(self):
    params = gulliver.I3LogLikelihoodFitParams()
    params.logl = 33.
    params.rlogl = 42.
    params.ndof = 55
    params.nmini = 36

    print(params)


unittest.main()