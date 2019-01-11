#!/usr/bin/env python

import sys
import unittest
import os
import unittest
import subprocess
from icecube.icetray import I3Test


# backward-compatibility fix of unittest, if
# expectedFailure decorator is not available, add it
if not hasattr(unittest, "expectedFailure"):
    def expectedFailure(func):
        def decorated(self, *args, **kwargs):
            class NoExpectedFailure: pass
            try:
                func(self, *args, **kwargs)
                raise NoExpectedFailure
            except NoExpectedFailure:
                raise AssertionError("failure was expected")
            except: # all other failures
                pass
        return decorated
    unittest.expectedFailure = expectedFailure


class TestSimpleExample(I3Test.TestExampleScripts):
    project_name = "cscd-llh"

# minAlgorithm: Options are MIGRAD, SIMPLEX, and MINIMIZE

# Minimizer: Options are "Brent", "GfxMinimizer", "Powell", and "Simplex"
# PDF: Options are "UPandel", "UPandelMpe", "HitNoHit", "HitNoHitMpe", and "PndlHnh","HnhDir "


#----------- tests that should fail 

    @unittest.expectedFailure
    def test_aa(self):
        minimizer_collection = ['BlahBlah']
        pdf_collection = ['HitNoHitMpe']
        
        input_file = self.I3_TESTDATA + "/event-viewer/Level3aGCD_IC79_EEData_Run00115990.i3"
        
        number=1200
        for minimizer in minimizer_collection:
            number += 10  
            for pdf in pdf_collection:
                number += 1
                cscd_llh_name = "cscd-llh" + str(number)
                name = "Vertex_Reco_"+minimizer+"_"+pdf
                print(name)
                self.run_example('example-cscd-llh.py','-i',input_file,'-m',minimizer,'-p', pdf,'-n',name)
                

    @unittest.expectedFailure
    def test_ab(self):
        minimizer_collection = ['BlahBlah']
        pdf_collection = ['HitNoHit']
        
        input_file = self.I3_TESTDATA + "/event-viewer/Level3aGCD_IC79_EEData_Run00115990.i3"
        
        number=1300
        for minimizer in minimizer_collection:
            number += 10  
            for pdf in pdf_collection:
                number += 1
                cscd_llh_name = "cscd-llh" + str(number)
                name = "Vertex_Reco_"+minimizer+"_"+pdf
                print(name)
                self.run_example('example-cscd-llh.py','-i',input_file,'-m',minimizer,'-p', pdf,'-n',name)


    @unittest.expectedFailure
    def test_ac(self):
        minimizer='Powell'
        pdf='HitNoHit'
        input_file = self.I3_TESTDATA + "/event-viewer/Level3aGCD_IC79_EEData_Run00115990.i3"
        self.run_example('example-cscd-llh.py','-i',input_file,'-m',minimizer,'-p', pdf, '--useparamenergy', '--useampweightpower', '--minuitverbositylevel=blahblah')      

    @unittest.expectedFailure
    def test_ad(self):
        minimizer='Powell'
        pdf='HitNoHit'
        input_file = self.I3_TESTDATA + "/event-viewer/Level3aGCD_IC79_EEData_Run00115990.i3"
        self.run_example('example-cscd-llh.py','-i',input_file,'-m',minimizer,'-p', pdf, '--useparamenergy', '--useampweightpower', '--minuitprintwarnings=blahblah')      


    @unittest.expectedFailure
    def test_ae(self):
        minimizer='Powell'
        pdf='HitNoHit'
        input_file = self.I3_TESTDATA + "/event-viewer/Level3aGCD_IC79_EEData_Run00115990.i3"
        self.run_example('example-cscd-llh.py','-i',input_file,'-m',minimizer,'-p', pdf, '--useparamenergy', '--useampweightpower', '--minuitstrategy=blahblah')      


    @unittest.expectedFailure
    def test_af(self):
        minimizer='Powell'
        pdf='BlahBlah'
        input_file = self.I3_TESTDATA + "/event-viewer/Level3aGCD_IC79_EEData_Run00115990.i3"
        self.run_example('example-cscd-llh.py','-i',input_file,'-m',minimizer,'-p', pdf, '--useparamenergy', '--useampweightpower')      

    @unittest.expectedFailure
    def test_ah(self):
        minimizer='Powell'
        pdf=''
        input_file = self.I3_TESTDATA + "/event-viewer/Level3aGCD_IC79_EEData_Run00115990.i3"
        self.run_example('example-cscd-llh.py','-i',input_file,'-m',minimizer,'-p', pdf, '--useparamenergy', '--useampweightpower')      




    


unittest.main()
