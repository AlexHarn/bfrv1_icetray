from icecube import icetray
from icecube.icetop_Level3_scripts import icetop_globals

@icetray.traysegment
def ReRunFilters(tray,name,
                 Detector=None,
                 isMC=False,
                 Pulses=icetop_globals.icetop_clean_hlc_pulses
                 ):
    '''
    This segment is ran because of the inconsistency between the different years regarding filtermask names, etc.
    We will rerun the filter on the P frame. 
    As a first step we keep all events which pass an IceTopSTA3, STA5 or IceTop_InFill_STA3 filter will be kept.
    We run the same filter on all years, but for IC79, IC86.2011 we need to get the prescale from the FilterMask.
    We do not keep a separate icetop_inice filter (as is done in IC79, IC86.2011). 
    Furthermore, if only the STA3 filter is passed, we run the SmallShowerFilter, and delete the events which do not pass this filter. 
    '''
    from icecube import filterscripts, icetop_Level3_scripts, dataclasses
    from icecube.icetray.i3logging import log_info,log_debug,log_fatal,log_warn
    from icecube.icetop_Level3_scripts.icetop_globals import names

    icetray.load('smallshower-filter', False)                                                                                                                                                                   
    # This unpacking is not needed for the I3CosmicRayFilter module, but is done such that the final output in L3 is the same for all events in all years.
    def UnpackDSTTriggers(frame):
        if frame.Has('I3TriggerHierarchy'):
            return True
        if not frame.Has('DSTTriggers'):
            icetray.logging.log_error('Neither I3TriggerHierarchy nor DSTTriggers found')
            return True

        trigger_hierarchy = frame['DSTTriggers'].unpack(frame['I3DetectorStatus'])
    
        frame['I3TriggerHierarchy'] = trigger_hierarchy
        return True

    tray.AddModule(UnpackDSTTriggers,name+"_unpackDSTTriggers",
                   Streams = [icetray.I3Frame.DAQ]
                   )

    # For IC79, and IC86.2011, we check whether one of the IT only filters (IceTopSTA or IceTop_InFill) is passed.
    # If so, we keep the event with its according L2 prescale.
    if Detector=="IC79" or Detector=="IC86.2011":
        if Detector=="IC79":
            filters=["IceTopSTA3_10","IceTopSTA8_10"]
        else:
            filters=["IceTopSTA3_11","IceTopSTA8_11","IceTop_InFill_STA3_11"]

        # For MC, we run this part after we run the unify naming module, so we take the L3 FilterMaskname
        # For data, we run this as soon as possible, thus we use the L2 FilterMask
        if isMC:
            filterMaskName=names["Level3"]["FilterMask"]
        else:
            filterMaskName=names[Detector]["FilterMask"]

        def ITonlyFilters(frame,Filters,isMC,filterMaskName):
            keepEvent=False
            if any([frame[filterMaskName][itfilter].prescale_passed for itfilter in Filters]):
                keepEvent=True
            if keepEvent:
                prescale=1
                if not isMC:
                    if frame[filterMaskName][Filters[0]].prescale_passed and not frame[filterMaskName][Filters[1]].condition_passed:
                        prescale=3
                frame["IceTop_EventPrescale"]=icetray.I3Int(prescale)
            return keepEvent


        tray.AddModule(ITonlyFilters,name+"_CheckITFilters",Filters=filters, isMC=isMC, filterMaskName=filterMaskName)
        # If we remove P, we should also remove the Q frames
        tray.AddModule("I3OrphanQDropper",name+"_drop_IC79_IC86_2011_Q")

    # Now run the filters again on the P frames.
    tray.AddModule('I3FilterModule<I3CosmicRayFilter_13>', name + '_CRFilter13',
                   DecisionName       = 'I3CosmicRayFilter_13',
                   IceTopPulseMaskKey = Pulses,
                   TriggerEvalList    = list(),
                   TriggerKey         = 'I3TriggerHierarchy'
                   )
    
    # Run the SmallShower filter on the IceTopSTA3-only events
    # This module searches for "good" STA3-STA4 events, i.e. not on the edge and clustered.                                                                                                                 
    # First delete the L2 ones (They should actually be fine, but okay.)
    tray.AddModule("Delete",name+"_deleteSmallShower",
                   Keys=["IsSmallShower","SmallShowerNStations"])

    IceTopSmallShowerFilter = icetray.module_altconfig('I3IcePickModule<I3SmallShowerFilter>',
                                                       CacheResults = False,                     # Default                                                                                            
                                                       DiscardEvents = True,                    # Default                                                                                                  
                                                       FilterGeometry = "IC86",                  # Current detector configuration                                                                   
                                                       InvertFrameOutput = False,                # Default                                                                                         
                                                       NEventsToPick = -1                        # Default                                                                                          
                                                       )

    tray.AddModule('I3IcePickModule<I3SmallShowerFilter>', name + "_SmallShowerFilter",
                   TopPulseKey        = Pulses,
                   DecisionName       = "IsSmallShower",
                   NStationResultName = "SmallShowerNStations",
                   FilterGeometry     = Detector[:4],
                   If                 = lambda fr: fr.Stop==icetray.I3Frame.Physics and fr["IceTopSTA3_13"].value and not fr["IceTopSTA5_13"].value
                   )
    
    # IceTop Infill STA2 filter check in Level2 cleaned hlc pulses.
    tray.AddModule(icetop_Level3_scripts.modules.ReevaluateTriggerOnIceTopSplit,
                   name + '_ReevaluateTriggerOnIceTopSplit',
                   Input  = Pulses,
                   Output = 'IceTop_TwoStationFilter'
                   )

    # Now remove the events which did not pass the STA5, InFillSTA3, InFillSTA2, or IsSmallShower filter 
    # Also write booleans in the frame whether it passed any standard filter or the infill filter.
    # ["IceTop_StandardFilter","IceTop_InFillFilter"]
    def removeFiltersNotPassed(frame):
        inFillPassed=False
        standardPassed=False
        infillsta2Passed = False
        if frame["IceTop_InFill_STA3_13"].value:
            inFillPassed=True

        if frame["IceTopSTA5_13"].value or ("IsSmallShower" in frame and frame["IsSmallShower"].value):
            standardPassed=True

        if frame.Has('IceTop_TwoStationFilter') and frame["IceTop_TwoStationFilter"].value:
            infillsta2Passed=True

        frame["IceTop_InFillFilter"]=icetray.I3Bool(inFillPassed)
        frame["IceTop_StandardFilter"]=icetray.I3Bool(standardPassed)           

        return (inFillPassed or standardPassed or infillsta2Passed)
    
    tray.AddModule(removeFiltersNotPassed,name+"_removeUnFiltered")

    tray.AddModule("I3OrphanQDropper",name+"_drop_q_unfiltered")

    if Detector!="IC79" and Detector!="IC86.2011":
        # For the other years, just add a dummy prescale.
        # Only run it after all filtering, saves a bit of time
        def AddPrescale(frame):
            frame["IceTop_EventPrescale"]=icetray.I3Int(1)
        tray.AddModule(AddPrescale,name+"_addDummyPrescale")
    
