#! /usr/bin/env python
import unittest
import sys
import os
import subprocess as subp


class ExamplesTestSuite( unittest.TestCase ):

    def runtest( self, example_name, input_filename ):
        ret_code = subp.call(
	    ("../examples/" + example_name, os.path.expandvars(input_filename)),
            cwd=os.path.dirname( __file__ ) # remove if you run in test/ folder
	)
        self.assertEqual( ret_code, 0, "return code should be zero" )

    def test_select_domlaunches( self ):
        self.runtest( "select_domlaunches.py",
                      "$I3_TESTDATA/event-viewer/Level3aGCD_IC79_EEData_Run00115990_slim.i3" )

    def test_select_recopulses( self ):
        self.runtest( "select_recopulses.py",
                      "$I3_TESTDATA/event-viewer/Level3aGCD_IC79_EEData_Run00115990_slim.i3" )


if __name__ == "__main__":
    unittest.main()
