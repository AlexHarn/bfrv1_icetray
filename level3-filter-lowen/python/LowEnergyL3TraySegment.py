###############################################
# Tray Segments for 2012 DeepCore L3 Processing
# Adapted from 2011 L3 Scripts (J. Koskinen) to
# work on 2012 Data.
# J. Daughhetee daughjd@gatech.edu
###############################################


from icecube import dataclasses
from icecube import icetray
from icecube.icetray import I3Units
from icecube import NoiseEngine
from icecube import linefit
from icecube.DeepCore_Filter import DOMS
from icecube import dipolefit, tensor_of_inertia
#from icecube.filter_2012 import Globals
from icecube.STTools.seededRT.configuration_services import I3DOMLinkSeededRTConfigurationService
from icecube.level3_filter_lowen import level3_spe

@icetray.traysegment
def DeepCoreCuts( tray, name, Iffy = lambda f: True,splituncleaned='InIcePulses',alttws=False,DoXYCut=False,year='12'):

    Pulses = 'SRTTWOfflinePulsesDC'   ### Pulsemap Generated in L2

    icetray.load("static-twc",False)

    def PassedDCFilter(frame):
        if frame.Has("FilterMask"):
            if frame["FilterMask"].get("DeepCoreFilter_"+year).condition_passed:
                return True
            # end if()
        # end if()
        return False
    # end PassedDCFilter()

    def RunAlternateTWs(frame):
        if alttws:
           return True
        return False

    ### Run NoiseEngine ###
    tray.AddSegment( NoiseEngine.WithCleaners, "NoiseEnginess",
                     HitSeriesName = splituncleaned,
                     If = lambda f: PassedDCFilter(f)
                     )

    DOMList = DOMS.DOMS( "IC86EDC")

    ### Need to generate TWC Pulses // No longer present in L2 ###

    tray.AddModule('I3StaticTWC<I3RecoPulseSeries>', name + '_StaticTWC_DC',
                   InputResponse = splituncleaned,
                   OutputResponse = 'TWOfflinePulsesDC',
                   TriggerConfigIDs = [1011],
                   TriggerName = "I3TriggerHierarchy",
                   WindowMinus = 5000,
                   WindowPlus = 4000,
                   If = lambda f: PassedDCFilter(f) and not f.Has("TWOfflinePulsesDC")
                   )

    tray.AddModule( "I3OMSelection<I3RecoPulseSeries>", "GenDCInTWFidPulses",
                    selectInverse  = True,
                    InputResponse  = 'TWOfflinePulsesDC',
                    OutputResponse = 'TWOfflinePulsesDCFid',
		    OutputOMSelection = 'Baddies',
                    OmittedKeys    = DOMList.DeepCoreFiducialDOMs,
                    If = lambda f: PassedDCFilter(f) and not f.Has("TWOfflinePulsesDCFid")
                    )

    # #############################
    # The goal here is to reduce the noise events by looking
    # for events which have tightly time correlated hits in the
    # DC Fiducial volume.
    # #############################


    tray.AddModule( "I3OMSelection<I3RecoPulseSeries>", "GenDCPulses",
                    selectInverse  = True,
                    InputResponse  = splituncleaned,
                    OutputResponse = 'OfflinePulsesDCFid',
                    OmittedKeys    = DOMList.DeepCoreFiducialDOMs,
                    If = lambda f: PassedDCFilter(f)
                    )

    tray.AddModule( "I3OMSelection<I3RecoPulseSeries>", "GenICVetoPulses_0",
                    selectInverse     = False,
                    InputResponse     = splituncleaned,
                    OutputResponse    = "OfflinePulsesICVeto",
                    OutputOMSelection = "BadOM_ICVeto_0",
                    OmittedKeys       = DOMList.DeepCoreFiducialDOMs,
                    If = lambda f: PassedDCFilter(f)
                    )

    tray.AddModule("I3DeepCoreVeto<I3RecoPulse>", "deepcore_filter_pulses",
                   InputFiducialHitSeries = "OfflinePulsesDCFid",
                   InputVetoHitSeries     = "OfflinePulsesICVeto",
                   DecisionName           = "DCFilterPulses",
                   MinHitsToVeto          = 2,
                   VetoChargeName         = "DCFilterPulses_VetoPE",
                   VetoHitsName           = "DCFilterPulses_VetoHits",
                   If = lambda f: PassedDCFilter(f)
                   )
    
    # #############################
    # Run a dynamic time window cleaning over the
    # StaticTW cleaned recopulses in the DeepCore
    # fiducial region.
    # #############################

    tray.AddModule( "I3TimeWindowCleaning<I3RecoPulse>", "DynamicTimeWindow175", 
                    InputResponse  = "TWOfflinePulsesDCFid",
                    OutputResponse = "DCFidPulses_DTW175", 
                    TimeWindow     = 175,
                   If = lambda f: PassedDCFilter(f) and RunAlternateTWs(f)
                    )

    tray.AddModule( "I3TimeWindowCleaning<I3RecoPulse>", "DynamicTimeWindow200", 
                    InputResponse  = "TWOfflinePulsesDCFid",
                    OutputResponse = "DCFidPulses_DTW200", 
                    TimeWindow     = 200,
                   If = lambda f: PassedDCFilter(f) and RunAlternateTWs(f)
                    )

    tray.AddModule( "I3TimeWindowCleaning<I3RecoPulse>", "DynamicTimeWindow250",
                    InputResponse  = "TWOfflinePulsesDCFid",
                    OutputResponse = "DCFidPulses_DTW250",
                    TimeWindow     = 250,
                    If = lambda f: PassedDCFilter(f) and RunAlternateTWs(f)
                    )    
    tray.AddModule( "I3TimeWindowCleaning<I3RecoPulse>", "DynamicTimeWindow300",
                    InputResponse  = "TWOfflinePulsesDCFid",
                    OutputResponse = "DCFidPulses_DTW300",
                    TimeWindow     = 300,
                    If = lambda f: PassedDCFilter(f)
                    )

    # #############################
    # Put all the MicroCount variables
    # into a map for elegance
    # #############################

    def MicroCount(frame):
      # if If(frame):
        MicroValuesHits = dataclasses.I3MapStringInt()
        MicroValuesPE   = dataclasses.I3MapStringDouble()

        DTWs = ["DTW175","DTW200","DTW250", "DTW300"]
        for DTW in DTWs:
            totalCharge = 0
            if not frame.Has("DCFidPulses_" + DTW):
                MicroValuesHits["STW9000_" + DTW] = 0
                MicroValuesPE["STW9000_" + DTW] = totalCharge
                continue
            # end if()

            for omkey in frame["DCFidPulses_" + DTW].apply(frame):
                for pulse in omkey[1]:
                    totalCharge += pulse.charge
                # end for()
            # end for()
            MicroValuesHits["STW9000_" + DTW] = len(frame["DCFidPulses_" + DTW].apply(frame))
            MicroValuesPE["STW9000_" + DTW] = totalCharge
        # end for()
        frame["MicroCountHits"] = MicroValuesHits
        frame["MicroCountPE"] = MicroValuesPE
        return
 #     return
    # end MicroCount()

    # #############################
    # Add the MicroCount MapStringDouble object to the frame
    # #############################

    tray.AddModule( MicroCount, "MicroCount", If = lambda f: PassedDCFilter(f))

    # ##############################
    # Rewrite the QR_Box method from the
    # slc-veto project into python so I don't
    # have to faff about with a code review.
    # ##############################

    def CalculateC2QR6AndFirstHitVertexZ( frame, PulseSeries):
