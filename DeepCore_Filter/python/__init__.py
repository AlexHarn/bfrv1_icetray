from icecube import icetray, DomTools, STTools
from icecube.icetray import I3Units, I3Frame
from . import DOMS, DOMList

from icecube.STTools.seededRT.configuration_services import I3DOMLinkSeededRTConfigurationService
icetray.load('libDeepCore_Filter', False)


@icetray.traysegment
def RunFilter( tray, name, 
               pulses = "InIcePulses",
               OutputName = "DCVeto", 
               DetectorConfig = "IC86EDC",
               If = lambda f: True
               ):

    ### Perform SeededRT using HLC instead of HLCCore ###
    SRTParams = {"allowSelfCoincidence"         : False,
                 "useDustlayerCorrection"       : True,
                 "dustlayerUpperZBoundary"      : 0*I3Units.m,
                 "dustlayerLowerZBoundary"      : -150*I3Units.m,
                 "ic_ic_RTRadius"               : 150.*I3Units.m,
                 "ic_ic_RTTime"                 : 1000.*I3Units.ns,
                 "dc_dc_RTRadius"               : 75.*I3Units.m,
                 "dc_dc_RTTime"                 : 500.*I3Units.ns,
                 "treat_string_36_as_deepcore"  : True}

    stConfigService = I3DOMLinkSeededRTConfigurationService(**SRTParams)

    tray.AddModule("I3SeededRTCleaning_RecoPulse_Module", name+"seededRTcleaning",
                   STConfigService         = stConfigService,
                   InputHitSeriesMapName   = pulses, 
                   OutputHitSeriesMapName  = pulses+"SRT",
                   SeedProcedure           = "AllHLCHits",
                   MaxNIterations          = -1,
                   Streams                 = [I3Frame.Physics],
                   If = lambda f: If(f) and deep_core_trigger_check(f) and not f.Has(pulses+"SRT"),
                   )

    dlist = DOMS.DOMS("IC86")
    
    tray.AddModule("I3OMSelection<I3RecoPulseSeries>",name+'selectICVetoDOMS',
                   OmittedKeys = dlist.DeepCoreVetoDOMs,
                   OutputOMSelection = 'GoodSelection_ICVeto',
                   InputResponse = pulses+'SRT',
                   OutputResponse = 'SRTPulseICVeto',
                   SelectInverse = True,
                   If = lambda f: If(f) and deep_core_trigger_check(f) and not f.Has("SRTPulseICVeto"),
                   )

    tray.AddModule("I3OMSelection<I3RecoPulseSeries>",name+'selectDCFidDOMs',
                   OmittedKeys= dlist.DeepCoreFiducialDOMs,
                   SelectInverse = True,
                   InputResponse = pulses+'SRT',
                   OutputResponse = 'SRTPulseDCFid',
                   OutputOMSelection = 'GoodSelection_DCFid',
                   If = lambda f: If(f) and deep_core_trigger_check(f) and not f.Has("SRTPulseDCFid"),
                   )

    tray.AddModule("I3DeepCoreVeto<I3RecoPulse>",name+'deepcore_filter',
                   ChargeWeightCoG = False,
                   DecisionName = OutputName,
                   FirstHitOnly = True,
                   InputFiducialHitSeries = 'SRTPulseDCFid',
                   InputVetoHitSeries = 'SRTPulseICVeto',
                   If = lambda f: If(f) and deep_core_trigger_check(f)
                   )



@icetray.traysegment
def DeepCoreTimeVeto( tray, name, 
                      pulses = "InIcePulses",
                      OutputName = "DCTimeVeto", 
                      DetectorConfig = "IC86EDC",
                      If = lambda f: True
                      ):

    ### Perform SeededRT using HLC instead of HLCCore ###
    SRTParams = {"allowSelfCoincidence"         : False,
                 "useDustlayerCorrection"       : True,
                 "dustlayerUpperZBoundary"      : 0*I3Units.m,
                 "dustlayerLowerZBoundary"      : -150*I3Units.m,
                 "ic_ic_RTRadius"               : 150.*I3Units.m,
                 "ic_ic_RTTime"                 : 1000.*I3Units.ns,
                 "dc_dc_RTRadius"               : 75.*I3Units.m,
                 "dc_dc_RTTime"                 : 500.*I3Units.ns,
                 "treat_string_36_as_deepcore"  : True}

    stConfigService = I3DOMLinkSeededRTConfigurationService(**SRTParams)

    tray.AddModule("I3SeededRTCleaning_RecoPulse_Module", name+"seededRTcleaning",
                   STConfigService         = stConfigService,
                   InputHitSeriesMapName   = pulses, 
                   OutputHitSeriesMapName  = pulses+"SRT",
                   SeedProcedure           = "AllHLCHits",
                   MaxNIterations          = -1,
                   Streams                 = [I3Frame.Physics],
                   If = lambda f: If(f) and deep_core_trigger_check(f) and not f.Has(pulses+"SRT"),
                   )

    dlist = DOMS.DOMS("IC86")
    
    tray.AddModule("I3OMSelection<I3RecoPulseSeries>",name+'selectICVetoDOMS',
                   OmittedKeys = dlist.DeepCoreVetoDOMs,
                   OutputOMSelection = 'GoodSelection_ICVeto',
                   InputResponse = pulses+'SRT',
                   OutputResponse = 'SRTPulseICVeto',
                   SelectInverse = True,
                   If = lambda f: If(f) and deep_core_trigger_check(f) and not f.Has("SRTPulseICVeto"),
                   )

    tray.AddModule("I3OMSelection<I3RecoPulseSeries>",name+'selectDCFidDOMs',
                   OmittedKeys= dlist.DeepCoreFiducialDOMs,
                   SelectInverse = True,
                   InputResponse = pulses+'SRT',
                   OutputResponse = 'SRTPulseDCFid',
                   OutputOMSelection = 'GoodSelection_DCFid',
                   If = lambda f: If(f) and deep_core_trigger_check(f) and not f.Has("SRTPulseDCFid"),
                   )

    tray.AddModule("I3DeepCoreTimeVeto<I3RecoPulse>",name+'deepcore_filter',
                   ChargeWeightCoG = False,
                   DecisionName = OutputName,
                   FirstHitOnly = True,
                   InputFiducialHitSeries = 'SRTPulseDCFid',
                   InputVetoHitSeries = 'SRTPulseICVeto',
                   If = lambda f: If(f) and deep_core_trigger_check(f)
                   )

def deep_core_trigger_check(frame):
    if frame.Stop == I3Frame.Physics:
        if not frame.Has("I3TriggerHierarchy"): return 0
        deep_core_bool = False
        for trig in frame["I3TriggerHierarchy"]:
            if trig.key.config_id in [1010, 1011]:
                deep_core_bool = True
        return deep_core_bool
    else:
        return 0


del icetray
