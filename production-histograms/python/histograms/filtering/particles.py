'''
This makes histograms of different track reconstructions. Sanity Checker
Zenith, azimuth, Energy, and one parameter per reco
'''
from icecube.production_histograms.histograms.frame_histograms import PhysicsHistogram
from icecube.production_histograms.histograms.particle_histogram_generator import generate_particle_histograms

_l1_particle_keys = ['PoleMuonLinefit',
                     'PoleMuonLlhFit',
                     'OnlineL2_SplitGeo1_SPE2itFit',
                     'OnlineL2_SplitTime1_BayesianFit',
                     'OnlineL2_SplitTime2_BayesianFit',
                     'OnlineL2_SplineMPE_MuEx',
                     'OnlineL2_SplitTime1_Linefit',
                     'OnlineL2_SplineMPE_TruncatedEnergy_AllDOMS_Muon',
                     'OnlineL2_SplitGeo2_Linefit',
                     'OnlineL2_SplineMPE_TruncatedEnergy_AllDOMS_Neutrino',
                     'OnlineL2_SplitTime2_SPE2itFit',
                     'OnlineL2_SplineMPE_TruncatedEnergy_ORIG_Neutrino',
                     'OnlineL2_SplineMPE_TruncatedEnergy_BINS_Muon',
                     'OnlineL2_SplitGeo1_BayesianFit',
                     'OnlineL2_SPE2itFit',
                     'OnlineL2_SplitTime2_Linefit',
                     'OnlineL2_SplitGeo2_BayesianFit',
                     'OnlineL2_MPEFit',
                     'OnlineL2_SplineMPE_TruncatedEnergy_BINS_Neutrino',
                     'OnlineL2_SplitTime1_SPE2itFit',
                     'OnlineL2_SplineMPE_TruncatedEnergy_ORIG_Muon',
                     'OnlineL2_SplineMPE',
                     'OnlineL2_SplineMPE_TruncatedEnergy_AllBINS_Neutrino',
                     'OnlineL2_SplineMPE_TruncatedEnergy_DOMS_Neutrino',
                     'OnlineL2_SplitGeo1_Linefit',
                     'OnlineL2_SplitGeo2_SPE2itFit',
                     'OnlineL2_SplineMPE_MuE',
                     'OnlineL2_BestFit_MuEx',
                     'OnlineL2_BestFit',
                     'OnlineL2_BayesianFit',
                     'OnlineL2_SplineMPE_TruncatedEnergy_DOMS_Muon',
                     'OnlineL2_SplineMPE_TruncatedEnergy_AllBINS_Muon']

l1_particle_histograms = list()
for frame_key in _l1_particle_keys:
    l1_particle_histograms.extend(generate_particle_histograms(frame_key))
        
