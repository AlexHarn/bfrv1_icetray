#!/usr/bin/env python

import os
import unittest
from icecube.icetray import I3Test

class TestSimpleExample(I3Test.TestExampleScripts):

    project_name = "frame_object_diff"

    def test_sim_2012_2013(self):
        '''
        Test that runs the one example script in this project.
        '''
        self.run_example('sim_2012_2013.py')

    def tearDown(self):
        '''
        Be nice and clean up after yourself.
        '''
        os.remove("./out.i3.gz")

unittest.main()