#      if If(frame):

        totalCharge = 0
        timeCharge  = {}
        timeOMKey   = {}

        geometry = frame[ "I3Geometry"]
        vertexZ = -999
        vertexY = -999
        vertexX = -999
        firstHitTime = 1e6


        for omkey in frame[ PulseSeries].apply(frame):
            for pulse in omkey[1]:
                timeCharge[pulse.time] = pulse.charge
                timeOMKey[pulse.time]  = omkey[0]
                totalCharge += pulse.charge
                if pulse.time < firstHitTime:
                    firstHitTime = pulse.time
                    vertexZ = geometry.omgeo[ omkey[0]].position.z
                    vertexY = geometry.omgeo[ omkey[0]].position.y
                    vertexX = geometry.omgeo[ omkey[0]].position.x
                # end if()                    
            # end for()
        # end for()

        frame[ "VertexGuessZ"] = dataclasses.I3Double( vertexZ)
        frame[ "VertexGuessY"] = dataclasses.I3Double( vertexY)
        frame[ "VertexGuessX"] = dataclasses.I3Double( vertexX)

        orderedTimeCharge = sorted(timeCharge.items())
        orderedTimeOMKey  = sorted(timeOMKey.items())

        redTimeCharge = orderedTimeCharge[2:]

        # Check to see whether there is at least one pulse
        # in the cleaned dataset. Normally this condition is
        # impossible to not satisfy,
        # but isolated HLC hits change the game.

        if len( redTimeCharge) < 1:
            print(" In the limit that zero/zero goes to large zero in the \n denominator, C2QR6 will be zero.")
            frame["C2QR6"] = dataclasses.I3Double(0)
            return
        # end if()

        totalCharge2 = 0
        C2QR6        = 0
        startTime    = redTimeCharge[0][0]

        for time, charge in redTimeCharge:
            if (time - startTime) < 600:
                C2QR6 += charge
            # end if()
            totalCharge2 += charge
        # end for()

        C2QR6 = C2QR6/totalCharge2

        if C2QR6 > 0:
            frame["C2QR6"] = dataclasses.I3Double(C2QR6)
        else:
            print(str(frame["I3EventHeader"].event_id) + " has somehow received a negative value of C2QR6")
            print(" value is :", C2QR6)
            return
        # end if()

        return
#      return
    # end CalculateC2QR6AndFirstHitVertexZ

    # #############################
    # Actually add the C2QR6 and
    # position of the first hit from the cleaned
    # pulse series to the frame
    # ##############################

    tray.AddModule( CalculateC2QR6AndFirstHitVertexZ, "CalcC2QR6AndFirstHitVertexZ", PulseSeries = Pulses, If = lambda f: PassedDCFilter(f))

    # ##############################
    # Rewrite the NAbove method from the
    # slc-veto project into python so I don't
    # have to faff about with a code review.
    # ##############################

    def CalculateNAbove200( frame, PulseSeries, ConfigID):
      #if If(frame):
        # Loop over all the triggers looking
        # for the times of all the selected triggers

        triggerTimes = []
        for trigger in frame[ "I3TriggerHierarchy"]:
            if trigger.key.config_id == ConfigID:
                triggerTimes.append( trigger.time)
            # end if()
        # end for()

        # Check that there is at least one of the
        # selected triggers.

        if len(triggerTimes) == 0:
            frame[ "NAbove200"] = dataclasses.I3Double( 0)
            return
        # end if()

        triggerTimes.sort()
        earliestTriggerTime = triggerTimes[0]

        chargeCounter = 0
        geometry = frame[ "I3Geometry"]
        try:
          for omkey in frame[ PulseSeries]:

            if not geometry.omgeo[ omkey[0]].position.z > -200 * I3Units.m:
                continue
            # end if()
            for pulse in omkey[1]:
                if (pulse.time - earliestTriggerTime) < 0 and (pulse.time - earliestTriggerTime) > -2000:
                    chargeCounter += pulse.charge
        except TypeError:

          for omkey in frame[ PulseSeries].apply(frame):

            if not geometry.omgeo[ omkey[0]].position.z > -200 * I3Units.m:
                continue
            # end if()
            for pulse in omkey[1]:
                if (pulse.time - earliestTriggerTime) < 0 and (pulse.time - earliestTriggerTime) > -2000:
                    chargeCounter += pulse.charge

        frame[ "NAbove200"] = dataclasses.I3Double( chargeCounter)

        return
    #  return
    # end CalculateNAbove200()

    # #############################
    # Actually add the NAbove200 variable
    # to the frame
    # ##############################

    tray.AddModule( CalculateNAbove200, "CalcNAbove200",
                    PulseSeries = splituncleaned,
                    ConfigID    = 1011,
                    If = lambda f: PassedDCFilter(f)
                    )

    # #############################
    # Jacob Daughhetee has shown that a ratio of 
    # SRT cleaned hits within DC to outside of 
    # DC can be a good bkg identifier as well.
    # Run the standard L2 SRT cleaning.
    # #############################


    tray.AddModule( "I3OMSelection<I3RecoPulseSeries>", "GenDCCFidSRTPulses",
                    selectInverse     = True,
                    InputResponse     = Pulses,
                    OutputResponse    = "SRTTWOfflinePulsesDCFid",
                    OutputOMSelection = "BadOM1",
                    OmittedKeys       = DOMList.DeepCoreFiducialDOMs,
                    If = lambda f: PassedDCFilter(f)
                    )

    tray.AddModule( "I3OMSelection<I3RecoPulseSeries>", "GenICVetoSRTPulses",
                    selectInverse     = False,
                    InputResponse     = Pulses,
                    OutputResponse    = "SRTTWOfflinePulsesICVeto",
                    OutputOMSelection = "BadOM2",
                    OmittedKeys       = DOMList.DeepCoreFiducialDOMs,
                    If = lambda f: PassedDCFilter(f)
                    )
    # ##############################
    # Suggestion by the WIMP group to
    # look at the RT cluster size of pulses
    # in the IC Veto region
    # ##############################

    classicSeededRTConfigService = I3DOMLinkSeededRTConfigurationService(
        treat_string_36_as_deepcore = False,
        allowSelfCoincidence    = True,
        useDustlayerCorrection  = False,
        dustlayerUpperZBoundary = 0*I3Units.m,
        dustlayerLowerZBoundary = -150*I3Units.m,
        ic_ic_RTTime           = 1000*I3Units.ns,
        ic_ic_RTRadius         = 250*I3Units.m
        )


    tray.AddModule( "I3StaticTWC<I3RecoPulseSeries>", "TWRTVetoPulses",
                    InputResponse    = splituncleaned,
                    OutputResponse   = "TWRTVetoSeries",
                    TriggerConfigIDs = [1011],
                    TriggerName      = "I3TriggerHierarchy",
                    WindowMinus      = 5000,
                    WindowPlus       = 0,
                    If = lambda f: PassedDCFilter(f)
                    )

    tray.AddModule( "I3OMSelection<I3RecoPulseSeries>", "GenRTVetoPulses",
                    selectInverse     = False,
                    InputResponse     = "TWRTVetoSeries",
                    OutputResponse    = "TWRTVetoSeries_ICVeto",
                    OutputOMSelection = "BadOM3",
                    OmittedKeys       = DOMList.DeepCoreFiducialDOMs,
                    If = lambda f: PassedDCFilter(f)
                    )

    tray.AddModule("I3RTVeto_RecoPulseMask_Module", "rtveto",
                   STConfigService         = classicSeededRTConfigService,
                   InputHitSeriesMapName   = "TWRTVetoSeries_ICVeto",
                   OutputHitSeriesMapName  = "RTVetoSeries250",
                   Streams                 = [icetray.I3Frame.Physics],
                   If = lambda f: PassedDCFilter(f)
                  )

    #tray.AddModule( "I3SeededRTHitCleaningModule<I3RecoPulse>", "RTVeto250",
    #                InputResponse  = "TWRTVetoSeries_ICVeto",
    #                OutputResponse = "RTVetoSeries250",
    #                Seeds          = "OneHitAutoSeed",
    #                RTRadius       = 250*I3Units.m,
    #                If = lambda f: PassedDCFilter(f)
    #                )

    def CountRTVetoSeriesNChannel( frame ):
