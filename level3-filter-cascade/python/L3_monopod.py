from I3Tray import *
from icecube import icetray
from icecube.icetray import traysegment
from icecube import icetray, dataclasses, dataio

@icetray.traysegment
def L3_Monopod(tray, name,year,Pulses='OfflinePulses',
    AmplitudeTable="/data/sim/sim-new/downloads/spline-tables/ems_mie_z20_a10.abs.fits",
    TimingTable="/data/sim/sim-new/downloads/spline-tables/ems_mie_z20_a10.prob.fits"):

    from icecube import wavedeform

    # fix waveform range: 
    from icecube import WaveCalibrator, DomTools
    kwargs = dict(Launches='InIceRawData', Waveforms='CalibratedWaveforms', Errata='CalibrationErrata')
        #tray.AddSegment(WaveCalibrator.DOMSimulatorCalibrator, name+'wavecal', If=lambda frame: frame.Has('InIceRawData'), **kwargs)
    tray.AddModule('I3WaveCalibrator',name+'wavecal', FADCSaturationMargin=1,If=lambda frame: not frame.Has('CalibratedWaveformRange') and frame.Has('InIceRawData'), **kwargs)

    tray.AddModule('I3WaveformTimeRangeCalculator', name+'Range', If=lambda frame: not frame.Has('CalibratedWaveformRange'))

    tray.AddModule('Delete', 'Delete', Keys=['CalibratedWaveforms'])
  
    # continue with normal operation
    tray.AddModule(wavedeform.AddMissingTimeWindow, 'pulserange', Pulses=Pulses,If=lambda frame: not frame.Has(Pulses+'TimeRange'))    


    from icecube.millipede import MonopodFit, HighEnergyExclusions
    from icecube import photonics_service, millipede
    
    #exclusions = tray.AddSegment(HighEnergyExclusions, Pulses='SRTOfflinePulses',ExcludeDeepCore=False,BadDomsList='BadDomsListSLC')
    exclusions = tray.AddSegment(HighEnergyExclusions, Pulses='SRTOfflinePulses',ExcludeDeepCore=False,BadDomsList='BadDomsList')
    
    table_base = '/data/sim/sim-new/spline-tables/ems_mie_z20_a10.%s.fits'
    photonics_service = photonics_service.I3PhotoSplineService(AmplitudeTable, TimingTable, 0.)
    
    millipede_config = dict(Pulses=Pulses, CascadePhotonicsService=photonics_service,
        PartialExclusion=False,
        Parametrization='HalfSphere')
        
    #tray.AddSegment(MonopodFit, 'L3_MonopodFit4', Seed='CscdL3_Credo_SpiceMie',
    #    PhotonsPerBin=5, Iterations=4,DOMEfficiency=0.99,BinSigma=2,MintimeWidth=15,BadDOMs=exclusions,**millipede_config)

    #tray.AddSegment(MonopodFit, 'L3_MonopodFit4_CascadeSeed', Seed='CascadeSeed',
    #    PhotonsPerBin=5, Iterations=4,DOMEfficiency=0.99,BinSigma=2,MintimeWidth=15,BadDOMs=exclusions,**millipede_config)

    if year == "2011":
        AmpSeed= 'CascadeLlhVertexFit'
    else:
        AmpSeed= 'CascadeLlhVertexFit_L2'

    tray.AddSegment(MonopodFit, 'RedoMonopodAmpFit', Seed=AmpSeed,
        PhotonsPerBin=-1, **millipede_config)

    tray.AddSegment(MonopodFit, 'L3_MonopodFit4_AmptFit',Seed='RedoMonopodAmpFit',
        PhotonsPerBin=5, Iterations=4,DOMEfficiency=0.99,BinSigma=2,MintimeWidth=15,BadDOMs=exclusions,**millipede_config)

