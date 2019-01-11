from icecube import icetray
 ## FIX the IC79 dethinning bug by removing the buggy charges :
@icetray.traysegment
def FixIC79MCDeThinningBug(tray,name, 
                           IT_pulses='CleanedHLCTankPulses',
                           ExcludedTanks='ClusterCleaningExcludedTanks'
                           ):
    from icecube import dataclasses,dataio, icetop_Level3_scripts
    from icecube.icetray.i3logging import log_fatal

    tray.AddModule("Rename",name+"_renameBuggy",
                   Keys = [IT_pulses,IT_pulses+'_preThinBug',
                           ExcludedTanks,ExcludedTanks+'_preThinBug'])
    
    # Outputs two I3RecoPulseSeriesMapMasks
    tray.AddModule(icetop_Level3_scripts.modules.cleanBadThinning,name+'_cleanme',
                   InputITpulseName = IT_pulses+'_preThinBug',
                   OutputPulseName = IT_pulses,
                   ExcludedPulseName = IT_pulses+'_BadThinPulses')
    

    # New thingy. Create Badtanks (rejected by cleanBadThinning) list.                                                                                                                                      
    # If there is a tank which has both a pulse which is accepted by cleanBadThinning and a pulse which is rejected by cleanBadThinning, we want to keep the good pulse, so will not put this tank in the badtanks list!
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
                   AcceptedPulses=IT_pulses,
                   RejectedPulses=IT_pulses+"_BadThinPulses",
                   ExcludedTanksName=IT_pulses+"_BadThinTanks",
                   )

    # Now add them to the list of excluded tanks created before.                                                                                                                                         
    tray.AddModule(icetop_Level3_scripts.modules.AddTanks,name+'_AddTanks',
                   TanksToAddName=IT_pulses+'_BadThinTanks',
                   ExcludedTanksName=ExcludedTanks+'_preThinBug',
                   OutputExcludedTanksName=ExcludedTanks
                   )
