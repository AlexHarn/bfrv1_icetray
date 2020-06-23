#!/usr/bin/env python

from I3Tray import *
import sys, os, glob
from icecube import icetray, dataio, dataclasses
from icecube import common_variables, spline_reco
from .functions import removeBugEvents, selectIceCubeOnly
from .reconstruct import BasicRecos, BayesianRecos, EnergyRecos

@icetray.traysegment
def IC12L4(tray, name, table_paths, is_numu, 
           gcdfile=None, infiles=None, 
           pulsemap_name="TWSRTHVInIcePulsesIC"):

    if gcdfile is not None and infiles is not None:
        files = [gcdfile]
        if isinstance(infiles,str):
            files.append(infiles)
        else:
            for f in infiles:files.append(f)

        print("Infiles: ", files)
        tray.Add(dataio.I3Reader, FilenameList=files)

    # remove all downgoing events relatively far from threshold
    def CutZenith(frame):
        if frame.Has("SplineMPE"):
            if frame["SplineMPE"].fit_status==dataclasses.I3Particle.OK:
                if frame["SplineMPE"].dir.zenith > 80.*icetray.I3Units.degree:
                    return True
        return False
    tray.Add(CutZenith)

    # remove events that have a muon with an energy smaller than rest mass
    if is_numu:
        tray.Add(removeBugEvents)

    # cut away low nch events
    def CutNCh(frame):
        pulses=dataclasses.I3RecoPulseSeriesMap.from_frame(frame, pulsemap_name)
        if len(pulses)<=6:
            return False
        else:
            return True
    tray.Add(CutNCh)

    # IceCube only spline-reco
    EnEstis = ["SplineMPETruncatedEnergy_SPICEMie_AllDOMS_Muon",
               "SplineMPETruncatedEnergy_SPICEMie_DOMS_Muon",
               "SplineMPETruncatedEnergy_SPICEMie_AllBINS_Muon",
               "SplineMPETruncatedEnergy_SPICEMie_BINS_Muon",
               "SplineMPETruncatedEnergy_SPICEMie_ORIG_Muon"]

    tray.AddSegment(spline_reco.SplineMPE, "SplineMPEIC",
        fitname="SplineMPEIC",
        configuration="max",
        PulsesName = pulsemap_name,
        TrackSeedList = ["SplineMPE"],
        BareMuTimingSpline = table_paths["infmuonprobsplinepath"],
        BareMuAmplitudeSpline = table_paths["infmuonampsplinepath"],
        StochTimingSpline = table_paths["stochprobsplinepath"],
        StochAmplitudeSpline = table_paths["stochampsplinepath"],
        EnergyEstimators = EnEstis)

    #DirectHitsDefs=common_variables.direct_hits.default_definitions
    DirectHitsDefs=common_variables.direct_hits.get_default_definitions()
    DirectHitsDefs.append(common_variables.direct_hits.I3DirectHitsDefinition("E",-15.,250.))

    for track in ["SplineMPEIC"]:
        tray.AddSegment(common_variables.direct_hits.I3DirectHitsCalculatorSegment,
            DirectHitsDefinitionSeries       = DirectHitsDefs,
            PulseSeriesMapName               = pulsemap_name,
            ParticleName                     = track,
            OutputI3DirectHitsValuesBaseName = track+'DirectHitsIC',
            BookIt                           = True)

        tray.AddSegment(common_variables.track_characteristics.I3TrackCharacteristicsCalculatorSegment,
            PulseSeriesMapName                     = pulsemap_name,
            ParticleName                           = track,
            OutputI3TrackCharacteristicsValuesName = track+'CharacteristicsIC',
            TrackCylinderRadius                    = 150*icetray.I3Units.m,
            BookIt                                 = True)

    # apply precuts for bdt
    def PrecutFilter(frame):
        coszen = frame["SplineMPEIC"].dir.zenith
        nch = frame["HitMultiplicityValuesIC"].n_hit_doms
        ldir = frame["SplineMPEICDirectHitsICC"].dir_track_length
        ndir = frame["SplineMPEICDirectHitsICC"].n_dir_doms
        if coszen<=85.*I3Units.degree or nch<=12 or ldir<=200.*I3Units.meter or ndir<=6:
            return False
    tray.Add(PrecutFilter)

    tray.Add("I3OrphanQDropper")

    # run all reconstructions for bdt
    tray.Add(BasicRecos)

    tray.Add(BayesianRecos)

    tray.Add(EnergyRecos,
        PhotonicsTableDir = table_paths["photonicsdir"],
        PhotonicsDriverDir = table_paths["photonicsdriverdir"],
        PhotonicsDriverFile = table_paths["photonicsdriverfile"])

