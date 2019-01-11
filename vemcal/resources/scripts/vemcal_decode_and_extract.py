#!/usr/bin/env python

from I3Tray import *

from os.path import expandvars, dirname, basename   # needed for path- and filename manipulations
from string  import *                               # needed for string manipulations i.e replace

import glob


load("libdataclasses")
load("libpfclasses")
load("libdataio")
load("libI3Db")
load("libpayload-parsing")
load("libdaq-decode")
load("libDOMcalibrator")
load("libtopwaveprocessor")
load("libicepick")
load("libfilter-tools")
load("libvemcal")


tray = I3Tray()

in_dir = "/mnt/fpmaster/pnflocal2/tilo"

# Get data files
file_pattern = '%s/PFFilt_PhysicsTrig_PhysicsFiltering_Run[0-9][0-9][0-9][0-9][0-9][0-9][0-9][0-9]_Subrun00000000_[0-9][0-9][0-9][0-9][0-9][0-9][0-9][0-9].i3' % (in_dir)

file_list = glob.glob(file_pattern)
file_list.sort()

dbserver = "sps-fpmaster.sps.icecube.southpole.usap.gov"
dbuser   = "pnf"

#**************************************************
#                 Reader & Muxer 
#**************************************************
tray.AddService("I3ReaderServiceFactory","readerfactory")(
    ("FileNameList",    file_list),
    ("OmitGeometry",    True),
    ("OmitCalibration", True),
    ("OmitStatus",      True),
    ("SkipMissingDrivingTime", True),
    )

tray.AddService( "I3DbOMKey2MBIDFactory","OmKey2MbId")(
    ("Host",     dbserver ),
    ("Username", dbuser ),
    ("Database", "I3OmDb" )
    )

tray.AddService( "I3DbGeometryServiceFactory", "Geometry")(
    ("Host",             dbserver ),
    ("Username",         dbuser ),
    ("Database",         "I3OmDb" ),
    ("CompleteGeometry", False )
    )

tray.AddService( "I3DbCalibrationServiceFactory","Calibration")(
    ("Host",     dbserver ),
    ("Username", dbuser ),
    ("Database", "I3OmDb" )
    )

tray.AddService( "I3DbDetectorStatusServiceFactory","Status")(
    ("Host",     dbserver ),
    ("Username", dbuser ),
    ("Database", "I3OmDb" )
    )

tray.AddService( "I3PayloadParsingEventDecoderFactory", "EventDecoder" ) (
    ( "HeaderId",                   "I3DAQEventHeader" ),       # Default
    ( "TriggerId",                  "I3TriggerHierarchy" ),     # Default
    ( "CpuDataId",                  "BeaconData" ),             # ! Name Beacon data launch series
    ( "MinBiasID",                  "MinBias"),
    ( "SpecialDataId",              "SyncPulseMap" ),           # ! Name TWR sync pulse launch series
    ( "SpecialDataOms",             [ OMKey(0,91),              # ! Identify sync pulse OMs
                                      OMKey(0,92) ] )
    )


tray.AddService("I3IcePickInstaller<FilterMaskFilter>", "minbias_filter")(
    ("ServiceName", "MinBiasFilter"),
    ("FilterResultName","FilterMask"),
    ("FilterNameList", ["IceTopMinBias_09"])
    )


tray.AddModule("I3Muxer","muxme")

tray.AddModule("I3IcePickModule<FilterMaskFilter>", "filtermask_filter")(
    ("FilterResultName","FilterMask"),
    ("FilterNameList", ["IceTopSTA3_09",
                        "IceTopSTA8_09",
                        "IceTopSTA3_InIceSMT_09",
                        "IceTopSTA8_InIceSMT_09",
                        "InIceSMT_IceTopCoincidence_09",
                        "IceTopMinBias_09"]),
    ("DiscardEvents", True),
    )


tray.AddModule("I3FrameBufferDecode","decode")



#**************************************************
#                  DOMcalibrator
#**************************************************
domcal_subtract_baseline     = False
domcal_subtract_transittime  = True
domcal_correct_pedestaldroop = False
domcal_calibration_mode      = 1
domcal_ATWD_saturation_level = 900

tray.AddModule("I3DOMcalibrator","domcal_minbias")(
    ("InputRawDataName",     "IceTopMinBias"),
    ("SubtractBaseline",     domcal_subtract_baseline),
    ("SubtractTransitTime",  domcal_subtract_transittime),
    ("CorrectPedestalDroop", domcal_correct_pedestaldroop),
    ("CalibrationMode",      domcal_calibration_mode),
    ("ATWDSaturationLevel",  domcal_ATWD_saturation_level),
    ("OutputATWDDataName",   "IceTopMinBiasCalATWD"),
    ("OutputFADCDataName",   "IceTopMinBiasCalFADC"),
    )

tray.AddModule("I3DOMcalibrator","domcal_physics")(
    ("InputRawDataName",     "IceTopRawData"),
    ("SubtractBaseline",     domcal_subtract_baseline),
    ("SubtractTransitTime",  domcal_subtract_transittime),
    ("CorrectPedestalDroop", domcal_correct_pedestaldroop),
    ("CalibrationMode",      domcal_calibration_mode),
    ("ATWDSaturationLevel",  domcal_ATWD_saturation_level),
    ("OutputATWDDataName",   "IceTopCalATWD"),
    ("OutputFADCDataName",   "IceTopCalFADC"),
    )


#**************************************************
#                 TopWaveProcessor
#**************************************************
itwp_baseline_method     = 0
itwp_correct_transittime = not domcal_subtract_transittime

tray.AddModule("I3TopWaveProcessor","ITWP_minbias")(
    ("InputWaveforms",     "IceTopMinBiasCalATWD"),
    ("BaseLineMethod",     itwp_baseline_method),
    ("CorrectTransitTime", itwp_correct_transittime),
    ("OutputHLCPulses",   "IceTopMinBiasPulses"),
    )

tray.AddModule("I3TopWaveProcessor","ITWP_physics")(
    ("InputWaveforms",     "IceTopCalATWD"),
    ("BaseLineMethod",     itwp_baseline_method),
    ("CorrectTransitTime", itwp_correct_transittime),
    ("OutputHLCPulses",   "IceTopPulses"),
    )

#**************************************************
#               VEMCal Extraction
#**************************************************

tray.AddModule("I3VEMCalExtractor","vemcal_extract")(
    ("IceTopMinBiasPulsesName", "IceTopMinBiasPulses"),
    ("IceTopPulsesName",        "IceTopPulses"),
    ("ShowerVeto",              "IceTopRawData"),
    )


tray.AddModule("I3IcePickModule<I3FrameObjectFilter>","select")(
    ("FrameObjectKey","I3VEMCalData"),
    ("DiscardEvents", True),
    )

tray.AddModule("Keep","keep")(
    ("Keys",["I3Geometry",
             "I3Calibration",
             "I3DetectorStatus",
             "DrivingTime",
             "I3VEMCalData"]),
    )

tray.AddModule("I3Writer","write")(
    ("FileName", "IceTop_VEMCalData.i3.gz"),
    #("Streams", ["Physics"]),
    )

tray.AddModule("Dump","dump")



tray.Execute()


