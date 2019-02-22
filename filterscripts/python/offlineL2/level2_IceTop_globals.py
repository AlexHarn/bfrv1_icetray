# the online filter globals
from icecube.filterscripts import filter_globals

bad_doms_list                              = 'BadDomsList'

# from filterscripts/python/filter_globals.py
inicesmttriggered                          = filter_globals.inicesmttriggered
icetopsmttriggered                         = filter_globals.icetopsmttriggered

icetop_physics_stream                      = filter_globals.IceTopSplitter
icetop_bad_doms                            = filter_globals.IceTopBadDoms
icetop_bad_tanks                           = filter_globals.IcetopBadTanks
icetop_tank_pulse_merger_excluded_tanks    = filter_globals.TankPulseMergerExcludedTanks
icetop_cluster_cleaning_excluded_tanks     = filter_globals.ClusterCleaningExcludedTanks
icetop_pe_pulses                           = filter_globals.IceTopDSTPulses
icetop_vem_pulses                          = 'OfflineIceTopVEMPulses'
icetop_hlc_vem_pulses                      = 'OfflineIceTopHLCVEMPulses'
icetop_slc_vem_pulses                      = 'OfflineIceTopSLCVEMPulses'
icetop_hlc_pulses                          = 'OfflineIceTopHLCTankPulses'         # these are the real icetop pulses
icetop_clean_hlc_pulses                    = filter_globals.CleanedHLCTankPulses  # this is only a pulse mask
icetop_clean_coinc_pulses                  = 'CleanedCoincOfflinePulses'
icetop_clean_trigger_hierarchy             = 'CleanedIceTopTriggerHierarchy'