#      if If(frame):
        totalCharge = 0
        if frame.Has("RTVetoSeries250"):
            for omkey in frame["RTVetoSeries250"].apply(frame):
                for pulse in omkey[1]:
                    totalCharge += pulse.charge
                # end for()
            # end for()
        # end if()
        frame["RTVetoSeries250PE"] = dataclasses.I3Double( totalCharge)
        return
 #     return
    # end CountRTVetoSeriesNChannel()

    tray.AddModule( CountRTVetoSeriesNChannel, "CountRTVetoSeries",If = lambda f: PassedDCFilter(f))

    # ##############################
    # Create a variable that gets pushed
    # to the frame that checks whether
    # the event passed the IC2011 LE L3
    # straight cuts and cleaning.
    # ##############################

    def passIC2012_LE_L3(frame):
#      if If(frame):
        IC2012_LE_L3_FrameObjects = [ "DCFilterPulses_VetoPE",
                                      "MicroCountHits",
                                      "MicroCountPE",
                                      "NAbove200",
                                      "NoiseEngine_bool",
                                      "C2QR6",
                                      "RTVetoSeries250PE",
                                      "SRTTWOfflinePulsesDCFid",
                                      "SRTTWOfflinePulsesICVeto",
                                      "VertexGuessZ"
                                      ]

        frameObjects = frame.keys()
        matchingFrameObjects =  set(IC2012_LE_L3_FrameObjects).intersection( set(frameObjects))

        if len(matchingFrameObjects) == 10:

            # Count the number of SRT cleaned PEs in the DeepCore fiducial and
            # IceCube Veto region for use in the Ratio Cut variable. Also,
            # count the number of PE for hits satisying the DeepCore Filter
            # `speed of light' criteria.

            totalChargeFiducial = 0
            totalChargeVeto     = 0
            for omkey in frame["SRTTWOfflinePulsesDCFid"]:
                for pulse in omkey[1]:
                    totalChargeFiducial += pulse.charge
                # end for()
            # end for()
            for omkey in frame["SRTTWOfflinePulsesICVeto"]:
                for pulse in omkey[1]:
                    totalChargeVeto += pulse.charge
                # end for()
            # end for()

            LE_L3_Vars = dataclasses.I3MapStringDouble()
            LE_L3_Vars["NoiseEngine"]        = frame["NoiseEngine_bool"].value
            LE_L3_Vars["STW9000_DTW175PE"]   = frame["MicroCountPE"].get("STW9000_DTW175")
            LE_L3_Vars["STW9000_DTW200PE"]   = frame["MicroCountPE"].get("STW9000_DTW200")
            LE_L3_Vars["STW9000_DTW250PE"]   = frame["MicroCountPE"].get("STW9000_DTW250")
            LE_L3_Vars["STW9000_DTW300PE"]   = frame["MicroCountPE"].get("STW9000_DTW300")
            LE_L3_Vars["STW9000_DTW175Hits"] = frame["MicroCountHits"].get("STW9000_DTW175")
            LE_L3_Vars["STW9000_DTW200Hits"] = frame["MicroCountHits"].get("STW9000_DTW200")
            LE_L3_Vars["STW9000_DTW250Hits"] = frame["MicroCountHits"].get("STW9000_DTW250")
            LE_L3_Vars["STW9000_DTW300Hits"] = frame["MicroCountHits"].get("STW9000_DTW300")
            LE_L3_Vars["C2QR6"]              = frame["C2QR6"].value
            LE_L3_Vars["NAbove200PE"]        = frame["NAbove200"].value
            LE_L3_Vars["DCFiducialPE"]       = totalChargeFiducial
            LE_L3_Vars["ICVetoPE"]           = totalChargeVeto
            LE_L3_Vars["CausalVetoPE"]       = frame["DCFilterPulses_VetoPE"].value
            LE_L3_Vars["CausalVetoHits"]     = frame["DCFilterPulses_VetoHits"].value
            LE_L3_Vars["VertexGuessZ"]       = frame["VertexGuessZ"].value
            LE_L3_Vars["VertexGuessY"]       = frame["VertexGuessY"].value
            LE_L3_Vars["VertexGuessX"]       = frame["VertexGuessX"].value
            LE_L3_Vars["RTVeto250PE"]        = frame["RTVetoSeries250PE"].value
            LE_L3_Vars["NchCleaned"]         = len(frame["SRTTWOfflinePulsesDC"].apply(frame))

            frame["IC2012_LE_L3_Vars"] = LE_L3_Vars

            if (frame["NoiseEngine_bool"].value and
		frame["MicroCountHits"].get("STW9000_DTW300") > 2 and
		frame["MicroCountPE"].get("STW9000_DTW300") > 2 and
		frame["NAbove200"].value < 12 and
		totalChargeFiducial > 0 and
		frame["VertexGuessZ"].value < -120 and
		frame["DCFilterPulses_VetoPE"].value < 7.0) :

                rt_veto_pass = ( (frame["RTVetoSeries250PE"].value < 4.0 and LE_L3_Vars["DCFiducialPE"] < 100) or \
                                     (frame["RTVetoSeries250PE"].value < 6.0 and LE_L3_Vars["DCFiducialPE"] >= 100 and LE_L3_Vars["DCFiducialPE"] < 150) or \
                                     (frame["RTVetoSeries250PE"].value < 10.0 and LE_L3_Vars["DCFiducialPE"] >= 150 and LE_L3_Vars["DCFiducialPE"] < 200) or (LE_L3_Vars["DCFiducialPE"] >= 200) )

		### Apply Track Energy Dependent Cuts // Send Failures to Expanded Fiducial Branch ###    

                if (totalChargeVeto*1.0/totalChargeFiducial < 1.5) and frame["C2QR6"].value > 0.4 and rt_veto_pass:
                    frame["IC2012_LE_L3"] = icetray.I3Bool(True)
                    frame["IC2012_LE_L3_No_RTVeto"] = icetray.I3Bool(True)
                    frame.Put("ExpBranchL3Fodder",icetray.I3Bool(False))
                elif (totalChargeVeto*1.0/totalChargeFiducial < 1.5) and frame["C2QR6"].value > 0.4 and not rt_veto_pass:
                    frame["IC2012_LE_L3"] = icetray.I3Bool(False)
                    frame["IC2012_LE_L3_No_RTVeto"] = icetray.I3Bool(True)
                    frame.Put("ExpBranchL3Fodder",icetray.I3Bool(False))
                elif rt_veto_pass:
                    frame["IC2012_LE_L3"] = icetray.I3Bool(False)
                    frame["IC2012_LE_L3_No_RTVeto"] = icetray.I3Bool(False)
                    frame.Put("ExpBranchL3Fodder",icetray.I3Bool(True))
                else:
                    frame["IC2012_LE_L3"] = icetray.I3Bool(False)
                    frame["IC2012_LE_L3_No_RTVeto"] = icetray.I3Bool(False)
                    frame.Put("ExpBranchL3Fodder",icetray.I3Bool(False))
                return
            # end if()
        # end if()
        frame["IC2012_LE_L3"]  = icetray.I3Bool(False)
        frame.Put("ExpBranchL3Fodder",icetray.I3Bool(False))
        return
    #  return
    # end def passIC2012_LE_L3()

    tray.AddModule( passIC2012_LE_L3, "LE_L3_pass",If = lambda f: PassedDCFilter(f))

    # ############################################
    # Remove all the bad omselections created when
    # producing the MicroCount outputs as well as
    # other junk from L3 processing
    # ############################################

    tray.AddModule("Delete", "delete_badomselection",
                   Keys = ["BadOMSelection",
                           "BadOM1",
                           "BadOM2",
                           "BadOM3",
                           "BadOM_DCFid_0",
                           "BadOM_ICVeto_0",
                           "DCFidPulses_DTW175",
                           "DCFidPulses_DTW200",
                           "DCFidPulses_DTW250",
                           "DCFidPulses_DTW300",
                           "SplitInIcePulses_STW_NoiseEnginess",
                           "SplitInIcePulses_STW_ClassicRT_NoiseEnginess",
                           "SRTTWOfflinePulsesICVeto",
                           "SRTTWOfflinePulsesDCFid",
                           "DCFilterPulses_VetoPE",
                           "DCFilterPulses_VetoHits",
                           "DCFilterPulses",
                           "C2QR6",
                           "Baddies",
                           "RTVetoSeries250",
                           "RTVetoSeries250PE",
                           "TWRTVetoSeriesTimeRange",
                           "TWRTVetoSeries_ICVeto",
                           "OfflinePulsesDCFid",
                           "OfflinePulsesICVeto",
                           "VertexGuessX",
                           "VertexGuessY",
                           "VertexGuessZ",
                           "TWOfflinePulsesDCFid",
                           "NAbove200",
                           "MicroCountPE",
                           "MicroCountHits",
                           'SRTTWOfflinePulsesICVetoCleanedKeys',
                           'SRTTWOfflinePulsesDCFidCleanedKeys',
                           'TWOfflinePulsesDCFidCleanedKeys',
                           'TWRTVetoSeries_ICVetoCleanedKeys',
                           'OfflinePulsesDCFidCleanedKeys',
                           'OfflinePulsesICVetoCleanedKeys',
                           ]
                   )


    return



