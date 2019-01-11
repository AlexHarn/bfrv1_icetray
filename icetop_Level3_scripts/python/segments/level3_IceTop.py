#!/usr/bin/env python
import os

def level3_IceTop(tray, name,
                  detector,
                  do_select = False,
                  isMC=False,
                  simulate_background_rate=0, # For now, we set this to 0. No background simulation was decided at the CR call.
                  add_jitter=False,
                  snowLambda=None,
                  ):

    """
    Not exactly in the order but more or less does these things for IceTop...

    Step 1 : Snow Factor, Snow Hacking/Interpolation, remove dethinning bug in IC79 mc 
    Step 2 : Remove Old Reconstructions, Remove InIce Frames, Update Names.
    Step 3 : SLC Calibration
    Step 4 : RT Cleaning
    Step 5 : IceTopReco using RT Cleaning, IceTopReco using CleanedHLCTankPulses
    Step 6 : SeededPulseSelector 
    """ 

    from icecube.icetray import I3Units
    from icecube import icetray, dataclasses, dataio, phys_services, icetop_Level3_scripts, toprec
    from icecube.icetray.i3logging import log_info,log_debug,log_fatal,log_warn
    from icecube.icetop_Level3_scripts import icetop_globals

    if not snowLambda:
        SnowFactor=icetop_globals.SnowFactors[detector]
        log_info("The snow lambda defined for each year will be used.")
    else:
        SnowFactor= snowLambda
        log_info("The snow lambda that you defined yourself will be used.")

    
    if (isMC and detector=="IC79"):
        # Due to bug in script, MCPrimaryInfo were put in the nullsplit frame during IC79 simulation.
        # First move MCPrimary and MCPrimaryInfo to Q frame
        tray.AddModule(icetop_Level3_scripts.modules.MoveMCPrimary,name+'_movemcprim')
        
        # Also replace the MCPrimary with the one from the MCTree again, due to time bug (reported by Emily on the call of July 5, 2016)
        # This could be made shorter when combined with the above module, but okay for now.
        def replace_MCPrimary(frame):
            if not frame.Has('MCPrimary'):
                return True
    
            if not frame.Has('I3MCTree'):
                icetray.logging.log_warn('No I3MCTree. Skipping frame.')
                return True
    
            if len(frame['I3MCTree'].get_primaries()) != 1:
                icetray.logging.log_fatal('I3MCTree contains {0} primaries'.format(len(frame['I3MCTree'].get_primaries())))
    
            mcp_old  = frame['MCPrimary']
            mcp_tree = frame['I3MCTree'].get_primaries()[0]
            
            if (mcp_old.major_id != mcp_tree.major_id) or (mcp_old.minor_id != mcp_tree.minor_id):
                icetray.logging.log_fatal('Primary particle: IDs do not match.')
    
            frame.Delete('MCPrimary')
            frame.Put('MCPrimary', mcp_tree)
            return True 
        tray.AddModule(replace_MCPrimary,name+'replace_MCPrim',Streams=[icetray.I3Frame.DAQ])
    
    # Remove all inice frames. Then remove the lonely Q frames. Actually, this should already be fine, but not in IC86 simulations. 
    tray.AddModule(lambda frame: frame['I3EventHeader'].sub_event_stream == icetop_globals.names[detector]["sub_event_stream"], name+'_subevent_stream')    
    tray.AddModule("I3OrphanQDropper",name+"_drop_q")

    # Now remove all events which did not pass a decent IT trigger (STA.. -trigger) and for STA3 events they should pass the smallshower filter.
    # For MC, we should wait with doing this if we do noise simulations, since these could cause events to pass which would not pass otherwise.
    # Running this here for data is what you should do, since this saves a lot of computing time.
    if not isMC:
        tray.AddSegment(icetop_Level3_scripts.segments.ReRunFilters,name+"_rerunFilters",
                        Detector=detector,
                        isMC=isMC)        
        
    # Remove now all Physics frames for MC.
    if isMC and simulate_background_rate > 0:
        tray.AddModule(lambda frame: frame.Stop != icetray.I3Frame.Physics, name + "_physics_dropper")
            
    # Unify naming over all years.
    # Convert excluded_stations to excluded_tanks for all years
    tray.AddModule(icetop_Level3_scripts.modules.UpdateNames, name+'_UpdateNames', Detector=detector)

    # Do SLC calibration (both charge and time)
    from icecube.tpx.segments import CalibrateSLCs
    tray.AddSegment(CalibrateSLCs, name+'_OfflineIceTopSLCVEMPulses',
                    SLCVEMPulses=icetop_globals.icetop_slc_vem_pulses,   # input 
                    SLCTankPulses=icetop_globals.icetop_slc_pulses,       # output, together with icetop_globals.icetop_slc_vem_pulses+'Calibrated' and TankPulseMergerExcludedSLCTanks
                    )
    
    tray.AddModule(icetop_Level3_scripts.modules.I3IceTopSLCTimeCorrect,name+'_SLCTimeCorrect',
                   SLCPulses=icetop_globals.icetop_slc_pulses,
                   If=lambda fr: icetop_globals.icetop_slc_pulses in fr and len(fr[icetop_globals.icetop_slc_pulses])>0)
    
    # Do background simulation
    # After slc calibration 
    if isMC and simulate_background_rate > 0:
        if simulate_background_rate is None:
            simulate_background_rate = icetop_globals.background_rate[detector]

        tray.AddSegment(icetop_Level3_scripts.segments.SimulateBackground, name + '_ITbackground',
                        HLCTankPulses=icetop_globals.icetop_hlc_pulses,
                        SLCTankPulses=icetop_globals.icetop_slc_pulses,
                        NoiseRate=simulate_background_rate,
                        AddJitter=add_jitter)

    # Now filter for MC 
    if isMC:
        tray.AddSegment(icetop_Level3_scripts.segments.ReRunFilters,name+"_rerunFilters",
                    Detector=detector,
                    isMC=isMC)

    # Do VEMCal correction for data, on all tankpulses.
    if not isMC:
        tray.AddModule(icetop_Level3_scripts.modules.RecalibrateVEMPulses,
                 InputPulsesP  = [icetop_globals.icetop_clean_hlc_pulses],
                 InputPulsesQ  = [icetop_globals.icetop_hlc_pulses, icetop_globals.icetop_slc_pulses]
                 )
    # Remove the L2 reconstructions
    tray.AddSegment(icetop_Level3_scripts.segments.level2_IceTop.RemoveOldLevel2, name+'_RemoveOldLevel2')

    CleanHLCTankPulses=icetop_globals.icetop_clean_hlc_pulses
    ExcludedHLCTanks=icetop_globals.icetop_cluster_cleaning_excluded_tanks
   
    # Toms seeded RT cleaning.
        
    tray.AddSegment(icetop_Level3_scripts.segments.CleanIceTop,name+'_clean_it',
                    detect_conf = detector,
                    it_pulses= CleanHLCTankPulses, 
                    it_pulses_clean=icetop_globals.icetop_HLCseed_clean_hlc_pulses,
                    excluded_tanks=ExcludedHLCTanks,
                    extra_excluded_tanks=icetop_globals.icetop_HLCseed_excluded_tanks
                    )

    # Fix dethinning bug present in IC79 simulation.     
    if (isMC and detector=="IC79"):
        
            tray.AddSegment(icetop_Level3_scripts.segments.FixIC79MCDeThinningBug,name+'_FixMCDeThin_SeededRT',
                            IT_pulses=icetop_globals.icetop_HLCseed_clean_hlc_pulses,
                            ExcludedTanks=icetop_globals.icetop_HLCseed_excluded_tanks
                            )

    CleanHLCTankPulses=icetop_globals.icetop_HLCseed_clean_hlc_pulses
    ExcludedHLCTanks=icetop_globals.icetop_HLCseed_excluded_tanks #Includes also the ClusterCleaningExcludedTanks 

    # Do ShowerCOG, ShowerPlane, Laputop with these pulses
    tray.AddSegment(icetop_Level3_scripts.segments.level2_IceTop.OfflineIceTopReco, name+'_RTOfflineIceTopReco',
                    Pulses=CleanHLCTankPulses,
                    Excluded=ExcludedHLCTanks,
                    SnowFactor=SnowFactor,
                    Detector=detector
                    )

    reco_track='Laputop'

    # Creates:
    # *IceTopLaputopSeededSelectedHLC
    # *IceTopLaputopSeededSelectedSLC
    # *IceTopLaputopSeededRejectedHLC
    # *IceTopLaputopSeededRejectedSLC
    # *IceTopLaputopSeededAfterPulsesSLC
    # *IceTopLaputopSeededAfterPulsesSLC

    try:
        from icecube.topeventcleaning.segments import SelectPulsesFromSeed
    except:
        from icecube.icetop_tank_select.segments import SelectPulsesFromSeed
    it_tank_select_seed = 'Laputop'
    tray.AddSegment(SelectPulsesFromSeed, name+'_SelectPulsesFromSeed',
		    Seed=it_tank_select_seed,
		    HLCPulses=icetop_globals.icetop_hlc_pulses,
		    SLCPulses=icetop_globals.icetop_slc_pulses,
		    debug=True,
		    tag='IceTopLaputopSeeded',
                    If=lambda fr: "Laputop" in fr and fr["Laputop"].fit_status_string=="OK")

    #if small events to be included, then you want SelectPulsesFromSeed using laputopsmall for small events.
    it_tank_select_seed = 'LaputopSmall'
    tray.AddSegment(SelectPulsesFromSeed, name+'_SelectPulsesFromSeedSmall',
                    Seed=it_tank_select_seed,
                    HLCPulses=icetop_globals.icetop_hlc_pulses,
                    SLCPulses=icetop_globals.icetop_slc_pulses,
                    debug=True,
                    tag='IceTopLaputopSmallSeeded',
                    If=lambda fr: "LaputopSmall" in fr and fr["Laputop"].fit_status_string!="OK")

    # "Snow correct" the pulses which are used for the laputop reconstruction.
    # Extra pulses with "_SnowCorrected" added will be created.
    # These can be used later on for checks, but NOT for a new reconstruction, since this is a non-perfect correction! 
    # The reconstruction should do its own snow correction.
    # These snow corrected pulses do can be used for Quality cuts (like maximal signal etc.)
    snow_correction = toprec.I3SimpleSnowCorrectionService("snow_correction", SnowFactor)
    tray.AddModule(icetop_Level3_scripts.modules.SnowCorrectPulses,name+'SnowCorrrect',
                   Pulses=[CleanHLCTankPulses],
                   Track=reco_track,
                   SnowService=snow_correction,
                   If=lambda fr: "Laputop" in fr and fr["Laputop"].fit_status_string=="OK")

    tray.AddModule(icetop_Level3_scripts.modules.SnowCorrectPulses,name+'SnowCorrrectSmall',
                   Pulses=[CleanHLCTankPulses],
                   Track=reco_track+"Small",
                   SnowService=snow_correction,
                   If=lambda fr: "LaputopSmall" in fr and fr["Laputop"].fit_status_string!="OK")
    
    # Do the cuts
    tray.AddSegment(icetop_Level3_scripts.segments.IceTopQualityCuts,name+'_IceTopQualityCuts',
                    pulses=CleanHLCTankPulses+"_SnowCorrected",
                    detector=detector,
                    removeOrNot=do_select,
                    reco_track=reco_track, 
                    isMC=isMC
                    )
    
