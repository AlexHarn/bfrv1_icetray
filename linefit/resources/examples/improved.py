#!/usr/bin/env python

#This is a complete example for the improved linefit code.  
from os.path import expandvars
from I3Tray import *
from icecube import icetray, dataclasses, dataio, phys_services, linefit, DomTools
#Get name of data file
i3testdata = expandvars("$I3_TESTDATA")
infile = [i3testdata+"/2007data/2007_I3Only_Run109732_Nch20.i3.gz"]
##################################################################
# BOOT INTO THE TRAY
##################################################################

load("libNFE")

tray = I3Tray()
tray.AddModule("I3Reader", "reader")(
    ("filenamelist", infile),
)

#Handle the Q frame transition
tray.AddModule("QConverter","myQ",WritePFrame = True)

#Stop after 10 frames
tray.AddModule("I3EventCounter","nprocessed")(
    ("NEvents",5),
    ("CounterStep",1),
)
##################################################################
# Run Generic Feature Extraction
##################################################################
'''
tray.AddModule("I3DOMcalibrator", "calibrator")(
   ("InputRawDataName",          "InIceRawData"),
   ("subtractTransitTime",                 True),   # default
   ("OutputATWDDataName", "InIceCalibratedATWD"),
   ("OutputFADCDataName", "InIceCalibratedFADC"),
   ("FADCTimeOffset",                     -15.0),
   ("CalibrateDataWithSLC",                True),   # altough there is no SLC in runfile
   ("ATWDSaturationLevel",                  900),   # default since May 2010
   )
'''

tray.AddModule("I3NFE", "atwd_extractor")(
   ("InputWaveformName",   "CalibratedATWD"),
   ("OutputPulseMapName",       "NFE_ATWDPulses"),
   )

tray.AddModule("I3NFE", "fadc_extractor")(
   ("InputWaveformName",   "CalibratedFADC"),
   ("OutputPulseMapName",       "NFE_FADCPulses"),
   )

tray.AddModule("I3NFEPulseMerger", "merger")(
   ("ATWDPulseMapName",         "NFE_ATWDPulses"),
   ("FADCPulseMapName",         "NFE_FADCPulses"),
   ("MergedPulseMapName", "NFEMergedPulses"),   # default
   ("AlreadySorted",                        True),
)
				
##################################################################
# Run improved Linefit
##################################################################
#Note that this example illustrates two ways to use this code.  You can use the
#code as tray segment, or as 4 modules.  

#Run as a tray segment 
tray.AddSegment(linefit.simple,"example", inputResponse = "NFEMergedPulses", fitName = "linefit_improved")
# 
# #Run as separate modules
tray.AddModule("DelayCleaning", "DelayCleaning", InputResponse =
"NFEMergedPulses", OutputResponse="Pulses_delay_cleaned")

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

