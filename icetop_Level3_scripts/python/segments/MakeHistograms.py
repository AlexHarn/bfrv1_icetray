
from icecube import icetray
from icecube.production_histograms.histograms.frame_histograms import PhysicsHistogram
from icecube.production_histograms import ProductionHistogramModule

from icecube.production_histograms.histograms.histogram import Histogram

from .. import histograms
from ..modules import ConditionalHistogramModule

@icetray.traysegment
def MakeHistograms(tray, name,
                   OutputFilename=None,
                   #condition=None,
                   condition = '"IT73AnalysisIceTopQualityCuts" in frame and all(frame["IT73AnalysisIceTopQualityCuts"].values()) and log10(frame["LaputopParams"].expected_signal(125.))>0.2',
                   isMC=False
                   ):
    
    # Histogram booking. 
    # We will limit the amount of histograms, since it takes a lot of time. 
    # Also, a module doing the cuts before the histogram is booked is written, but we will not use it for now. Could be used later.
    # No prescales are added eiter.
    # All this limits the sensitivity of these histograms to catch errors, but it's better than nothing.

    l3_histograms = []
    # We'll book some pulse info
    l3_histograms += histograms.IceTopRecoPulses('IceTopHLCSeedRTPulses')
    l3_histograms += histograms.IceTopRecoPulses('IceTopHLCSeedRTPulses_SnowCorrected')
    # This is sensitive to info of individual DOMs
    l3_histograms += [histograms.IndividualTankPulses("IceTopHLCSeedRTPulses")]
    
    # Reconstruction
    l3_histograms += histograms.TopRec('Laputop')

    l3_histograms += histograms.InIceRecoPulses('CoincPulses')
    
    l3_histograms += histograms.Millipede('MillipedeFitParams')
    l3_histograms += histograms.D4R('CoincMuonReco_SPEFit2_D4R_Params')
    if isMC:
        l3_histograms += histograms.MCPrimary('MCPrimary')                                                                                                                                                       

    '''
    l3_histograms += histograms.TopRec('LaputopSmall')
    l3_histograms += histograms.IceTopContainment('Laputop_FractionContainment')
    l3_histograms += histograms.IceTopRecoPulses('CleanedHLCTankPulses')
    l3_histograms += histograms.IceTopRecoPulses('IceTopLaputopSeededSelectedHLC')
    l3_histograms += histograms.IceTopRecoPulses('IceTopLaputopSeededSelectedSLC')
    l3_histograms += histograms.IceTopRecoPulses('OfflineIceTopHLCTankPulses')
    l3_histograms += histograms.IceTopRecoPulses('OfflineIceTopSLCTankPulses')
    l3_histograms += histograms.InIceRecoPulses('InIcePulses')
    l3_histograms += histograms.InIceRecoPulses('SRTCoincPulses')
    l3_histograms += histograms.InIceRecoPulses('CoincLaputopCleanedPulses')
    l3_histograms += histograms.StochasticReco('Stoch_Reco')
    l3_histograms += histograms.D4R('I3MuonEnergyLaputopParams')
    l3_histograms += histograms.InIceReco('CoincMuonReco_SPEFit2')
    l3_histograms += [histograms.IndividualTankPulses("CleanedHLCTankPulses")]
    l3_histograms += [histograms.IndividualTankPulses("OfflineIceTopHLCTankPulses")]
    '''
    '''
    l3_histograms_cuts = []
    l3_histograms_cuts += histograms.IceTopRecoPulses('IceTopHLCSeedRTPulses')
    l3_histograms_cuts += histograms.IceTopRecoPulses('IceTopHLCSeedRTPulses_SnowCorrected')
    l3_histograms_cuts += [histograms.IndividualTankPulses("IceTopHLCSeedRTPulses")]

    l3_histograms_cuts += histograms.TopRec('Laputop')

    l3_histograms_cuts += histograms.InIceRecoPulses('CoincPulses')

    l3_histograms_cuts += histograms.Millipede('MillipedeFitParams')
    l3_histograms_cuts += histograms.D4R('CoincMuonReco_SPEFit2_D4R_Params')
    if isMC:
        l3_histograms_cuts += histograms.MCPrimary('MCPrimary')                                                                                                                                             

    
    l3_histograms_cuts += histograms.TopRec('LaputopSmall')
    l3_histograms_cuts += histograms.IceTopContainment('Laputop_FractionContainment')
    l3_histograms_cuts += histograms.IceTopRecoPulses('CleanedHLCTankPulses')
    l3_histograms_cuts += histograms.IceTopRecoPulses('IceTopLaputopSeededSelectedHLC')
    l3_histograms_cuts += histograms.IceTopRecoPulses('IceTopLaputopSeededSelectedSLC')
    l3_histograms_cuts += histograms.IceTopRecoPulses('OfflineIceTopHLCTankPulses')
    l3_histograms_cuts += histograms.IceTopRecoPulses('OfflineIceTopSLCTankPulses')
    l3_histograms_cuts += histograms.InIceRecoPulses('InIcePulses')
    l3_histograms_cuts += histograms.InIceRecoPulses('SRTCoincPulses')
    l3_histograms_cuts += histograms.InIceRecoPulses('CoincLaputopCleanedPulses')
    l3_histograms_cuts += histograms.StochasticReco('Stoch_Reco')
    l3_histograms_cuts += histograms.D4R('I3MuonEnergyLaputopParams')
    l3_histograms_cuts += histograms.InIceReco('CoincMuonReco_SPEFit2')
    l3_histograms_cuts += histograms.MCPrimary('MCPrimary')
    l3_histograms_cuts += [histograms.IndividualTankPulses("CleanedHLCTankPulses")]
    l3_histograms_cuts += [histograms.IndividualTankPulses("OfflineIceTopHLCTankPulses")]
    
    
    l3_histograms.append(ConditionalHistogramModule('IT73',
                                                    l3_histograms_cuts,
                                                    condition))
    '''
    # Prescales make sure that only one in so many events/frames is booked.
    # default:
    histogram_prescales={"Geometry" : 1,
               "Calibration": 1,
               "DetectorStatus": 1,
               "DAQ": 1,
               "Physics": 1,
               }

    # We'll do one out of 10 data frames. 
    if not isMC:
        histogram_prescales={"Geometry" : 1,
                   "Calibration":1 ,
                   "DetectorStatus": 1,
                   "DAQ": 10,
                   "Physics": 10,
                   }
        
    tray.Add(ProductionHistogramModule, name+"_L3Histograms",
             OutputFilename = OutputFilename,
             Histograms = l3_histograms,
             Prescales=histogram_prescales
             
    )
    
