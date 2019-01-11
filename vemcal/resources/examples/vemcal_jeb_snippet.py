#!/usr/bin/env python

# !!! THIS SCRIPT IS NOT EXECUTABLE !!!
#
#  It is just is an example how to
#  integrate the IceTop Muon Calibration 
#  into the JEB.
#

from I3Tray import *

load("libdataclasses")
load("libpfclasses")
load("libdataio")
load("libDOMcalibrator")
load("libtopwaveprocessor")
load("libvemcal")
load("libicepick")


tray = I3Tray()


#**************************************************
#              IceTop DOMcalibrator
#**************************************************
domcal_subtract_baseline     = False
domcal_subtract_transittime  = True
domcal_correct_pedestaldroop = False
domcal_calibration_mode      = 1
domcal_ATWD_saturation_level = 900

# Calibrate all IceTopMinBias hits
tray.AddModule("I3DOMcalibrator","domcal_minbias")(
    ("InputRawDataName",            "IceTopMinBias"),
    ("SubtractBaseline",            domcal_subtract_baseline),
    ("SubtractTransitTime",         domcal_subtract_transittime),
    ("CorrectPedestalDroopDualTau", domcal_correct_pedestaldroop),
    ("CalibrationMode",             domcal_calibration_mode),
    ("ATWDSaturationLevel",         domcal_ATWD_saturation_level),
    ("OutputATWDDataName",          "IceTopMinBiasCalATWD"),
    ("OutputFADCDataName",          "IceTopMinBiasCalFADC"),
    )

# Select only HG-LG pairs from IceTopRawData to  
# calibrate them in the next step. This way we only
# calibrate and process the relevant hits in order
# to save computing time
tray.AddModule("I3HGLGPairSelector","hglg_selector")(
    ("InputDOMLaunches","IceTopRawData"),
    ("OutputDOMLaunches","IceTopHGLGData"),
    )

# Calibrate the HG-LG pairs (only a few per event)
tray.AddModule("I3DOMcalibrator","domcal_hglg")(
    ("InputRawDataName",            "IceTopHGLGData"),
    ("SubtractBaseline",            domcal_subtract_baseline),
    ("SubtractTransitTime",         domcal_subtract_transittime),
    ("CorrectPedestalDroopDualTau", domcal_correct_pedestaldroop),
    ("CalibrationMode",             domcal_calibration_mode),
    ("ATWDSaturationLevel",         domcal_ATWD_saturation_level),
    ("OutputATWDDataName",          "IceTopHGLGCalATWD"),
    ("OutputFADCDataName",          "IceTopHGLGCalFADC"),
    )

#**************************************************
#              IceTop TopWaveProcessor
#**************************************************
itwp_baseline_method     = 0
itwp_correct_transittime = not domcal_subtract_transittime

tray.AddModule("I3TopWaveProcessor","ITWP_minbias")(
    ("InputWaveforms",     "IceTopMinBiasCalATWD"),
    ("BaseLineMethod",     itwp_baseline_method),
    ("CorrectTransitTime", itwp_correct_transittime),
    ("OutputHLCPulses",   "IceTopMinBiasPulses"),
    )

tray.AddModule("I3TopWaveProcessor","ITWP_hglg")(
    ("InputWaveforms",     "IceTopHGLGCalATWD"),
    ("BaseLineMethod",     itwp_baseline_method),
    ("CorrectTransitTime", itwp_correct_transittime),
    ("OutputHLCPulses",   "IceTopHGLGPulses"),
    )

#**************************************************
#              IceTop VEMCalExtractor
#**************************************************

tray.AddModule("I3VEMCalExtractor","vemcal_extract")(
    ("IceTopMinBiasPulsesName", "IceTopMinBiasPulses"),
    ("IceTopPulsesName",        "IceTopHGLGPulses"),
    ("ShowerVeto",              "IceTopRawData"),
    )

#**************************************************
#           IceTop MuonCalibration Filter
#**************************************************

tray.AddModule("I3IcePickModule<I3FrameObjectFilter>","vemcal_filter") (
    ("FrameObjectKey","I3VEMCalData"),
    ("DecisionName","IceTopMuonCalibration_10"),
    ("DiscardEvents",False),
    )

