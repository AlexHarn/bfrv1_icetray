# written by Anna Pollmann <anna.pollmann@uni-wuppertal.de>
#
# This filter is supposed to select tracks with velocities
# 0.1c to 0.75c independent of their brightness

from icecube import icetray, dataclasses
import decimal


#############################################################################################

def round_ceiling(val):
    # for some reason this code used a function called ceil which was actually a round with ceiling round (ROUND_CEILING)
    # instead of bankers round (or ROUND_HALF_EVEN) as in the normal python round function
    # not sure if this was planned or if this was an accident, need to contact original author,
    # changing to a bankers round now would alter the physics of the filter as the usecase at the moment is
    # 0.5 * N
    # //FHL
    return decimal.Decimal.from_float(val).quantize(1, rounding=decimal.ROUND_UP)


@icetray.traysegment
def monopoleCV(tray, name,RecoPulses,ParticleName,
               tag="",pretag="",
               remove=[],
               If=lambda f: True
               ):

    from icecube.common_variables import hit_multiplicity
    from icecube.common_variables import track_characteristics
    from icecube.common_variables import time_characteristics

    hmv="HitMultiplicityValues"
    tcv="TrackCharacteristicsValues"
    tv="TimeCharacteristics"


    ##Common Variables

    tray.AddSegment(hit_multiplicity.I3HitMultiplicityCalculatorSegment, pretag +hmv+tag,
		     PulseSeriesMapName                = RecoPulses,
		     OutputI3HitMultiplicityValuesName = pretag+hmv+tag,
		     BookIt                            = False,
		     If = If,
		     )

    tray.AddSegment(track_characteristics.I3TrackCharacteristicsCalculatorSegment, pretag+tcv+tag,
            PulseSeriesMapName              = RecoPulses,
            OutputI3TrackCharacteristicsValuesName = pretag+tcv+tag,
            ParticleName                    = ParticleName,
            TrackCylinderRadius             = 100/icetray.I3Units.m,
            If = If,

	        )

    tray.AddModule(time_characteristics.I3TimeCharacteristicsCalculator, pretag+tv+tag,
            PulseSeriesMapName              = RecoPulses,
            OutputI3TimeCharacteristicsValuesName = pretag+tv+tag,
            If = If,
    )

    remove.extend([pretag+hmv+tag, pretag+tcv+tag, pretag+tv+tag])


#############################################################################################

class ChargeCleaning(icetray.I3Module):

    def __init__(self, context):
        icetray.I3Module.__init__(self, context)

        self.AddOutBox("OutBox")

        self.AddParameter("InputRecoPulses",
                          "",
                          "PulseSeriesReco")

        self.AddParameter("OutputRecoPulses",
                          "",
                          "HighChargePulseSeriesReco")

        self.AddParameter("ChargeFraction",
                          "",
                          "0.5")

        self.AddParameter("If", "", lambda f: True)

    def Configure(self):
        self.input = self.GetParameter("InputRecoPulses")
        self.out   = self.GetParameter("OutputRecoPulses")
        self.frac  = self.GetParameter("ChargeFraction")
        self.If    = self.GetParameter("If")

        if not 0 < self.frac <= 1:
           raise RuntimeError("Charge fraction must be between 0 and 1. Current value %f" %self.frac)

    # ------------------------------------------------------------------------------------

    def Physics(self, frame):

        if not self.If(frame):
            self.PushFrame(frame)
            return True

        ### Get the input data
        if self.input in frame:
            pulsemap = frame[self.input]
            if isinstance(pulsemap, dataclasses.I3RecoPulseSeriesMapMask):
                pulsemap = pulsemap.apply(frame)
        else:
            self.PushFrame(frame)
            return True

        # Order Doms by charge (charge is the amount of photons reaching one dom
        # Select the highest charged doms

        ### Get total charge for each DOM
        domCharges = []
        for entry in pulsemap:
            charge = 0
            for pulse in entry.data():
                charge += pulse.charge
            if charge != 0: # obi: avoid charge=0 -> Feature extractore couldn't reconstruct the puls from waveform
                domCharges.append([entry.key(),charge])

        ### Sort list of DOMs by charge
        domCharges.sort(key=lambda item: item[1],reverse=True)

        ### Get DOMs with hightest integrated charge
        keylist = []
        nSelect = max( int(round_ceiling(self.frac*len(domCharges))), min(2,len(domCharges)) )
        for i in range(nSelect):
           keylist.append(domCharges[i][0])

        ### Make a new RecoPulseSeriesMap
        NewPulseMap = dataclasses.I3RecoPulseSeriesMap()
        nhits=0
        for entry in keylist:

            ### this is needed to process the full RecoPulseSeriesMap
            if len(pulsemap[entry]) == 0:
                continue

            # use only the first pulse!
            NewPulseVector = dataclasses.I3RecoPulseSeries()
            NewPulseVector.append(pulsemap[entry][0])
            NewPulseMap[entry] = NewPulseVector
            nhits+=1

        frame["MM_DC_nHits"] = dataclasses.I3Double(nhits)
        frame[self.out] = NewPulseMap
        self.PushFrame(frame)
        return True

