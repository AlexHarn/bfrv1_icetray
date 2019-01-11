#!/usr/bin/env python

from I3Tray import *
from os.path import expandvars

from icecube.linefit.test_modules.LFTestCompare import LFTestCompare


## Test6: Compare all the variables from the new fit to what was expected 
## for this one event.
  
tray = I3Tray()

tray.AddModule("I3Reader", "reader",
	FileName=os.environ['I3_TESTDATA'] + '/reco-toolbox/I3TestEvent_Pulse_1.i3.gz'
	)

tray.AddModule("I3LineFit","linefit")(
    ("InputRecoPulses", "Pulses"))

#------ uncomment these lines to see event -----
#load("libeventviewer")
#tray.AddModule("I3EventViewerModule","viewer")
#-----------------------------------------------

tray.AddModule(LFTestCompare,"test")


    
tray.Execute()

