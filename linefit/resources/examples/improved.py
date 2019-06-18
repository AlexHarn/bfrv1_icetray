#!/usr/bin/env python

#This is a complete example for the improved linefit code.  
from os.path import expandvars
from I3Tray import *
from icecube import icetray, dataclasses, dataio, phys_services, linefit, DomTools
#Get name of data file
i3testdata = expandvars("$I3_TESTDATA")

infile = [
    i3testdata+'/GCD/GeoCalibDetectorStatus_2012.56063_V0.i3.gz',
    i3testdata+'/sim/Level3_nugen_numu_IC86.2012.011069.000000_20events.i3.bz2']

tray = I3Tray()
tray.AddModule("I3Reader", "reader")(
    ("filenamelist", infile),
)

##################################################################
# Run improved Linefit
##################################################################
#Note that this example illustrates two ways to use this code.  You can use the
#code as tray segment, or as 4 modules.  

#Run as a tray segment 
tray.AddSegment(linefit.simple,"example", inputResponse = "HVInIcePulses", fitName = "linefit_improved")
# 
# #Run as separate modules
tray.AddModule("DelayCleaning", "DelayCleaning", InputResponse =
"HVInIcePulses", OutputResponse="Pulses_delay_cleaned")

tray.AddModule("HuberFit", "HuberFit", Name = "HuberFit", InputRecoPulses =
"Pulses_delay_cleaned")

tray.AddModule("Debiasing", "Debiasing", OutputResponse = "debiasedHits",
InputResponse = "Pulses_delay_cleaned", Seed = "HuberFit")

tray.AddModule("I3LineFit","linefit_final", Name = "linefit_final",
InputRecoPulses = "debiasedHits", LeadingEdge= "ALL", AmpWeightPower= 0.0)


#"linefit_final" is the final product of whole series. 
##################################################################
# Write output to file and move to next frame
##################################################################

tray.AddModule("I3Writer","writer")(
	#("Streams",[icetray.I3Frame.Physics]),	
	("FileName","example.i3"),
	("CompressionLevel",0),
)

tray.Execute()

