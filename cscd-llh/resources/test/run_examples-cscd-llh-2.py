#!/usr/bin/env python

import sys
import unittest
import os
import subprocess

from icecube.icetray import I3Test

class TestSimpleExample(I3Test.TestExampleScripts):
    project_name = "cscd-llh"


# minAlgorithm: Options are MIGRAD, SIMPLEX, and MINIMIZE

# Minimizer: Options are "Brent", "GfxMinimizer", "Minuit", "Powell", and "Simplex"
# PDF: Options are "UPandel", "UPandelMpe", "HitNoHit", "HitNoHitMpe", and "PndlHnh","HnhDir "


#---------- 2) ----------------
    def test_b(self):
        minimizer='Powell'
        pdf='HitNoHit'
        input_file = self.I3_TESTDATA + "/event-viewer/Level3aGCD_IC79_EEData_Run00115990.i3"
        self.run_example('example-cscd-llh.py','-i',input_file,'-m',minimizer,'-p', pdf, '--useparamenergy', '--useampweightpower')

    def tearDown(self):
        '''
        Be nice and clean up after yourself.
        '''
        if os.path.exists("./CscdLlhPlots.root"):
            os.remove("./CscdLlhPlots.root")
             
unittest.main()
