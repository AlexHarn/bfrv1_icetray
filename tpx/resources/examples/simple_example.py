#!/usr/bin/env python
# -*- encoding: utf-8 -*-

import os.path

from icecube import icetray, dataclasses, I3Db, WaveCalibrator, dataio, icepick, phys_services, tpx
from I3Tray import *

load('libBadDomList')
load('libdaq-decode')
load('libfilter-tools')
load('libpayload-parsing')

import argparse
parser = argparse.ArgumentParser(description='Simple TPX example.')
parser.add_argument('-o', '--output', metavar='i3_file', type=str, help='Output filename',
                    default=os.path.expandvars('${I3_BUILD}/simple_example_tpx.i3'))
parser.add_argument('inputfile', metavar='i3_file', type=str, nargs='?', help='Input file',
                    default='/data/exp/IceCube/2011/filtered/PFFilt/0701/PFFilt_PhysicsTrig_PhysicsFiltering_Run00118392_Subrun00000000_00000061.tar.bz2')
args = parser.parse_args()

dbserver    = 'dbs2.icecube.wisc.edu'
dbuser      = 'www'
# inputfile = '/data/exp/IceCube/2010/filtered/level2/0701/Level2_IC79_data_Run00116111_Part00000000_IT.i3.gz'
inputfile   = args.inputfile
outputfile  = args.output


tray = I3Tray()

workspace = os.path.expandvars('${I3_BUILD}')
xmlf = workspace + '/BadDomList/resources/scripts/QueryConfiguration.xml'
tray.AddService('I3BadDomListFactory', 'BadDomList',
    QFileName = xmlf
    )

tray.AddService('I3DbOMKey2MBIDFactory', 'OmKey2MbId',
    Host     = dbserver,
    Username = dbuser,
    Database = 'I3OmDb'
    )

tray.AddService('I3DbGeometryServiceFactory', 'Geometry',
    Host             = dbserver,
    Username         = dbuser,
    Database         = 'I3OmDb',
    CompleteGeometry = False
    )

tray.AddService('I3DbCalibrationServiceFactory', 'Calibration',
    Host     = dbserver,
    Username = dbuser,
    Database = 'I3OmDb'
    )

tray.AddService('I3DbDetectorStatusServiceFactory', 'Status',
    Host     = dbserver,
    Username = dbuser,
    Database = 'I3OmDb'
    )

tray.AddService('I3PayloadParsingEventDecoderFactory', 'EventDecoder',
    HeaderId       = 'I3DAQEventHeader',
    TriggerId      = 'I3TriggerHierarchy',
    CpuDataId      = 'BeaconData',
    MinBiasID      = 'MinBias',
    SpecialDataId  = 'SyncPulseMap',
    SpecialDataOms = [OMKey(0, 91),
                      OMKey(0, 92)]
    )

tray.AddModule('I3Reader', 'Reader',
    Filename = inputfile
    )

tray.AddModule('I3MetaSynth', 'challengeassumptions',
    GeometryService       = 'I3GeometryService',
    CalibrationService    = 'I3CalibrationService',
    DetectorStatusService = 'I3DetectorStatusService'
)

tray.AddModule('I3IcePickModule<FilterMaskFilter>', 'FiltermaskFilter',
    FilterResultName = 'FilterMask',
    FilterNameList   = ['IceTopSTA8_11',
                        'IceTopSTA3_11'],
    DiscardEvents    = True,
    DecisionName     = ''
    )

tray.AddModule('QConverter', 'nullsplitter')
#     WritePFrame = True,
#     QKeys       = ['BadDomsList',
#                    'BadDomsListSLC',
#                    'DrivingTime',
#                    'FilterMask',
#                    'PoleCascadeLinefit',
#                    'PoleCascadeLinefitParams',
#                    'PoleEHESummaryPulseInfo',
#                    'PoleMuonLinefit',
#                    'PoleMuonLinefitParams',
#                    'PoleMuonLlhFit',
#                    'PoleMuonLlhFitFitParams',
#                    'PoleMuonLlhFitMuE',
#                    'PoleToI',
#                    'PoleToIParams',
#                    'NFEMergedPulses',
#                    'NFEMergedPulsesExtraInfo',
#                    'TWNFEMergedPulsesHLC',
#                    'Pole_SLC_HLCLinefit',
#                    'Pole_SLC_HLCLinefitParams',
#                    'Pole_SLC_HLCLlhFit',
#                    'Pole_SLC_HLCLlhFitFitParams']
# )

tray.AddModule('I3FrameBufferDecode', 'Decoder')

tray.AddModule('I3BadDomListModule', 'BadDomListModule',
    BadDomsListVector = 'BadDomsList'
    )

tray.AddModule('I3WaveCalibrator', 'WaveCalibrator',
    Launches             = 'IceTopRawData',
    Waveforms            = 'IceTopCalibratedWaveforms',
#   ATWDMode             = WaveCalibrator.CalibrationMode.CALIBRATE_NONSATURATED,
    ATWDSaturationMargin = 123,   # 1023-900 == 123
    FADCSaturationMargin = 0,
    )

tray.AddModule('I3WaveformSplitter', 'WaveformSplitter',
    Input               = 'IceTopCalibratedWaveforms',
    PickUnsaturatedATWD = True,
    HLC_ATWD            = 'IceTopCalibratedATWD',
    HLC_FADC            = 'IceTopCalibratedFADC',
    SLC                 = 'IceTopCalibratedSLC',
    Force               = True                        # Put empty maps in the frame as well
    )

tray.AddModule('I3TopHLCPulseExtractor', 'TopHLCPulseExtractor',
    Waveforms        = 'IceTopCalibratedATWD',     # Default
    PEPulses         = 'IceTopHLCPEPulses',
    VEMPulses        = 'IceTopHLCVEMPulses',
    PulseInfo        = 'IceTopHLCPulseInfo',
    )

tray.AddModule('I3TopSLCPulseExtractor', 'TopSLCPulseExtractor',
    Waveforms = 'IceTopCalibratedSLC',     # Default
    PEPulses  = 'IceTopSLCPEPulses',
    VEMPulses = 'IceTopSLCVEMPulses'
    )

tray.AddModule('I3EventCounter', 'Eventcounter',
    PhysicsCounterName = 'Events',
    NEvents            = 10
    )

tray.AddModule('I3Writer', 'Writer',
    Filename = outputfile,
    SkipKeys = ['IceTopCalibratedFADC',
                'IceTopCalibratedWaveforms',
                'I3DAQData',
                'I3DAQEventHeader',
                'IceTopBeaconData',
                'IceTopMinBias',
                'IceTopRawData',
                'InIceBeaconData',
                'InIceMinBias',
                'InIceRawData'],
    Streams  = [icetray.I3Frame.Geometry,
                icetray.I3Frame.Calibration,
                icetray.I3Frame.DetectorStatus,
                icetray.I3Frame.DAQ]
    )



tray.Execute()

