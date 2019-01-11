#!/usr/bin/env python

from os.path import expandvars

infile = expandvars("$I3_TESTDATA") + "/icetop/Raw_IC79_data_Run00117306_10events_IT.i3.bz2"

from I3Tray import *
from icecube import icetray, dataclasses, dataio, phys_services, WaveCalibrator, tpx, topeventcleaning, toprec

tray = I3Tray()

#**************************************************
#                  Read file
#**************************************************
tray.AddModule("I3Reader","reader")(
    ("FileName", infile)
    )


#**************************************************
#      Wave Calibrator / WaveformSplitter
#**************************************************
waveforms = 'IceTopWaveForms'
tray.AddModule('I3WaveCalibrator', 'wave_calibrator',
    Launches = 'IceTopRawData',
    Waveforms = waveforms,
    FADCSaturationMargin = 900,
    Errata = 'IceTopCalibrationErrata',
    )

hlc_atwd_waveforms = 'IceTopHLCATWDWaveforms'
tray.AddModule('I3WaveformSplitter', 'waveform_splitter',
    Input = waveforms,
    Force = True,  # ! put all maps in the frame, even if they are empty
    SplitATWDChannels = False,
    SplitATWDChips = False,
    PickUnsaturatedATWD = True,  # ! do not keep all ATWDs, but only the highest non-saturated gain one
    HLC_ATWD = hlc_atwd_waveforms,
    HLC_FADC = 'IceTopHLCFADCWaveforms',
    SLC = 'IceTopSLCWaveforms'
    )


#**************************************************
#                        tpx
#**************************************************

hlc_vem_pulses = 'IceTopHLCVEMPulses'
tray.AddModule('I3TopHLCPulseExtractor', 'top_hlc_pulse_extractor',
    Waveforms = hlc_atwd_waveforms,
    PulseInfo = 'IceTopPulseInfo',
    VEMPulses = hlc_vem_pulses
    )

tank_pulses = 'IceTopTankPulses'
tank_pulse_merger_excluded = 'IceTopTankPulseMergerExcludedTanks'
tray.AddModule('I3HLCTankPulseMerger', 'tank_pulse_merger',
    InputVEMPulses = hlc_vem_pulses,
    BadDomList = 'IceTopBadDOMs',
    BadTankList = 'IceTopBadTanks',
    OutputTankPulses = tank_pulses,
    ExcludedTanks = tank_pulse_merger_excluded,
    MaxHGLGTimeDiff = 40*I3Units.ns
    )

#**************************************************
#                  topeventcleaning
#**************************************************

cleaned_pulses = 'CleanedTankPulses'
excluded_tanks = 'ClusterCleaningExcludedTanks'
physics_stream = 'icetop_physics'
tray.AddModule('I3TopHLCClusterCleaning', physics_stream,
    InputPulses = tank_pulses,
    BadTankList = tank_pulse_merger_excluded,
    OutputPulses = cleaned_pulses,
    ExcludedTanks = excluded_tanks,
    InterStationTimeTolerance = 200*I3Units.ns,
    IntraStationTimeTolerance = 200*I3Units.ns
    )

def DiscardSmallSubEvents(frame, SubEventStream = '', Pulses = '', MinNStation = 3):
    """
    Discard all P frames of given SubEventStream where the number of stations (calculated from Pulses) is smaller than MinNStations.
    Keeps all events where Pulses do not exist.
    """
    if not 'I3EventHeader' in frame or frame['I3EventHeader'].sub_event_stream != SubEventStream or not Pulses in frame:
        return True
    pulseMap = dataclasses.I3RecoPulseSeriesMap.from_frame(frame, Pulses)
    stationList = set([pulse.key().string for pulse in pulseMap])
    nstations = len(stationList)
    return nstations >= MinNStation

tray.AddModule(DiscardSmallSubEvents, 'discard_small_subevents',
    SubEventStream = physics_stream,
    Pulses = cleaned_pulses,
    MinNStation = 5
    )

    

#**************************************************
#     first guess reconstruction - plane & cog
#**************************************************

tray.AddModule("I3TopRecoCore", "core",
    DataReadout = cleaned_pulses,
    NTanks = 7
    )

tray.AddModule("I3TopRecoPlane","plane",
    DataReadout = cleaned_pulses
    )

#**************************************************
#                     lateralfit
#**************************************************

#tray.AddModule("I3TopLateralFit", "latfit",
#    DataReadout = cleaned_pulses,
## BadStations is deprecated
##   BadStations = excluded_stations
#    )

tray.AddModule("Dump", "dump")

#**************************************************
#                Write to I3-file
#**************************************************

# outfile = outdir + "run_evbuilder_with_toprec.i3"
#tray.AddModule("I3Writer","write")(
#    ("filename","test_evbuilder_with_toprec.i3"),
#    )




tray.Execute()