@icetray.traysegment
def ExpFidDeepCoreCuts( tray, name, If = lambda f: True,splituncleaned='InIcePulses',alttws=False,DoXYCut=True,year='12'):
    
    icetray.load("static-twc",False)
    from icecube.STTools.seededRT.configuration_services import I3DOMLinkSeededRTConfigurationService
    #from icecube.std_processing import level2_MuonReco, level2_globals

    Pulses = "SRTTWOfflinePulsesExpDC"   ### Standard Cleaned Pulses for Expanded Fiducial Branch

    ### Define Point inside Polygon function for VertexGuess ###

    def point_inside_polygon(x,y,poly):

      n = len(poly)
      inside =False

      p1x,p1y = poly[0]
      for i in range(n+1):
        p2x,p2y = poly[i % n]
        if y > min(p1y,p2y):
            if y <= max(p1y,p2y):
                if x <= max(p1x,p2x):
                    if p1y != p2y:
                        xinters = (y-p1y)*(p2x-p1x)/(p2y-p1y)+p1x
                    if p1x == p2x or x <= xinters:
                        inside = not inside
        p1x,p1y = p2x,p2y

      return inside

    ### Find Events that pass on the expanded filter branch OR failed E-dependent L3 Cuts in Standard Branch ###

    def ExpFidL3EventInput(frame):
        if frame.Has("FilterMask"):
            if frame.Has("DeepCoreFilter_TwoLayerExp_"+year):
                if frame["FilterMask"].get("DeepCoreFilter_TwoLayerExp_"+year).condition_passed and not frame["FilterMask"].get("DeepCoreFilter_"+year).condition_passed:
                    return True
        if frame.Has('ExpBranchL3Fodder'):
            if frame["FilterMask"].get("DeepCoreFilter_"+year).condition_passed and frame["ExpBranchL3Fodder"].value:
                return True
        return False


    ### Perform all alternate TWs for MicroHits? ###

    def RunAlternateTWs(frame):
        if alttws:
           return True
        return False

    ### Separate DCTrig events from those without DC trigger ###

    def MoreThanDCTrig(frame):
        if frame.Has('I3TriggerHierarchy'):
                moredctrig=False
                for i in frame['I3TriggerHierarchy']:
                        if i.key.config_id == 1006 or i.key.config_id == 21001:
                                moredctrig=True
                return moredctrig

    ### Events piped in from Std L3 need to be wiped clean ###

    def LEL3_ScrapCleanup(frame):
            if frame.Has('ExpBranchL3Fodder'):
                if frame['ExpBranchL3Fodder'].value:
                    frame.Delete('MicroCountPE')
                    frame.Delete('MicroCountHits')
                    frame.Delete('DCFilterPulses_VetoPE')
                    frame.Delete('NAbove200')
                    frame.Delete('C2QR6')
                    frame.Delete('DCFilterPulses_VetoHits')
                    frame.Delete('RTVetoSeries250PE')
                    frame.Delete('NchCleaned')
                    frame.Delete('VertexGuessX')
                    frame.Delete('VertexGuessY')
                    frame.Delete('VertexGuessZ')
                    frame.Delete('DCFilterPulses')
                    frame.Delete('RTVetoSeries250')
                    frame.Delete('TWRTVetoSeries')
                    return
                return

    tray.AddModule( LEL3_ScrapCleanup, "CleanSlate")

    ### Generate Hit Series ###

    ### Events Exclusive to Expanded Branch Filter Miss 2012 L2 Processing! ###

    classicSeededRTConfigService = I3DOMLinkSeededRTConfigurationService(
        treat_string_36_as_deepcore = False,
        allowSelfCoincidence    = True,
        useDustlayerCorrection  = False,
        dustlayerUpperZBoundary = 0*I3Units.m,
        dustlayerLowerZBoundary = -150*I3Units.m,
        ic_ic_RTTime           = 1000*I3Units.ns,
        ic_ic_RTRadius         = 150*I3Units.m
        )

    tray.AddModule("I3SeededRTCleaning_RecoPulse_Module", "MissingL2seededRTcleaning",
        STConfigService         = classicSeededRTConfigService,
        InputHitSeriesMapName   = splituncleaned,
        OutputHitSeriesMapName  = "SRTInIcePulses",
        SeedProcedure           = "HLCCoreHits",
        NHitsThreshold          = 2,
        Streams                 = [icetray.I3Frame.Physics],
        If = lambda f: not f.Has('SRTInIcePulses') and ExpFidL3EventInput(f)
        )

    
    #######################################################################

    tray.AddModule( "I3TimeWindowCleaning<I3RecoPulse>", "TimeWindowDC" ) (
        ( "InputResponse",               splituncleaned ),    # ! Use pulse series
        ( "OutputResponse",              "TWOfflinePulsesExpDC" ), # ! Name of cleaned pulse series
        ( "TimeWindow",                  9000 * I3Units.ns ),         # ! 9 usec time window
        ( "If",                          lambda f: ExpFidL3EventInput(f))
        )

    tray.AddModule( "I3TimeWindowCleaning<I3RecoPulse>", "TimeWindowSRTDC",
        InputResponse = 'SRTInIcePulses',    # ! Use pulse series
        OutputResponse = "SRTTWOfflinePulsesExpDC", # ! Name of cleaned pulse series
        TimeWindow = 9000 * I3Units.ns,        # ! 9 usec time window
        If= lambda f: ExpFidL3EventInput(f)
        )

    tray.AddModule('I3DipoleFit',  'DCL2_DipoleFit_EDC',
                   Name = 'DipoleFit_EDC',
                   InputRecoPulses = 'SRTTWOfflinePulsesExpDC',
                   AmpWeightPower = 0.0,
                   DipoleStep = 0,
                   MinHits = 5,
                   If = lambda f: ExpFidL3EventInput(f) and not f.Has('DipoleFit_DC'),
                   )

    tray.AddModule('I3TensorOfInertia', 'DCL2_ToI_EDC',
                   InputReadout = 'SRTTWOfflinePulsesExpDC',
                   Name = 'ToI_EDC',
                   MinHits = 3,
                   AmplitudeOption = 1,
                   AmplitudeWeight = 1,
                   InputSelection = '',
                   If = lambda f: ExpFidL3EventInput(f) and not f.Has('ToI_DC'),
                   )

    tray.AddSegment(level3_spe.SPE, 'SPE_EDC', Pulses = 'SRTTWOfflinePulsesExpDC', N_iter = 2, If = lambda f: ExpFidL3EventInput(f) and not f.Has('SPEFit2_DC'), suffix = '_EDC')

    tray.AddModule("Rename","Wolololo",keys=['SPEFit2_EDC','SPEFit2_DC','ToI_EDC',"ToI_DC",'CascadeLast_EDC','CascadeLast_DC',
                                           'DipoleFit_EDC','DipoleFit_DC','LineFit_EDC','LineFit_DC','ToI_EDCParams','ToI_DCParams'],
                                     If = lambda f: ExpFidL3EventInput(f))
 

    # #############################
    # The goal here is to reduce the noise events by looking
    # for events which have tightly time correlated hits in the
    # DC Fiducial volume.
    # #############################

    DOMList = DOMS.DOMS( "IC86TwoLayVeto")

    tray.AddModule( "I3OMSelection<I3RecoPulseSeries>", "GenDCCExpFidPulses",
                    selectInverse  = True,
                    InputResponse  = "TWOfflinePulsesExpDC",
                    OutputResponse = "TWOfflinePulsesExpDCFid",
                    OmittedKeys    = DOMList.DeepCoreFiducialDOMs,
                    If= lambda f: ExpFidL3EventInput(f)
                    )

    # #############################
    # Run the DCVeto using pulses instead
    # of launches. The IC2011 online veto
    # used DOMLaunches.
    # #############################

    tray.AddModule( "I3OMSelection<I3RecoPulseSeries>", "GenDCCExpFidPulses_0",
                    selectInverse     = True,
                    InputResponse     = splituncleaned,
                    OutputResponse    = "OfflinePulsesExpDCFid",
                    OutputOMSelection = "BadOM_DCFid_0",
                    OmittedKeys       = DOMList.DeepCoreFiducialDOMs,
                    If= lambda f: ExpFidL3EventInput(f)
                    )

    tray.AddModule( "I3OMSelection<I3RecoPulseSeries>", "GenICTwoLayVetoPulses_0",
                    selectInverse     = False,
                    InputResponse     = splituncleaned,
                    OutputResponse    = "OfflinePulsesICTwoLayVeto",
                    OutputOMSelection = "BadOM_ICVeto_0",
                    OmittedKeys       = DOMList.DeepCoreFiducialDOMs,
                    If= lambda f: ExpFidL3EventInput(f)
                    )

    tray.AddModule("I3DeepCoreVeto<I3RecoPulse>", "expdeepcore_filter_pulses",
                   InputFiducialHitSeries = "OfflinePulsesExpDCFid",
                   InputVetoHitSeries     = "OfflinePulsesICTwoLayVeto",
                   DecisionName           = "DCFilterPulses",
                   MinHitsToVeto          = 2,
                   VetoChargeName         = "DCFilterPulses_VetoPE",
                   VetoHitsName           = "DCFilterPulses_VetoHits",
                   If = lambda f: ExpFidL3EventInput(f)
                   )

    # #############################
    # Runs an X microsecond dynamic time window cleaning over the
    # StaticTW cleaned recopulses in the DeepCore
    # fiducial region.
    # #############################

    tray.AddModule( "I3TimeWindowCleaning<I3RecoPulse>", "ExpFidDynamicTimeWindow175",
                    InputResponse  = "TWOfflinePulsesExpDCFid",
                    OutputResponse = "DCFidPulses_DTW175",
                    TimeWindow     = 175,
                    If = lambda f: ExpFidL3EventInput(f) and RunAlternateTWs(f)
                    )

    tray.AddModule( "I3TimeWindowCleaning<I3RecoPulse>", "ExpFidDynamicTimeWindow200",
                    InputResponse  = "TWOfflinePulsesExpDCFid",
                    OutputResponse = "DCFidPulses_DTW200",
                    TimeWindow     = 200,
                    If = lambda f: ExpFidL3EventInput(f) and RunAlternateTWs(f)
                    )

    tray.AddModule( "I3TimeWindowCleaning<I3RecoPulse>", "ExpFidDynamicTimeWindow250",
                    InputResponse  = "TWOfflinePulsesExpDCFid",
                    OutputResponse = "DCFidPulses_DTW250",
                    TimeWindow     = 250,
                    If = lambda f: ExpFidL3EventInput(f) and RunAlternateTWs(f)
                    )
    tray.AddModule( "I3TimeWindowCleaning<I3RecoPulse>", "ExpFidDynamicTimeWindow300",
                    InputResponse  = "TWOfflinePulsesExpDCFid",
                    OutputResponse = "DCFidPulses_DTW300",
                    TimeWindow     = 300,
                    If = lambda f: ExpFidL3EventInput(f)
                    )

    # #############################
    # Put all the MicroCount variables
    # into a map for elegance
    # #############################

    def MicroCount(frame):
