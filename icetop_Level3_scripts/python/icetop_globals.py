# IceTop constants
SnowFactors = {'IC79': 2.1, 'IC86.2011':2.25, 'IC86.2012':2.25, 'IC86.2013':2.3, 'IC86.2014':2.3, 'IC86.2015':2.3, 'IC86.2016':2.3}

background_rate = {'IC79': 1677., 'IC86.2011': 1518., 'IC86.2012': 1492., 'IC86.2013': 1424.}

## L2 to L3 conversion of names:
names = {
    'IC79': { 'sub_event_stream':'top_hlc_clusters',
              'FilterMask':'FilterMask',
              'HLCVEMPulses':'IceTopHLCVEMPulses',
              'HLCPulses':'IceTopHLCTankPulses',
              'CleanHLCPulses':'CleanedHLCTankPulses',
              'SLCVEMPulses':'IceTopSLCVEMPulses',
              'SLCPulses':'IceTopSLCTankPulses',
              'TankPulseMergerExcluded':'ExcludedHLCStations',
              'ClusterCleaningExcluded':'ClusterCleaningExcludedStations',
              'InIcePulses':'OfflinePulses',
              'CalibrationErrata':'CalibrationErrata', 
              'I3TriggerHierarchy':'I3TriggerHierarchy'
              },  
    
    'IC86.2011': { 'sub_event_stream':'ice_top',
                   'FilterMask':'FilterMask',
                   'HLCVEMPulses':'OfflineIceTopHLCVEMPulses',
                   'HLCPulses':'OfflineIceTopHLCTankPulses',
                   'CleanHLCPulses':'CleanedHLCTankPulses',
                   'SLCVEMPulses':'OfflineIceTopSLCVEMPulses',
                   'SLCPulses':'OfflineIceTopSLCTankPulses',
                   'TankPulseMergerExcluded':'TankPulseMergerExcludedStations',
                   'ClusterCleaningExcluded':'ClusterCleaningExcludedStations',
                   'InIcePulses':'OfflinePulses',
                   'CalibrationErrata':'OfflineInIceCalibrationErrata',
                   'I3TriggerHierarchy':'I3TriggerHierarchy'},
    'IC86.2012': { 'sub_event_stream':'ice_top',
                   'FilterMask':'QFilterMask',
                   'HLCVEMPulses':'OfflineIceTopHLCVEMPulses',
                   'HLCPulses':'OfflineIceTopHLCTankPulses',
                   'CleanHLCPulses':'CleanedHLCTankPulses',
                   'SLCVEMPulses':'OfflineIceTopSLCVEMPulses',
                   'SLCPulses':'OfflineIceTopSLCTankPulses',
                   'TankPulseMergerExcluded':'TankPulseMergerExcludedTanks',
                   'ClusterCleaningExcluded':'ClusterCleaningExcludedTanks',
                   'InIcePulses':'InIcePulses',
                   'CalibrationErrata':'CalibrationErrata',
                   'I3TriggerHierarchy':'DSTTriggers'},
    'IC86.2013': { 'sub_event_stream':'IceTopSplit',
                   'FilterMask':'QFilterMask',
                   'HLCVEMPulses':'OfflineIceTopHLCVEMPulses',
                   'HLCPulses':'OfflineIceTopHLCTankPulses',
                   'CleanHLCPulses':'CleanedHLCTankPulses',
                   'SLCVEMPulses':'OfflineIceTopSLCVEMPulses',
                   'SLCPulses':'OfflineIceTopSLCTankPulses',
                   'TankPulseMergerExcluded':'TankPulseMergerExcludedTanks',
                   'ClusterCleaningExcluded':'ClusterCleaningExcludedTanks',
                   'InIcePulses':'InIcePulses',
                   'CalibrationErrata':'CalibrationErrata',
                   'I3TriggerHierarchy':'DSTTriggers'},
    'IC86.2014': { 'sub_event_stream':'IceTopSplit',
                   'FilterMask':'QFilterMask',
                   'HLCVEMPulses':'OfflineIceTopHLCVEMPulses',
                   'HLCPulses':'OfflineIceTopHLCTankPulses',
                   'CleanHLCPulses':'CleanedHLCTankPulses',
                   'SLCVEMPulses':'OfflineIceTopSLCVEMPulses',
                   'SLCPulses':'OfflineIceTopSLCTankPulses',
                   'TankPulseMergerExcluded':'TankPulseMergerExcludedTanks',
                   'ClusterCleaningExcluded':'ClusterCleaningExcludedTanks',
                   'InIcePulses':'InIcePulses',
                   'CalibrationErrata':'CalibrationErrata',
                   'I3TriggerHierarchy':'DSTTriggers'},
    'IC86.2015': { 'sub_event_stream':'IceTopSplit',
                   'FilterMask':'QFilterMask',
                   'HLCVEMPulses':'OfflineIceTopHLCVEMPulses',
                   'HLCPulses':'OfflineIceTopHLCTankPulses',
                   'CleanHLCPulses':'CleanedHLCTankPulses',
                   'SLCVEMPulses':'OfflineIceTopSLCVEMPulses',
                   'SLCPulses':'OfflineIceTopSLCTankPulses',
                   'TankPulseMergerExcluded':'TankPulseMergerExcludedTanks',
                   'ClusterCleaningExcluded':'ClusterCleaningExcludedTanks',
                   'InIcePulses':'InIcePulses',
                   'CalibrationErrata':'CalibrationErrata',
                   'I3TriggerHierarchy':'DSTTriggers'},
    'IC86.2016': { 'sub_event_stream':'IceTopSplit',
                   'FilterMask':'QFilterMask',
                   'HLCVEMPulses':'OfflineIceTopHLCVEMPulses',
                   'HLCPulses':'OfflineIceTopHLCTankPulses',
                   'CleanHLCPulses':'CleanedHLCTankPulses',
                   'SLCVEMPulses':'OfflineIceTopSLCVEMPulses',
                   'SLCPulses':'OfflineIceTopSLCTankPulses',
                   'TankPulseMergerExcluded':'TankPulseMergerExcludedTanks',
                   'ClusterCleaningExcluded':'ClusterCleaningExcludedTanks',
                   'InIcePulses':'InIcePulses',
                   'CalibrationErrata':'CalibrationErrata',
                   'I3TriggerHierarchy':'DSTTriggers'},
    'Level3': { 'sub_event_stream':'ice_top',
                'FilterMask':'QFilterMask',
                'HLCVEMPulses':'OfflineIceTopHLCVEMPulses',
                'HLCPulses':'OfflineIceTopHLCTankPulses',
                'CleanHLCPulses':'CleanedHLCTankPulses',
                'SLCVEMPulses':'OfflineIceTopSLCVEMPulses',
                'SLCPulses':'OfflineIceTopSLCTankPulses',
                'TankPulseMergerExcluded':'TankPulseMergerExcludedTanks',
                'ClusterCleaningExcluded':'ClusterCleaningExcludedTanks',
                'InIcePulses':'InIcePulses',
                'CalibrationErrata':'CalibrationErrata',
                'I3TriggerHierarchy':'I3TriggerHierarchy'
                }
    }

