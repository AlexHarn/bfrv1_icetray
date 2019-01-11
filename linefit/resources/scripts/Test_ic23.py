#!/usr/bin/env python

from I3Tray import *
from os.path import expandvars

from icecube.linefit.test_modules.LFTestIC23 import LFTestIC23

from icecube import dataio

load("libdataclasses")
load("liblinefit")
load("libdataio")

## Test7: Compare all the variables from the new fit to what was expected 
## for this one event.
  
tray = I3Tray()

tray.AddModule("I3Reader", "reader",
	FileName=os.environ['I3_TESTDATA'] + '/reco-toolbox/I3TestEvent_Pulse_2.i3.gz'
	)

tray.AddModule("I3LineFit","linefit")(
    ("InputRecoPulses", "Pulses"))

#------ uncomment these lines to see event -----
#load("libeventviewer")
#tray.AddModule("I3EventViewerModule","viewer")
#-----------------------------------------------

tray.AddModule(LFTestIC23,"test")


    
tray.Execute()

