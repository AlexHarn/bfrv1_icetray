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


#---------- 7) ----------------
    def test_g(self):
        minimizer_collection = ['Powell', 'Simplex']
        pdf_collection = ['UPandel', 'UPandelMpe', 'PndlHnh']
        
        input_file = self.I3_TESTDATA + "/event-viewer/Level3aGCD_IC79_EEData_Run00115990.i3"
        
        number=100
        for minimizer in minimizer_collection:
            number += 1  
            for pdf in pdf_collection:
                number += 1
                cscd_llh_name = "cscd-llh" + str(number)
                name = "Vertex_Reco_"+minimizer+"_"+pdf
                
                self.run_example('example-cscd-llh.py','-i',input_file,'-m',minimizer,'-p', pdf,'-n',name,'--useparamt','--maxcalls=50000')

    def tearDown(self):
        '''
        Be nice and clean up after yourself.
        '''
        if os.path.exists("./CscdLlhPlots.root"):
            os.remove("./CscdLlhPlots.root")
             
unittest.main()