#############################################################################################


def checkIfPulsesInFrame(frame, name):
    # this looks like something added as a bug fix
    # I have no idea why there should be pulsemaps with no charge at all //FHL
    if name in frame:
        pulsemap=frame[name]
        if isinstance(pulsemap, dataclasses.I3RecoPulseSeriesMapMask):
            pulsemap = pulsemap.apply(frame)
        for entry in pulsemap:
            for pulse in entry.data():
                if pulse.charge !=0:
                    return True
    return False

#############################################################################################

def mpfilter(frame, softcuts=True):
    from icecube.filterscripts import filter_globals

    pretagIC="MM_IC_"
    pretagDC="MM_DC"
    DCPulses=pretagDC+"_Pulses"
    SelectedDCPulses=DCPulses+"_First"+"_Charge_0_5"

    hmv="HitMultiplicityValues"
    tcv="TrackCharacteristicsValues"
    tv="TimeCharacteristics"

    # soft cuts = larger passing rate
    if softcuts:
        ICndomvalue=6
        ICspeedvalue=0.8
        IClength=250
        ICgap=200
        ICtime=4000
        #
        DCndomvalue=6
        DCspeedvalue=0.7
        DCgap=float('inf')
        DCTime=2750
        DCfwhm=2500
    # hard cuts = smaller passing rate
    else:
        ICndomvalue=6
        ICspeedvalue=0.8
        IClength=400
        ICgap=200
        ICtime=5000
        #
        DCndomvalue=6
        DCspeedvalue=0.6
        DCgap=100
        DCTime=3000
        DCfwhm=2500

    # IC Filter
    icetray.logging.log_debug(
        'IC Frames found: {0} {1} {2} {3}'.format((pretagIC + hmv) in frame, (pretagIC + "LineFitI") in frame,
                                                  (pretagIC + tcv) in frame, (pretagIC + tv) in frame))

    #caching the frame set because we need to use if for the DC part as well
    cachedoriginalframekeysset = set(frame.keys())
    # check that everything we need is in frame, utilizing issubset
    ICKeep = False
    if {pretagIC + hmv,
        pretagIC + "LineFitI",
        pretagIC + tcv,
        pretagIC + tv} <= cachedoriginalframekeysset:

        n_doms=frame[pretagIC+hmv].n_hit_doms

        lf=frame[pretagIC+"LineFitI"]
        status=lf.fit_status
        speed=lf.speed / dataclasses.I3Constants.c

        tc=frame[pretagIC+tcv]
        lengths=tc.track_hits_separation_length
        gap=tc.empty_hits_track_length

        time=frame[pretagIC+tv].timelength_last_first

        #this was only for debugging purposes
        #if verbose:
        #    decisionPairs=[
        #        [ "ndom", n_doms, ">",ICndomvalue],
        #        [ "status", status, "==",0],
        #        [ "speed", speed, ">",0.0],
        #        [ "speed", speed, "<",ICspeedvalue],
        #        [ "lengths", lengths, ">",IClength],
        #        [ "gap", gap, "<",ICgap],
        #        [ "time", time, ">",ICtime],
        #        ]
        #    for name, var, sign, thre in decisionPairs:
        #        keep=False
        #        if (sign==">" and var > thre) or \
        #                (sign=="<" and var < thre) or \
        #                ((sign=="==" or sign=="=") and var == thre) or \
        #                (sign=="!=" and var != thre):
        #            keep=True
        #        print("Dec: %6s %10s %10.2f %10.2f " % (str(keep), name, var, thre))

        ICKeep = (n_doms > ICndomvalue) and \
                (status == 0) and \
                (speed > 0.0 ) and \
                (speed < ICspeedvalue ) and \
                (lengths < -IClength or lengths > IClength ) and \
                (gap < ICgap ) and \
                (time > ICtime )

    frame[filter_globals.MonopoleFilter+"_IC"] = icetray.I3Bool(ICKeep)

    # DC Filter
    icetray.logging.log_debug(
        'DC Frames found: {0} {1} {2} {3}'.format((pretagDC + hmv + SelectedDCPulses) in frame,
                                                  (pretagDC + "LineFitI_" + SelectedDCPulses) in frame,
                                                  (pretagDC + tv + SelectedDCPulses) in frame,
                                                  (pretagDC + tcv + SelectedDCPulses) in frame))
    DCKeep=False
    # check that everything we need is in frame, utilizing issubset
    if {pretagDC + hmv + SelectedDCPulses,
        pretagDC + "LineFitI_" + SelectedDCPulses,
        pretagDC + tv + SelectedDCPulses,
        pretagDC + tcv + SelectedDCPulses} <= cachedoriginalframekeysset:

        n_doms=frame[pretagDC+hmv+SelectedDCPulses].n_hit_doms

        lf=frame[pretagDC+"LineFitI_"+SelectedDCPulses]
        status=lf.fit_status
        speed=lf.speed / dataclasses.I3Constants.c

        t=frame[pretagDC+tv+SelectedDCPulses]
        time=t.timelength_last_first
        fwhm=t.timelength_fwhm

        gap=frame[pretagDC+tcv+SelectedDCPulses].empty_hits_track_length

        #this was only for debugging purposes
        #if verbose:
        #    decisionPairs=[
        #        [ "ndom", n_doms, ">",DCndomvalue],
        #        [ "status", status, "==",0],
        #        [ "speed", speed, ">",0.0],
        #        [ "speed", speed, "<",DCspeedvalue],
        #        [ "gap", gap, "<",DCgap],
        #        [ "time", time, ">",DCTime],
        #        [ "fwhm", fwhm, ">",DCfwhm],
        #        ]
        #    for name, var, sign, thre in decisionPairs:
        #        keep=False
        #        if (sign==">" and var > thre) or \
        #                (sign=="<" and var < thre) or \
        #                ((sign=="==" or sign=="=") and var == thre) or \
        #                (sign=="!=" and var != thre):
        #            keep=True
        #        print("Dec: %s %10s %10.2f %10.2f " % (str(keep), name, var, thre))

        DCKeep = (n_doms > DCndomvalue) and \
                (status == 0) and \
                (speed > 0.0 ) and \
                (speed < DCspeedvalue ) and \
                (time > DCTime ) and \
                (gap < DCgap ) and \
                (fwhm > DCfwhm)

    frame[filter_globals.MonopoleFilter+"_DC"] = icetray.I3Bool(DCKeep)

    # decision
    MMFilter = ICKeep or DCKeep
    frame[filter_globals.MonopoleFilter+"_key"] = icetray.I3Bool(MMFilter)
    return True

