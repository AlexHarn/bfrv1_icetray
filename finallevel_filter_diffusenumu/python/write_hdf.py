#!/usr/bin/env python

from icecube import dataio, dataclasses, icetray, gulliver, lilliput, hdfwriter, cramer_rao, linefit, paraboloid, millipede, ophelia, simclasses, finiteReco
from icecube.phys_services import I3Calculator
from icecube.common_variables import *
from I3Tray import *

import sys

@icetray.traysegment
def write_hdf(tray, name, hdf_out):
    keep_keys = []
    for reco in ["BestTrack", "SplineMPE", "SplineMPEIC"]:
        keep_keys.append(reco)
        keep_keys.append(reco+"FitParams")
        keep_keys.append(reco+"Characteristics")
        keep_keys.append(reco+"CharacteristicsIC")
        for TW in ["A", "B", "C", "D", "E"]:
            keep_keys.append(reco+"DirectHits"+TW)
            keep_keys.append(reco+"DirectHitsIC"+TW)
    keep_keys.append("HitStatisticsValues")
    keep_keys.append("HitStatisticsValuesIC")
    keep_keys.append("HitMultiplicityValues")
    keep_keys.append("HitMultiplicityValuesIC")
    keep_keys.append("I3EventHeader")
    for count in ["RecomAttempts", "ReducedCount", "SplitCount"]:
        keep_keys.append("HiveSplitter"+count)

    for reco in ["LineFit", "SPEFitSingle", "SPEFit2"]:
        for n in ["Split1", "Split2"]:
            for split in ["Geo", "Time"]:
                keep_keys.append(reco+split+n)
                if reco=="LineFit":
                    keep_keys.append(reco+split+n+"Params")
                else:
                    keep_keys.append(reco+split+n+"FitParams")
    for reco in ["SPEFit2"]:
        for bayes in ["Bayesian", "BayesianIC"]:
            keep_keys.append(reco+bayes)
            keep_keys.append(reco+bayes+"FitParams")
    for reco in ["LineFit", "SPEFit2", "SPEFitSingle", "MPEFit"]:
        for suffix in ["_HV", "_TWHV", "IC"]:
            keep_keys.append(reco+suffix)
            if reco=="LineFit":
                keep_keys.append(reco+suffix+"Params")
            else:
                keep_keys.append(reco+suffix+"FitParams")
    for reco in ["MPEFit"]:
        for special in ["HighNoise", "Paraboloid"]:
            keep_keys.append(reco+special)
            keep_keys.append(reco+special+"FitParams")

    keep_keys.append("MCECenter")
    keep_keys.append("MCMostEnergeticInIce")
    for i in range(10):
        keep_keys.append("MCPrimary%d"%(i,))
    keep_keys.append("MuEXAngular4")

    for reco in ["SplineMPETruncatedEnergy_SPICEMie", "SplineMPEICTruncatedEnergySPICEMie"]:
        for method in ["_AllBINS", "_AllDOMS", "_BINS", "_ORIG"]:
            for ext in ["_MuEres", "_Muon", "_Neutrino", "_dEdX"]:
                keep_keys.append(reco+method+ext)
    for reco in ["SplineMPE", "SplineMPEIC"]:
        keep_keys.append(reco+"MuEXDifferential")
        keep_keys.append(reco+"MuEXDifferential_r")

    keep_keys.append("CorsikaWeightMap")
    keep_keys.append("I3MCWeightDict")
    keep_keys += ['SplineMPEICParaboloid', 'SplineMPEICParaboloidFitParams']

    keep_keys += ['LeptonInjectorProperties']
    keep_keys += ['Scattering', 'Absorption', 'AnisotropyScale',
                  'DOMEfficiency', 'HoleIceForward', 'IceWavePlusModes']

    ##post_L5 keys:
    for k in ['Passed_HESE','CausalQTot',
              'TrueMuoneEnergyAtDetectorEntry',
              'TrueMuoneEnergyAtDetectorLeave',
              'TrueMuoneEnergyAtInteraction',
              'TUM_dnn_energy',
              'PostDiffusePSRecoSplineMPEMaxSettingsFit',
              'PostDiffusePSRecoSplineMPEMaxSettingsFitFitParams',
              'PostDiffusePSRecoSplineMPEParaboloid',
              'PostDiffusePSRecoSplineMPEParaboloidFitParams',
              ]:
        keep_keys.append(k)

    tray.Add(hdfwriter.I3HDFWriter, "hd5s1",
        Output=hdf_out,
        Keys=keep_keys,
        SubEventStreams=["Final"])


