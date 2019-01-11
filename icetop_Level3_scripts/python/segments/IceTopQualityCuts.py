#!/usr/bin/python
from icecube import icetray

@icetray.traysegment
def IceTopQualityCuts(tray, name,
                      detector='IC79',
                      pulses='CleanedHLCTankPulses',
                      isMC=False,
                      reco_track='Laputop',
                      removeOrNot=False):
    "Segment to collect all IceTop quality cuts. Last module in this segment: "
    "do the actual cut or put booleans in a frameobject to say whether they "
    "survived or not."

    from icecube import icetop_Level3_scripts, toprec
    try:
        # recclasses is needed to read LaputopParams in trunk
        from icecube import recclasses
        LaPar = recclasses.LaputopParameter
    except ImportError:
        pass

    # Outputs the station density. To exclude strange event topologies (~
    # inclined?). Main output bool = StationDensity_passed
    tray.AddModule(icetop_Level3_scripts.modules.StationDensity, name+'_StationDensity',
                   InputITpulses=pulses,
                   InputCOG=reco_track)

    # Mainly check whether largest signal is not on edge. Main output bool = MaxStationTotalInEdge_passed
    # Not written to frame if snow corrected pulses do not exist (or are all zero)
    # Thus not for STA3-STA4 events which do not pass the small filter. 
    tray.AddModule(icetop_Level3_scripts.modules.MaxSignalCuts, name+'_MaxSignalCuts',
                   Pulses=[pulses],
                   )

    # Check whether the particles are contained.
    tray.AddModule(icetop_Level3_scripts.modules.CheckContainment,
                   name+'_CheckContainment', Particles=[reco_track], Detector=detector)
    recoFracContained = reco_track+"_FractionContainment"

    # Actual module which actually cuts out the events which have one of the above set booleans to False.
    # OR do not remove the events and keep them in a map of the key names and bool values.
    # The list of keys is best to be logical.(ie, first filter, then pulses cuts, then reco cuts, as above.)
    # We will remove the keys in the frame.

    def get_beta(frame_obj):
        'get beta in a format-agnostic way'
        if hasattr(frame_obj, 'beta'):
            # old Params
            return frame_obj.beta
        # new params
        return frame_obj.value(LaPar.Beta)

    tray.AddModule(icetop_Level3_scripts.modules.MakeQualityCuts, name+'_DoIceTopCuts',
                   removeEvents=removeOrNot,
                   CutOrder=["IceTop_StandardFilter",
                             "StationDensity",
                             "IceTopMaxSignal",
                             "IceTopNeighbourMaxSignal",
                             "IceTopMaxSignalInEdge",
                             "IceTop_reco_succeeded",
                             "BetaCutPassed",
                             recoFracContained],
                   CutsToEvaluate={"IceTop_StandardFilter": (lambda fr: "IceTop_StandardFilter" in fr and fr["IceTop_StandardFilter"].value),
                                   "StationDensity": (lambda fr: "StationDensity" in fr and fr["StationDensity"].value > 0.2),
                                   "IceTopMaxSignal": (lambda fr: "IceTopMaxSignal" in fr and fr["IceTopMaxSignal"].value >= 6.0),
                                   "IceTopNeighbourMaxSignal": (lambda fr: "IceTopNeighbourMaxSignal" in fr and fr["IceTopNeighbourMaxSignal"].value >= 4.0),
                                   "IceTopMaxSignalInEdge": (lambda fr: "IceTopMaxSignalInEdge" in fr and fr["IceTopMaxSignalInEdge"].value == False),
                                   "IceTop_reco_succeeded": (lambda fr: reco_track in fr and fr[reco_track].fit_status_string == "OK"),
                                   "BetaCutPassed": (lambda fr: reco_track+"Params" in fr and get_beta(fr[reco_track+"Params"]) > 1.4 and get_beta(fr[reco_track+"Params"]) < 9.5),
                                   recoFracContained: (lambda fr: recoFracContained in fr and fr[recoFracContained].value < 0.96)},
                   CutsNames={"IceTop_StandardFilter": "IceTop_StandardFilter",
                              "StationDensity": "StationDensity_passed",
                              "IceTopMaxSignal": "IceTopMaxSignalAbove6",
                              "IceTopNeighbourMaxSignal": "IceTopNeighbourMaxSignalAbove4",
                              "IceTopMaxSignalInEdge": "IceTopMaxSignalInside",
                              "IceTop_reco_succeeded": "IceTop_reco_succeeded",
                              "BetaCutPassed": "BetaCutPassed",
                              recoFracContained: recoFracContained}
                   )