#      if If(frame):
        MicroValuesHits = dataclasses.I3MapStringInt()
        MicroValuesPE   = dataclasses.I3MapStringDouble()

        DTWs = ["DTW175","DTW200","DTW250", "DTW300"]
        for DTW in DTWs:
            totalCharge = 0
            if not frame.Has("DCFidPulses_" + DTW):
                MicroValuesHits["STW9000_" + DTW] = 0
                MicroValuesPE["STW9000_" + DTW] = totalCharge
                continue
            # end if()

            for omkey in frame["DCFidPulses_" + DTW].apply(frame):
                for pulse in omkey[1]:
                    totalCharge += pulse.charge
                # end for()
            # end for()
            MicroValuesHits["STW9000_" + DTW] = len(frame["DCFidPulses_" + DTW].apply(frame))
            MicroValuesPE["STW9000_" + DTW] = totalCharge
        # end for()
        frame["MicroCountHits"] = MicroValuesHits
        frame["MicroCountPE"] = MicroValuesPE
        return
    #  return
    # end MicroCount()

    # #############################
    # Add the MicroCount MapStringDouble object to the frame
    # #############################
    tray.AddModule( MicroCount, "ExpMicroCount",If = lambda f: ExpFidL3EventInput(f))

    # ##############################
    # Rewrite the QR_Box method from the
    # slc-veto project into python so I don't
    # have to faff about with a code review.
    # ##############################

    def CalculateC2QR6AndFirstHitVertexZ( frame, PulseSeries):

      #if If(frame):
        totalCharge = 0
        timeCharge  = {}
        timeOMKey   = {}

        geometry = frame[ "I3Geometry"]
        vertexZ = -999
        vertexY = -999
        vertexX = -999
        firstHitTime = 1e6


        for omkey in frame[ PulseSeries].apply(frame):
            for pulse in omkey[1]:
                timeCharge[pulse.time] = pulse.charge
                timeOMKey[pulse.time]  = omkey[0]
                totalCharge += pulse.charge
                if pulse.time < firstHitTime:
                    firstHitTime = pulse.time
                    vertexZ = geometry.omgeo[ omkey[0]].position.z
                    vertexY = geometry.omgeo[ omkey[0]].position.y
                    vertexX = geometry.omgeo[ omkey[0]].position.x
                # end if()                    
            # end for()
        # end for()

        frame["VertexGuessZ"] = dataclasses.I3Double( vertexZ)
        frame["VertexGuessY"] = dataclasses.I3Double( vertexY)
        frame["VertexGuessX"] = dataclasses.I3Double( vertexX)

        orderedTimeCharge = sorted(timeCharge.items())
        orderedTimeOMKey  = sorted(timeOMKey.items())

        redTimeCharge = orderedTimeCharge[2:]

        # Check to see whether there is at least one pulse
        # in the cleaned dataset. Normally this condition is
        # impossible to not satisfy,
        # but isolated HLC hits change the game.

        if len( redTimeCharge) < 1:
            print(" In the limit that zero/zero goes to large zero in the \n denominator, C2QR6 will be zero.")
            frame["C2QR6"] = dataclasses.I3Double(0)
            return
        # end if()

        totalCharge2 = 0
        C2QR6        = 0
        startTime    = redTimeCharge[0][0]

        for time, charge in redTimeCharge:
            if (time - startTime) < 600:
                C2QR6 += charge
            # end if()
            totalCharge2 += charge
        # end for()

        C2QR6 = C2QR6/totalCharge2

        if C2QR6 > 0:
            frame["C2QR6"] = dataclasses.I3Double(C2QR6)
        else:
            print(str(frame["I3EventHeader"].event_id) + " has somehow received a negative value of C2QR6")
            print(" value is :", C2QR6)
            return
        # end if()

        return
    #  return
    # end CalculateC2QR6AndFirstHitVertexZ

    # #############################
    # Actually add the C2QR6 and
    # position of the first hit from the cleaned
    # pulse series to the frame
    # ##############################

    tray.AddModule( CalculateC2QR6AndFirstHitVertexZ, "ExpCalcC2QR6AndFirstHitVertexZ", PulseSeries = 'SRTTWOfflinePulsesExpDC',If =  lambda f: ExpFidL3EventInput(f))

    # ##############################
    # Rewrite the NAbove method from the
    # slc-veto project into python so I don't
    # have to faff about with a code review.
    # ##############################

    def CalculateNAbove200( frame, PulseSeries, ConfigID):
      #if If(frame):

        # Loop over all the triggers looking
        # for the times of all the selected triggers

        triggerTimes = []
        for trigger in frame[ "I3TriggerHierarchy"]:
            if trigger.key.config_id in ConfigID:
                triggerTimes.append( trigger.time)
            # end if()
        # end for()

        # Check that there is at least one of the
        # selected triggers.

        if len(triggerTimes) == 0:
            frame[ "NAbove200"] = dataclasses.I3Double( 0)
            return
        # end if()

        triggerTimes.sort()
        earliestTriggerTime = triggerTimes[0]

        chargeCounter = 0
        geometry = frame[ "I3Geometry"]
        try:
          for omkey in frame[ PulseSeries]:

            if not geometry.omgeo[ omkey[0]].position.z > -200 * I3Units.m:
                continue
            # end if()
            for pulse in omkey[1]:
                if (pulse.time - earliestTriggerTime) < 0 and (pulse.time - earliestTriggerTime) > -2000:
                    chargeCounter += pulse.charge
        except TypeError:

          for omkey in frame[ PulseSeries].apply(frame):

            if not geometry.omgeo[ omkey[0]].position.z > -200 * I3Units.m:
                continue
            # end if()
            for pulse in omkey[1]:
                if (pulse.time - earliestTriggerTime) < 0 and (pulse.time - earliestTriggerTime) > -2000:
                    chargeCounter += pulse.charge

        frame[ "NAbove200"] = dataclasses.I3Double( chargeCounter)

        return
    #  return
    # end CalculateNAbove200()

    # #############################
    # Actually add the NAbove200 variable
    # to the frame
    # ##############################

    tray.AddModule( CalculateNAbove200, "ExpCalcNAbove200",
                    PulseSeries = splituncleaned,
                    ConfigID    = [1011,1006,21001],
                    If =  lambda f: ExpFidL3EventInput(f)
                    )

    # #############################
    # Jacob Daughhetee has shown that a ratio of 
    # SRT cleaned hits within DC to outside of 
    # DC can be a good bkg identifier as well.
    # Run the standard L2 SRT cleaning.
    # #############################


    tray.AddModule( "I3OMSelection<I3RecoPulseSeries>", "GenDCCExpFidSRTPulses",
                    selectInverse     = True,
                    InputResponse     = Pulses,
                    OutputResponse    = "SRTTWOfflinePulsesExpDCFid",
                    OutputOMSelection = "BadOM1",
                    OmittedKeys       = DOMList.DeepCoreFiducialDOMs,
                    If =  lambda f: ExpFidL3EventInput(f)
                    )

    tray.AddModule( "I3OMSelection<I3RecoPulseSeries>", "GenICTwoLayVetoSRTPulses",
                    selectInverse     = False,
                    InputResponse     = Pulses,
                    OutputResponse    = "SRTTWOfflinePulsesICTwoLayVeto",
                    OutputOMSelection = "BadOM2",
                    OmittedKeys       = DOMList.DeepCoreFiducialDOMs,
                    If =  lambda f: ExpFidL3EventInput(f)
                    )

    # ##############################
    # Suggestion by the WIMP group to
    # look at the RT cluster size of pulses
    # in the IC Veto region
    # ##############################

    RTVetoConfiguredSeededRTConfigService = I3DOMLinkSeededRTConfigurationService(
	        useDustlayerCorrection  = False,
	        dustlayerUpperZBoundary = 0*I3Units.m,
	        dustlayerLowerZBoundary = -150*I3Units.m,
	        ic_ic_RTTime           = 1000*I3Units.ns,
	        ic_ic_RTRadius         = 250*I3Units.m
	        )

    tray.AddModule( "I3StaticTWC<I3RecoPulseSeries>", "ExpTWRTVetoPulses",
                    InputResponse    = splituncleaned,
                    OutputResponse   = "TWRTVetoSeries",
                    TriggerConfigIDs = [1006,21001],
                    FirstTriggerOnly = True,
                    TriggerName      = "I3TriggerHierarchy",
                    WindowMinus      = 5000,
                    WindowPlus       = 0,
                    If =  lambda f: ExpFidL3EventInput(f) and MoreThanDCTrig(f)
                    )
    tray.AddModule( "I3StaticTWC<I3RecoPulseSeries>", "ExpTWRTVetoPulses_DCTrigOnly",
                    InputResponse    = splituncleaned,
                    OutputResponse   = "TWRTVetoSeries",
                    TriggerConfigIDs = [1011],
                    FirstTriggerOnly = True,
                    TriggerName      = "I3TriggerHierarchy",
                    WindowMinus      = 5000,
                    WindowPlus       = 0,
                    If =  lambda f: ExpFidL3EventInput(f) and not MoreThanDCTrig(f)
                    )

    tray.AddModule( "I3OMSelection<I3RecoPulseSeries>", "ExpGenRTVetoPulses",
                    selectInverse     = False,
                    InputResponse     = "TWRTVetoSeries",
                    OutputResponse    = "TWRTVetoSeries_ICTwoLayVeto",
                    OutputOMSelection = "BadOM3",
                    OmittedKeys       = DOMList.DeepCoreFiducialDOMs,
                    If =  lambda f: ExpFidL3EventInput(f)
                    )

    tray.AddModule("I3RTVeto_RecoPulseMask_Module", "expfidrtveto",
                   STConfigService         = RTVetoConfiguredSeededRTConfigService,
                   InputHitSeriesMapName   = "TWRTVetoSeries_ICTwoLayVeto",
                   OutputHitSeriesMapName  = "RTVetoSeries250",
                   Streams                 = [icetray.I3Frame.Physics],
                   If = lambda f: ExpFidL3EventInput(f)
                  )

    #tray.AddModule( "I3SeededRTHitCleaningModule<I3RecoPulse>", "ExpRTVeto250",
    #                InputResponse  = "TWRTVetoSeries_ICTwoLayVeto",
    #                OutputResponse = "RTVetoSeries250",
    #                Seeds          = "OneHitAutoSeed",
    #                RTRadius       = 250*I3Units.m,
    #                If =  lambda f: ExpFidL3EventInput(f)
    #                )

    def CountRTVetoSeriesNChannel( frame):
      #if If(frame):
        totalCharge = 0
        if frame.Has("RTVetoSeries250"):
            for omkey in frame["RTVetoSeries250"].apply(frame):
                for pulse in omkey[1]:
                    totalCharge += pulse.charge
                # end for()
            # end for()
        # end if()
        frame["RTVetoSeries250PE"] = dataclasses.I3Double( totalCharge)
        return
      #return
    # end CountRTVetoSeriesNChannel()


    tray.AddModule( CountRTVetoSeriesNChannel, "ExpCountRTVetoSeries",If =  lambda f: ExpFidL3EventInput(f))

    # ##############################
    # Check if vertex guess lies
    # within fiducial volume polygon
    # ##############################

    def VertexInsideEDC(frame):
	#    if If(frame):    
                ExpFidPoly = [(-173.0,-320.0),(247.0,-249.0),(379.0,137.0),(271.0,273.0),(31.0,240.0),(-53.0,322.0),(-215.0,295),(-360.0,-90.0)]
                InsideExpDCPolygon = point_inside_polygon(frame["VertexGuessX"].value,frame["VertexGuessY"].value,ExpFidPoly)
                frame.Put("VertexInsideExpDC",icetray.I3Bool(InsideExpDCPolygon))
                return
    tray.AddModule(VertexInsideEDC,'VGuessInExpDC',If=lambda f: ExpFidL3EventInput(f))

    # ##############################
    # Create a variable that gets pushed
    # to the frame that checks whether
    # the event passed the IC2012 LE L3
    # straight cuts and cleaning.
    # ##############################
    
    def passIC2012_ExpLE_L3(frame):
 #     if If(frame):
        IC2012_ExpLE_L3_FrameObjects = [ "DCFilterPulses_VetoPE",
                                      "MicroCountHits",
                                      "MicroCountPE",
                                      "NAbove200",
                                      "C2QR6",
                                      "RTVetoSeries250PE",
                                      "SRTTWOfflinePulsesExpDCFid",
                                      "SRTTWOfflinePulsesICTwoLayVeto",
                                      "VertexGuessZ",
                                      "VertexInsideExpDC"
                                      ]

        frameObjects = frame.keys()
        matchingFrameObjects =  set(IC2012_ExpLE_L3_FrameObjects).intersection( set(frameObjects))

        if len(matchingFrameObjects) == 10:

            # Count the number of SRT cleaned PEs in the DeepCore fiducial and
            # IceCube Veto region for use in the Ratio Cut variable. Also,
            # count the number of PE for hits satisying the DeepCore Filter
            # `speed of light' criteria.
            totalChargeFiducial = 0
            totalChargeVeto     = 0

            for omkey in frame["SRTTWOfflinePulsesExpDCFid"]:
                for pulse in omkey[1]:
                    totalChargeFiducial += pulse.charge
                # end for()
            # end for()
            for omkey in frame["SRTTWOfflinePulsesICTwoLayVeto"]:
                for pulse in omkey[1]:
                    totalChargeVeto += pulse.charge
                # end for()
            # end for()

            LE_L3_Vars = dataclasses.I3MapStringDouble()
            LE_L3_Vars["STW9000_DTW175PE"]   = frame["MicroCountPE"].get("STW9000_DTW175")
            LE_L3_Vars["STW9000_DTW200PE"]   = frame["MicroCountPE"].get("STW9000_DTW200")
            LE_L3_Vars["STW9000_DTW250PE"]   = frame["MicroCountPE"].get("STW9000_DTW250")
            LE_L3_Vars["STW9000_DTW300PE"]   = frame["MicroCountPE"].get("STW9000_DTW300")
            LE_L3_Vars["STW9000_DTW175Hits"] = frame["MicroCountHits"].get("STW9000_DTW175")
            LE_L3_Vars["STW9000_DTW200Hits"] = frame["MicroCountHits"].get("STW9000_DTW200")
            LE_L3_Vars["STW9000_DTW250Hits"] = frame["MicroCountHits"].get("STW9000_DTW250")
            LE_L3_Vars["STW9000_DTW300Hits"] = frame["MicroCountHits"].get("STW9000_DTW300")
            LE_L3_Vars["C2QR6"]              = frame["C2QR6"].value
            LE_L3_Vars["NAbove200PE"]        = frame["NAbove200"].value
            LE_L3_Vars["DCFiducialPE"]       = totalChargeFiducial
            LE_L3_Vars["ICVetoPE"]           = totalChargeVeto
            LE_L3_Vars["CausalVetoPE"]       = frame["DCFilterPulses_VetoPE"].value
            LE_L3_Vars["CausalVetoHits"]     = frame["DCFilterPulses_VetoHits"].value
            LE_L3_Vars["VertexGuessZ"]       = frame["VertexGuessZ"].value
            LE_L3_Vars["VertexGuessY"]       = frame["VertexGuessY"].value
            LE_L3_Vars["VertexGuessX"]       = frame["VertexGuessX"].value
            LE_L3_Vars["VertexInsideExpDC"]  = frame["VertexInsideExpDC"].value
            LE_L3_Vars["RTVeto250PE"]        = frame["RTVetoSeries250PE"].value
            LE_L3_Vars["NchCleaned"]         = len(frame["SRTTWOfflinePulsesExpDC"].apply(frame))

            frame["IC2012_ExpLE_L3_Vars"] = LE_L3_Vars

            if (frame["MicroCountHits"].get("STW9000_DTW300") > 2 and
		frame["MicroCountPE"].get("STW9000_DTW300") > 2.0 and
		frame["NAbove200"].value < 6.0 and
		frame["VertexGuessZ"].value < -200.0 and
		frame["DCFilterPulses_VetoPE"].value < 4.0 and
		frame["SRTTWOfflinePulsesExpDCFid"] > 0 and
		frame["RTVetoSeries250PE"].value < 3.0 and
		frame["VertexInsideExpDC"].value):

                  frame["IC2012_ExpLE_L3"] = icetray.I3Bool(True)
                  return
            # end if()
        # end if()
        frame["IC2012_ExpLE_L3"]  = icetray.I3Bool(False)
        return
    #  return
    # end def passIC2012_ExpLE_L3()

    tray.AddModule( passIC2012_ExpLE_L3, "LE_L3_pass_expfid",If =  lambda f: ExpFidL3EventInput(f))

    # ############################################
    # Remove all the bad omselections created when
    # producing the MicroCount outputs as well as
    # other junk from L3 processing
    # ############################################
  
    tray.AddModule("Delete", "delete_badomselection_expstream",
                   Keys = ["BadOMSelection",
                           "BadOM1",
                           "BadOM2",
                           "BadOM3",
                           "BadOM_DCFid_0",
                           "BadOM_ICVeto_0",
                           "DCFidPulses_DTW175",
                           "DCFidPulses_DTW200",
                           "DCFidPulses_DTW250",
                           "DCFidPulses_DTW300",
                           "SplitInIcePulses_STW_NoiseEnginess",
                           "SplitInIcePulses_STW_ClassicRT_NoiseEnginess",
                           "SRTTWOfflinePulsesICTwoLayVeto",
                           "SRTTWOfflinePulsesDCExpFid",
                           "DCFilterPulses_VetoPE",
                           "DCFilterPulses_VetoHits",
                           "DCFilterPulses",
                           "C2QR6",
                           "Baddies",
                           "RTVetoSeries250",
                           "RTVetoSeries250PE",                           
                           "TWRTVetoSeriesTimeRange",
                           "TWRTVetoSeries_ICTwoLayVeto",
                           "OfflinePulsesDCExpFid",
                           "OfflinePulsesICTwoLayVeto",
                           "VertexGuessX",
                           "VertexGuessY",
                           "VertexGuessZ",
			   "VertexInsideExpDC",
			   "OfflinePulsesExpDCFidCleanedKeys",
			   "OfflinePulsesExpDCFid",
			   "SRTTWOfflinePulsesExpDCFid",
			   "SRTTWOfflinePulsesExpDCFidCleanedKeys",
			   "TWOfflinePulsesExpDC",
			   "TWOfflinePulsesExpDCFid",
                           "TWOfflinePulsesDCExpFid",
			   "SPEFit2_EDCFitParams",
			   "SPEFitSingle_EDC",
			   "SPEFitSingle_EDCFitParams",
                           "NAbove200",
                           "MicroCountPE",
                           "MicroCountHits",
                           'TWOfflinePulsesExpDCFidCleanedKeys',
                           'SRTTWOfflinePulsesDCExpFidCleanedKeys',
                           'SRTTWOfflinePulsesICTwoLayVetoCleanedKeys',
                           'TWRTVetoSeries_ICTwoLayVetoCleanedKeys',
                           'OfflinePulsesDCExpFidCleanedKeys',
                           'OfflinePulsesICTwoLayVetoCleanedKeys',
                           ]
                   )

    return

