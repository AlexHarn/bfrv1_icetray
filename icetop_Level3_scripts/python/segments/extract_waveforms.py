from icecube import icetray, WaveCalibrator
from icecube.icetop_Level3_scripts import icetop_globals

@icetray.traysegment
def ExtractWaveforms(tray, name, InputLaunches=icetop_globals.icetop_raw_data,
                     OutputWaveforms='IceTopVEMCalibratedWaveforms',
                     If=lambda f: True):
    OutputWaveforms = name + 'CalibratedWaveforms'
    OutputVEMWaveforms = name + 'VEMCalibratedWaveforms'

    tray.AddModule("I3WaveCalibrator", name+"_WaveCalibrator_IceTop",
                   Launches=InputLaunches, If=lambda fr: If(fr) and InputLaunches in fr,
                   Waveforms='ReextractedWaveforms',
                   WaveformRange="",
                   Errata="ReextractedErrata")

    tray.AddModule('I3WaveformSplitter', name + '_IceTopSplitter',
                   Input = 'ReextractedWaveforms',
                   HLC_ATWD = OutputWaveforms,
                   HLC_FADC = 'ReextractedHLCFADCWaveforms',
                   SLC = 'ReextractedSLCWaveforms',
                   Force = True, # ! put all maps in the frame, even if they are empty
                   PickUnsaturatedATWD = True,  # ! do not keep all ATWDs, but only the highest non-saturated gain one
                   If = lambda fr: If(fr) and InputLaunches in fr,
                   )

    from icecube.icetop_Level3_scripts.modules import I3VEMConverter
    tray.AddModule(I3VEMConverter, name + '_I3VEMConverter',
                   Waveforms = OutputWaveforms,
                   VEMWaveforms = OutputVEMWaveforms,
                   If = lambda fr: If(fr) and InputLaunches in fr,
                   )

