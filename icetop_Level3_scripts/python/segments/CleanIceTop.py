from icecube import icetray
@icetray.traysegment
def CleanIceTop(tray, name,
                detect_conf='IT73_IC79',
                it_pulses='CleanedHLCTankPulses',
                it_pulses_clean='IT_RT_180m_450ns',
                excluded_tanks='ClusterCleaningExcludedTanks', 
                extra_excluded_tanks='ExtraCleanedExcludedTanks'               
                ):
    
    from icecube import dataclasses, SeededRTCleaning, icetop_Level3_scripts
    #apply SeededRTCleaning FOR ICETOP, because TopEventCleaning did not remove all bad pulses.
    ## for MC : DON't use thinbugfixed pulses yet, because could artificially create missing tanks/stations which is not good for SRT
    tray.AddModule( "I3SeededRTHitCleaningModule<I3RecoPulse>", name+"_IsolatedStationCut",
                    InputResponse = it_pulses ,    # ! Name of input 
                    OutputResponse = 'IT_RT_180m_450ns',        # ! Name of output 
                    DiscardedHLCResponse = "SRTExcludedPulses", # ! Name of HLC output that were cleaned out in SeededRT
                    RTRadius = 180 ,                            # ! Radius for RT cleaning
                    RTTime = 450 ,                              # ! Time for RT cleaning
                    DeepCoreRTRadius = -1 ,          # Default
                    DeepCoreRTTime = -1 ,            # Default
                    MaxIterations = 3 ,              # Don't only keep HLCCore, but also all stations within RT of the core (3 is dangerous for large Radius, just 1 is ok, but for small 1-station radius, do 3 iter because is already a strong SRT!)
                    Seeds = "HLCcore" ,              # ! do not use all HLC hits as seed
                    HLCCoreAllowNoSeeds =  True,
                    HLCCoreThreshold = 5 ,           # the core must be a cluster of 3 stations, thus 5 tanks surrounding one other in RT
                    CylinderHeight = -1 )            # Default

    # New thingy. Create Badtanks (rejected by SRT) list.
    # If there is a tank which has both a pulse which is accepted by SRT and a pulse which is rejected by SRT, we want to keep the good pulse, so will not put this tank in the badtanks list! 
    # Mainly useful for afterpulses!
    def MakeExcludedTanksList(frame, AcceptedPulses, RejectedPulses,ExcludedTanksName):
        if AcceptedPulses in frame and RejectedPulses in frame:
            accPulses=frame[AcceptedPulses]
            rejPulses=frame[RejectedPulses]
            if accPulses.__class__ == dataclasses.I3RecoPulseSeriesMapMask:
                accPulses = accPulses.apply(frame)
            if rejPulses.__class__ == dataclasses.I3RecoPulseSeriesMapMask:
                rejPulses = rejPulses.apply(frame)

            # create tank key list for accPulses                                                                                                                                                             
            accTanks=dataclasses.TankKey.I3VectorTankKey()
            for omk_acc in accPulses.keys():
                accTanks.append(dataclasses.TankKey(omk_acc))

            excludedTanks=dataclasses.TankKey.I3VectorTankKey()
            for omk in rejPulses.keys():
                tk=dataclasses.TankKey(omk)
                if not tk in accTanks:
                    excludedTanks.append(tk)
            frame[ExcludedTanksName]=excludedTanks


    tray.AddModule(MakeExcludedTanksList,name+"_makeExclList",
                   AcceptedPulses='IT_RT_180m_450ns',
                   RejectedPulses="SRTExcludedPulses",
                   ExcludedTanksName="SRTExcludedTanks",
                   )

    # Now add them to the list of excluded tanks created before.
    tray.AddModule(icetop_Level3_scripts.modules.AddTanks,name+'_AddTanks',    
                   TanksToAddName='SRTExcludedTanks',    
                   ExcludedTanksName=excluded_tanks,    
                   OutputExcludedTanksName=extra_excluded_tanks
                   )

    # Create RecoPulseSeriesMapMask instead of RecoPulseSeriesMap
    tray.AddModule(icetop_Level3_scripts.functions.makePulseMask,name+"_createSRTMask",
                   PrimaryPulsesName=it_pulses,
                   SecondaryPulsesName='IT_RT_180m_450ns',
                   SecondaryMaskName=it_pulses_clean)
    