#############################################################################################

@icetray.traysegment
def MonopoleFilter(tray, name,
                   # this is split by topo.splitter ("InIceSplit") but not cleaned, only non-split ("NullSplit") filter is the slop filter
                   pulses= "SplitUncleanedInIcePulses",
                   seededRTConfig = "",
                   keepFrames=False,
                   If=lambda f: True):

    from icecube.filterscripts import filter_globals
    icetray.load("filterscripts",False)
    from icecube import dataclasses
    from icecube.DeepCore_Filter import DOMS
    from icecube import linefit

    from icecube.common_variables import hit_multiplicity
    from icecube.common_variables import track_characteristics
    from icecube.common_variables import time_characteristics


    hmv="HitMultiplicityValues"
    tcv="TrackCharacteristicsValues"
    tv="TimeCharacteristicsValues"

    domlist = DOMS.DOMS("IC86") # only one layer of IC DOMs around DC
    # DC strings: 26, 27, 35, 36, 37, 45, 46, 79, 80, 81, 82, 83, 84, 85, 86
    # IC strings: all but 79, 80, 81, 82, 83, 84, 85, 86

    pretagIC="MM_IC_"
    pretagDC="MM_DC"
    CleanedPulses='MM_Cleaned_'+pulses
    ICPulses=pretagIC+'Pulses'
    DCPulses=pretagDC+"_Pulses"
    SelectedDCPulses=DCPulses+"_First"+"_Charge_0_5"

    remove=[]
    # ------------------------------------------------------------------------------------

    # split into DC and IC sample

    # taken from DC filter: keeps much more hits of luminescence tracks!
    # Perform SeededRT using HLC instead of HLCCore.
    tray.AddModule('I3SeededRTCleaning_RecoPulseMask_Module', name + 'SeededRT',
                   InputHitSeriesMapName  = pulses,
                   OutputHitSeriesMapName = CleanedPulses,
                   STConfigService        = seededRTConfig,
                   SeedProcedure          = 'AllHLCHits',
                   MaxNIterations         = -1,
                   Streams                = [icetray.I3Frame.Physics],
                   If = If
    )

    remove.append(CleanedPulses)

    # no time window cleaning to get more than 6000 ns

    # ------------------------------------------------------------------------------------
    # IceCube

    tray.AddModule("I3OMSelection<I3RecoPulseSeries>",name+'selectICDOMS',
                   OmittedStrings = list(domlist.exclusiveIceCubeStrings),
                   OutputOMSelection = pretagIC+'Selection',
                   InputResponse = CleanedPulses,
                   OutputResponse = ICPulses,
                   SelectInverse = True,
                   If = If
                   )

    tray.AddSegment(linefit.simple, name + "_imprv_LF",
		     inputResponse= ICPulses,
		     fitName = pretagIC+"LineFitI",
		     If = If
		     )

    monopoleCV(tray,"CV_IC",
               RecoPulses=ICPulses, # same as in filter
               ParticleName= pretagIC+"LineFitI",
               tag="",
               pretag=pretagIC,
               remove=remove,
               If =  lambda f: If(f) and checkIfPulsesInFrame(f, ICPulses)
               )

    remove.extend([pretagIC+"Selection", ICPulses,
                   pretagIC+"LineFitI", ICPulses+"CleanedKeys"
                   ])
    # ------------------------------------------------------------------------------------
    # DeepCore

    tray.AddModule("I3OMSelection<I3RecoPulseSeries>",name+'selectDCDOMs',
                   OmittedKeys= domlist.DeepCoreFiducialDOMs,
                   SelectInverse = True,
                   InputResponse = CleanedPulses,
                   OutputResponse = DCPulses,
                   OutputOMSelection = pretagDC+'Selection',
                   If = If
                   )

    tray.AddModule(ChargeCleaning, name+"ChargeCleaning_First_05",
                   InputRecoPulses  = DCPulses,
                   OutputRecoPulses = SelectedDCPulses,
                   ChargeFraction   = 0.5,
                   If = lambda f: If(f) and DCPulses in f
                  )

    tray.AddSegment(linefit.simple, name + "_imprv_LFDC"+SelectedDCPulses,
         inputResponse= SelectedDCPulses,
         fitName = pretagDC+"LineFitI_"+SelectedDCPulses,
         If = lambda f: If(f) and DCPulses in f
         )

    monopoleCV(tray,"CV_DC",
        RecoPulses=SelectedDCPulses, # same as in filter
        ParticleName= pretagDC+"LineFitI_"+SelectedDCPulses,
        tag=SelectedDCPulses,
        pretag=pretagDC,
        If = lambda f: If(f) and \
        "MM_DC_nHits" in f and \
        f["MM_DC_nHits"]>0 and \
        (pretagDC+"LineFitI_"+SelectedDCPulses) in f
        )

    remove.extend([DCPulses, pretagDC+"Selection", DCPulses+"_First", SelectedDCPulses,
                   pretagDC+"LineFitI_"+SelectedDCPulses, DCPulses+"CleanedKeys", "MM_DC_nHits"])


    # ------------------------------------------------------------------------------------
    # Filter and end

    tray.AddModule(mpfilter, name+"_filter",
                   If = If
                   )

    tray.AddModule("I3FilterModule<I3BoolFilter>",name + "_MonopoleFilter",
                   Boolkey = filter_globals.MonopoleFilter+"_key",
                   DecisionName = filter_globals.MonopoleFilter,
                   DiscardEvents = False,
                   If = If,
                   )

    remove.extend([filter_globals.MonopoleFilter+"_DC",
                   filter_globals.MonopoleFilter+"_IC",
                   filter_globals.MonopoleFilter+"_key"
                   ])

    if not keepFrames:
        tray.AddModule("Delete",'MM_leftOvers',
                       Keys=remove,
                       If = If,
                       )