@icetray.traysegment
def DCL3MasterSegment( tray, name, Filter=True, year='12'):

  tray.AddSegment( DeepCoreCuts, "L3DeepCoreCuts",splituncleaned='InIcePulses', year=year)
  tray.AddSegment( ExpFidDeepCoreCuts,'L3ExpDeepCoreCuts_2012',splituncleaned='InIcePulses',year=year)

  def Filterizer(frame,branch=1):
        passbool=False
        stdbranch=False
        expbranch=False
        nortbranch=False

        extra_event=False

        if frame.Has('IC2012_LE_L3'):
                stdbranch=frame['IC2012_LE_L3'].value
                #stdbranch=True
        if frame.Has('IC2012_LE_L3_No_RTVeto'):
                nortbranch=frame['IC2012_LE_L3_No_RTVeto'].value
        if frame.Has('IC2012_ExpLE_L3'):
                expbranch=frame['IC2012_ExpLE_L3'].value
                #expbranch = True
        if branch == 1:
                event=frame['I3EventHeader'].event_id
                if event==20209 or event==20923 or event==40699 or event==96753 or event==133793 or event==194914 or event==211350:
                    extra_event=True
                passbool = expbranch or stdbranch or nortbranch or extra_event
        if branch == 2:
                passbool = stdbranch
        if branch == 3:
                passbool = expbranch
        if branch == 4:
                passbool = nortbranch
        frame['LowEnergy_2012L3_Bool'] = icetray.I3Bool(passbool)
        return passbool

  if Filter:
    tray.AddModule(Filterizer,'FilterToL3')


