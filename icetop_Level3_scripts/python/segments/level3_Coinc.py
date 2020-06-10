#!/usr/bin/env python

from icecube import icetray
@icetray.traysegment
def level3_Coinc(tray,name,
                 Detector,
                 IceTopTrack='Laputop',
                 InIcePulses='InIcePulses',
                 IceTopPulses='CleanedHLCTankPulses',
                 isMC=False,
                 pass2a=False,
                 do_select=False,
                 domeff=0.99, #Beware!
                 spline_dir="/data/sim/sim-new/downloads/spline-tables/"
                 ):

    from icecube import dataclasses, icetop_Level3_scripts
    from icecube.icetop_Level3_scripts import icetop_globals
    import math
    

    # This module tries to select the inice pulses which are in a coincident time window with the IceTop pulses.
    # Normally, it only looks to the trigger and then selects pulses in a time window around this.
    # However, if there are more multiple IT triggers, it uses the IT pulses in this P frame to find the correct IT trigger for this P frame.
    # This module only runs if the coincident filter is passed and if there are good IT pulses in this P frame.
    # Otherwise we would put IC pulses in P frames which have for example only one cleaned, thus nbad, pulse.                                                                                             
    # We could probably make this module even smarter...                                                                 
    from icecube import coinc_twc
    tray.AddModule("I3CoincTWC<I3RecoPulseSeries>",name+"_CoincTWC",
                   CheckSingleSMTs = True,          # default
                   cleanWindowMaxLength = 6500.,    # default
                   cleanWindowMinus = 300.,         # default
                   cleanWindowPlus = 400.,          # default
                   IceTopVEMPulsesName = IceTopPulses,  # just be safe to select IT SMT, if random coinc, will NOT pass the whole chain of cuts!
                   InputResponse = InIcePulses,
                   KeepLookingForCausalTrigger = False,   # default
                   KeepMultipleCausalTriggers = True,   # default
                   OutputResponse =icetop_globals.inice_coinc_pulses,
                   TriggerHierarchyName = 'I3TriggerHierarchy',
                   UseSMT3 = False,
                   UseSMT8 = True,
                   MaxTimeDiff = 1500.,         # should be sharper? NO, just need to find A IT trigger, good reco + InIce size will clean out remaining random coinc!                                      
                   WindowMax = 7000.,
                   WindowMin = 2000.,          # 2000 should be suited for inclined showers up to 65 degrees, which land outside IceTop and hit the edge of IceCube.                                        
                   Strategy = 'method2',       # 99.6% efficient!! vs 98% of method1
                   Stream = icetray.I3Frame.Physics,
                   If= lambda fr: (("IceTop_StandardFilter" in fr and fr["IceTop_StandardFilter"].value) or ("IceTop_InFillFilter" in fr and fr["IceTop_InFillFilter"].value)) and IceTopPulses in fr and len(dataclasses.I3RecoPulseSeriesMap.from_frame(fr,IceTopPulses).keys())>0
                   )

    # This segment actually does some preparations for the reconstruction of the energy loss of the muon bundle with millipede. 
    # It cleans the inice pulses according to the inice track, and outputs NCh_+CleanCoincPulses, which is a boolean to see whether the cleaned pulses contain at least 8 DOMs.                             
    # It furthermore handles time windows and calibration errata.
    tray.AddSegment(icetop_Level3_scripts.segments.SelectCleanInIcePulses,name+'_selectClean',
                    CoincPulses=icetop_globals.inice_coinc_pulses,
                    IceTopTrack=IceTopTrack,
                    Detector=Detector,
                    CleanCoincPulses=icetop_globals.inice_clean_coinc_pulses,
                    Pass2a=pass2a,
                    If=lambda fr: icetop_globals.inice_coinc_pulses in fr
                    )

    # Segment where a reconstruction of the energy loss profile using millipede is performed, together with certain modules that Tom used. We could replace those or add some reconstruction modules here.
    tray.AddSegment(icetop_Level3_scripts.segments.EnergylossReco,name+'_ElossReco',
                    InIcePulses = icetop_globals.inice_clean_coinc_pulses,
                    dom_eff = domeff,
                    splinedir = spline_dir,
                    IceTopTrack=IceTopTrack,
                    If=lambda fr: "NCh_"+icetop_globals.inice_clean_coinc_pulses in fr and fr['NCh_' + icetop_globals.inice_clean_coinc_pulses].value
                    )

    tray.AddSegment(icetop_Level3_scripts.segments.muonReconstructions,name+'_muonReco', 
                    If= lambda fr: icetop_globals.inice_coinc_pulses in fr)

    # Perform the quality cuts that Tom did in his analysis.
    # Collect in IT73AnalysisInIceQualityCuts
    tray.AddModule(icetop_Level3_scripts.modules.MakeQualityCuts,name+'_DoInIceCuts',
                   RemoveEvents=do_select,
                   CutOrder=["NCh_"+icetop_globals.inice_clean_coinc_pulses,"MilliRlogl","MilliQtot","MilliNCasc","StochReco"],
                   CutsToEvaluate={"NCh_"+icetop_globals.inice_clean_coinc_pulses:(lambda fr: fr["NCh_"+icetop_globals.inice_clean_coinc_pulses].value),"MilliRlogl":(lambda fr: "MillipedeFitParams" in fr and math.log10(fr["MillipedeFitParams"].rlogl)<2), "MilliQtot": (lambda fr: "MillipedeFitParams" in fr and math.log10(fr["MillipedeFitParams"].predicted_qtotal/fr["MillipedeFitParams"].qtotal)>-0.03), "MilliNCasc": (lambda fr: "Millipede_dEdX" in fr and len([part for part in fr["Millipede_dEdX"] if part.energy > 0]) >= 3),"StochReco": (lambda fr: "Stoch_Reco" in fr and fr["Stoch_Reco"].status == dataclasses.I3Particle.OK)},
                   CutsNames={"NCh_"+icetop_globals.inice_clean_coinc_pulses:"NCh_"+icetop_globals.inice_clean_coinc_pulses+"Above7","MilliRlogl":"MilliRloglBelow2","MilliQtot":"MilliQtotRatio","MilliNCasc":"MilliNCascAbove2","StochReco":"StochRecoSucceeded"},
                   CollectBools="IT73AnalysisInIceQualityCuts"
                   )  
