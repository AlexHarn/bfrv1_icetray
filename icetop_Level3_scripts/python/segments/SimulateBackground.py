#!/usr/bin/python
from icecube import icetray

# Simulate the background noise for IceTop and possible extra jitter.

@icetray.traysegment
def SimulateBackground(tray, name,
                       HLCTankPulses=None,
                       SLCTankPulses=None,
                       NoiseRate=1500,
                       AddJitter=False):
    
    from icecube.icetray import I3Units
    from icecube import icetray, dataclasses, dataio, phys_services, topeventcleaning
    from icecube.icetray.i3logging import log_info,log_debug,log_fatal,log_warn
    from icecube.icetop_Level3_scripts import icetop_globals
    from icecube import top_background_simulator
    if AddJitter:
        from icecube import top_charge_jitter_simulator

    log_info("SimulateBackground: background noise with rate {0} Hz will be added".format(NoiseRate))

    # add noise to uncleaned tank pulses
    tray.AddModule('TopBackgroundSimulator', name + '_noisesim',
            NoiseRate=NoiseRate,
            HLCTankPulses=HLCTankPulses,
            SLCTankPulses=SLCTankPulses)

    if AddJitter:
        # NOTE: need to change the seed settings, or include this in the module, as done in the background simulation 
        tray.AddService('I3GSLRandomServiceFactory', name+'_rng2',
                        Seed=1,
                        InstallServiceAs=name+'_rng2')

        # Default jitter is 5 % on HLCs and 10 % on SLCs. 
        tray.AddModule('TopChargeJitterSimulator', name+'_jitter_sim',
                       HLCTankPulses=HLCTankPulses,
                       SLCTankPulses=SLCTankPulses,
                       RandomServiceName=name+'_rng2')

    # remove stuff we are going to re-create
    tray.AddModule('Delete', name + '_delete',
                   Keys = [icetop_globals.icetop_cluster_cleaning_excluded_tanks])

    # run cluster cleaning
    tray.AddModule('I3TopHLCClusterCleaning', 'ice_top',
            InputPulses               = HLCTankPulses,
            OutputPulses              = icetop_globals.icetop_clean_hlc_pulses,
            BadTankList               = icetop_globals.icetop_tank_pulse_merger_excluded_tanks,
            ExcludedTanks             = icetop_globals.icetop_cluster_cleaning_excluded_tanks,
            InterStationTimeTolerance = 200.0 * I3Units.ns,              # Default
            IntraStationTimeTolerance = 200.0 * I3Units.ns,              # Default
            )
    


