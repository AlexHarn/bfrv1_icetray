'''
This makes histograms of SPEFit2
'''
import math
from math import cos, log10
from I3Tray import I3Units
from icecube.production_histograms.histograms.frame_histograms import PhysicsHistogram
from icecube.production_histograms.histograms.particle_histogram_generator import generate_particle_histograms

_l2_particle_keys = ['CascadeLlh_TopoSplit_IC1',
                     'SPEFit2_DC',
                     'CascadeLlhVertexFitSplit1_L2',
                     'CascadeLlhVertexFit_L2',
                     'OnlineL2_SplitTime2_BayesianFit',
                     'OnlineL2_SplineMPE_MuEx',
                     'OnlineL2_SplitTime1_Linefit',
                     'OnlineL2_SplineMPE_TruncatedEnergy_AllDOMS_Muon',
                     'OnlineL2_SplitGeo2_Linefit',
                     'OnlineL2_SplitGeo1_SPE2itFit',
                     'OnlineL2_SplineMPE_TruncatedEnergy_AllDOMS_Neutrino',
                     'OnlineL2_SplitTime1_BayesianFit',
                     'SPEFitSingle_DC',
                     'CascadeToISplit2_L2',
                     'OnlineL2_SplineMPE_TruncatedEnergy_ORIG_Neutrino',
                     'OnlineL2_SplitGeo1_BayesianFit',
                     'SPEFitSingle',
                     'OnlineL2_SplineMPE_TruncatedEnergy_BINS_Muon',
                     'MPEFitMuEX',
                     'SPEFit2MuEX_FSS',
                     'CascadeLast_L2',
                     'OnlineL2_SplineMPE_TruncatedEnergy_AllBINS_Muon',
                     'CascadeLineFitSplit1_L2',
                     'CascadeToISplit1_L2',
                     'OnlineL2_SPE2itFit',
                     'CascadeLast_IC_Singles_L2',
                     'MM_IC_LineFitI',
                     'CascadeImprovedLineFit_L2',
                     'OnlineL2_SplitTime2_Linefit',
                     'ToI_DC',
                     'OnlineL2_SplineMPE_TruncatedEnergy_AllBINS_Neutrino',
                     'CascadeLlh_TopoSplit_IC0',
                     'OnlineL2_SplitGeo2_BayesianFit',
                     'OnlineL2_MPEFit',
                     'OnlineL2_SplineMPE_TruncatedEnergy_BINS_Neutrino',
                     'OnlineL2_SplitTime1_SPE2itFit',
                     'CascadeLineFitSplit2_L2',
                     'OnlineL2_SplitTime2_SPE2itFit',
                     'OnlineL2_SplineMPE_TruncatedEnergy_ORIG_Muon',
                     'FiniteRecoFit',
                     'PoleMuonLinefit',
                     'MPEFit',
                     'OnlineL2_SplineMPE',
                     'DipoleFit_DC',
                     'SPEFit2',
                     'CascadeLast_DC',
                     'MM_DC_LineFitI_MM_DC_Pulses_1P_C05',
                     'LineFit',
                     'OnlineL2_SplineMPE_TruncatedEnergy_DOMS_Neutrino',
                     'OnlineL2_SplitGeo1_Linefit',
                     'CascadeLineFit_L2',
                     'CascadeLast_TopoSplit_IC1',
                     'CascadeLast_TopoSplit_IC0',
                     'CascadeLlhVertexFitSplit2_L2',
                     'OnlineL2_SplitGeo2_SPE2itFit',
                     'PoleMuonLlhFit',
                     'OnlineL2_SplineMPE_MuE',
                     'OnlineL2_BestFit_MuEx',
                     'OnlineL2_BestFit',
                     'CascadeLlhVertexFit_IC_Singles_L2',
                     'LineFit_DC',
                     'CascadeDipoleFit_L2',
                     'OnlineL2_SplineMPE_TruncatedEnergy_DOMS_Muon',
                     'OnlineL2_BayesianFit']


l2_particle_histograms = list()
for frame_key in _l2_particle_keys:
    l2_particle_histograms.extend(generate_particle_histograms(frame_key))