# General
filtermask= 'QFilterMask'
sub_event_stream='ice_top'

#IceTop part
#pulses in L2
icetop_raw_data                            = 'IceTopRawData'
icetop_bad_doms                            = 'IceTopBadDOMs'                    # note: these are named IceTopBadDoms in online
icetop_bad_tanks                           = 'IceTopBadTanks'
icetop_hlc_vem_pulses                      = 'OfflineIceTopHLCVEMPulses'
icetop_slc_vem_pulses                      = 'OfflineIceTopSLCVEMPulses'
icetop_hlc_pulses                          = 'OfflineIceTopHLCTankPulses'       # these are the real icetop HLC pulses
icetop_slc_pulses                          = 'OfflineIceTopSLCTankPulses'       # these are the real icetop SLC pulses
icetop_clean_hlc_pulses                    = 'CleanedHLCTankPulses'             # this is only a pulse mask
#icetop_clean_slc_pulses                    = 'CleanedSLCTankPulses'             # this is only a pulse mask
icetop_tank_pulse_merger_excluded_tanks    = 'TankPulseMergerExcludedTanks'
icetop_cluster_cleaning_excluded_tanks     = 'ClusterCleaningExcludedTanks'

#Pulses created in L3
icetop_HLCseed_clean_hlc_pulses            = 'IceTopHLCSeedRTPulses'
icetop_HLCseed_excluded_tanks              = 'IceTopHLCSeedRTExcludedTanks'
#+ all the ones created by topeventcleaning.SelectPulsesFromSeed (IceTopLaputopSeeded...)

icetop_waveforms                           = 'CalibratedIceTopWaveforms'        # ???
icetop_pe_pulses                           = 'IceTopDSTPulses'
icetop_vem_pulses                          = 'OfflineIceTopVEMPulses'
icetop_clean_trigger_hierarchy             = 'CleanedIceTopTriggerHierarchy'    # ???
icetop_shower_cog                          = 'ShowerCOG'
icetop_shower_plane                        = 'ShowerPlane'
icetop_shower_laputop                      = 'Laputop'
icetop_small_shower_decision               = 'IsSmallShower'
icetop_small_shower_nstation_name          = 'SmallShowerNStations'
icetop_small_shower_laputop                = 'LaputopSmallShower'

#InIce part
inice_pulses='InIcePulses' 
inice_coinc_pulses='CoincPulses'   # These are the inice pulses related to the icetop ones
inice_clean_coinc_pulses='Coinc'+icetop_shower_laputop+'CleanedPulses'


