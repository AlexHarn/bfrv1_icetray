# written by Anna Pollmann <anna.pollmann@uni-wuppertal.de>
#
# This filter is supposed to select tracks with velocities
# 0.1c to 0.75c independent of their brightness

from icecube import icetray, dataclasses
import decimal

#############################################################################################
#Const definitions
HMV = "HitMultiplicityValues"
TCV = "TrackCharacteristicsValues"
TV = "TimeCharacteristicsValues"
LINEFIT = "LineFitI"

#use different pretags for filter and offline -> you can run both at the same time and compare output easy //FHL
PRETAGLEVEL = "MM_F_" #FILTER
#PRETAGLEVEL = "MM_"  #OFFLINE LV2
PRETAGIC = PRETAGLEVEL + "IC_"
PRETAGDC = PRETAGLEVEL + "DC_"

ICPULSES = PRETAGIC + "Pulses"
DCPULSES = PRETAGDC + "Pulses"
POSTTAGSELECTEDPULSES = "_1P_C05"
DCSELECTEDPULSES = DCPULSES + POSTTAGSELECTEDPULSES
DC_NHITS = PRETAGDC + "nHits"
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

    ##Common Variables
    tray.AddSegment(hit_multiplicity.I3HitMultiplicityCalculatorSegment, name+'_'+pretag +HMV+tag,
		     PulseSeriesMapName                = RecoPulses,
		     OutputI3HitMultiplicityValuesName = pretag+HMV+tag,
		     If = If,
		     )

    tray.AddSegment(track_characteristics.I3TrackCharacteristicsCalculatorSegment, name+'_'+pretag+TCV+tag,
            PulseSeriesMapName              = RecoPulses,
            OutputI3TrackCharacteristicsValuesName = pretag+TCV+tag,
            ParticleName                    = ParticleName,
            TrackCylinderRadius             = 100/icetray.I3Units.m,
            If = If,

	        )

    tray.AddModule(time_characteristics.I3TimeCharacteristicsCalculator, name+'_'+pretag+TV+tag,
            PulseSeriesMapName              = RecoPulses,
            OutputI3TimeCharacteristicsValuesName = pretag+TV+tag,
            If = If,
    )

    remove.extend([pretag+HMV+tag, pretag+TCV+tag, pretag+TV+tag])


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
                          "Minimal fraction of all DOMs selected for OutputRecoPulses",
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

        # Sort list of DOMs by charge
        domCharges.sort(key=lambda item: item[1], reverse=True)
        # Calculate the number of DOMs to be copied into the new map
        nSelect = max(int(round_ceiling(self.frac * len(domCharges))), min(2, len(domCharges)))

        # list of DOMs where we want to pick the first pulse
        selectedDOMs = [omkey for omkey, charge in domCharges[:nSelect] if not len(pulsemap[omkey]) == 0]
        # not sure why we need the check for the number of pulses for the dom, it was in the old code with a
        # comment that this is needed to process the whole pulsemap, if think it is not required but will leave it here
        # //FHL

        # Mask of first Pulse of the selected DOMs
        frame[self.out] = dataclasses.I3RecoPulseSeriesMapMask(frame, self.input, lambda omkey, index, pulse:
                                                               index == 0 and omkey in selectedDOMs
                                                               )

        frame[DC_NHITS] = dataclasses.I3Double(len(selectedDOMs))
        self.PushFrame(frame)
        return True

#############################################################################################


def checkIfPulsesInFrame(frame, name):
    # this looks like something added as a bug fix
    # I have no idea why there should be pulsemaps with no charge at all //FHL
    # the more I think about is the less sense this makes, why do we not need to check the same
    # thing for the DC part?
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
        'IC Frames found: {0} {1} {2} {3}'.format((PRETAGIC + HMV) in frame, (PRETAGIC + LINEFIT) in frame,
                                                  (PRETAGIC + TCV) in frame, (PRETAGIC + TV) in frame))

    #caching the frame set because we need to use if for the DC part as well
    cachedoriginalframekeysset = set(frame.keys())
    # check that everything we need is in frame, utilizing issubset
    ICKeep = False
    if {PRETAGIC + HMV,
        PRETAGIC + LINEFIT,
        PRETAGIC + TCV,
        PRETAGIC + TV} <= cachedoriginalframekeysset:

        n_doms=frame[PRETAGIC+HMV].n_hit_doms

        lf=frame[PRETAGIC+LINEFIT]
        status=lf.fit_status
        speed=lf.speed / dataclasses.I3Constants.c

        tc=frame[PRETAGIC+TCV]
        lengths=tc.track_hits_separation_length
        gap=tc.empty_hits_track_length

        time=frame[PRETAGIC+TV].timelength_last_first

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
        'DC Frames found: {0} {1} {2} {3}'.format((PRETAGDC + HMV + DCSELECTEDPULSES) in frame,
                                                  (PRETAGDC + LINEFIT + "_" + DCSELECTEDPULSES) in frame,
                                                  (PRETAGDC + TV + DCSELECTEDPULSES) in frame,
                                                  (PRETAGDC + TCV + DCSELECTEDPULSES) in frame))
    DCKeep=False
    # check that everything we need is in frame, utilizing issubset
    if {PRETAGDC + HMV + DCSELECTEDPULSES,
        PRETAGDC + LINEFIT + "_" + DCSELECTEDPULSES,
        PRETAGDC + TV + DCSELECTEDPULSES,
        PRETAGDC + TCV + DCSELECTEDPULSES} <= cachedoriginalframekeysset:

        n_doms=frame[PRETAGDC+HMV+DCSELECTEDPULSES].n_hit_doms

        lf=frame[PRETAGDC+ LINEFIT + "_" +DCSELECTEDPULSES]
        status=lf.fit_status
        speed=lf.speed / dataclasses.I3Constants.c

        t=frame[PRETAGDC+TV+DCSELECTEDPULSES]
        time=t.timelength_last_first
        fwhm=t.timelength_fwhm

        gap=frame[PRETAGDC+TCV+DCSELECTEDPULSES].empty_hits_track_length

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
    # Filter is split into two parts, IceCube (IC) and DeepCore (DC)

    from icecube.filterscripts import filter_globals
    icetray.load("filterscripts",False)
    from icecube import dataclasses, linefit
    from icecube.DeepCore_Filter import DOMS

    domlist = DOMS.DOMS("IC86") # only one layer of IC DOMs around DC
    # DC strings: 26, 27, 35, 36, 37, 45, 46, 79, 80, 81, 82, 83, 84, 85, 86
    # IC strings: all but 79, 80, 81, 82, 83, 84, 85, 86

    CleanedPulses=PRETAGLEVEL + '_Cleaned_'+pulses

    remove=[]
    # ------------------------------------------------------------------------------------

    # taken from DC filter: keeps much more hits of luminescence tracks!
    # Perform SeededRT using HLC instead of HLCCore.
    tray.AddModule('I3SeededRTCleaning_RecoPulseMask_Module', name + '_SeededRT',
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

    tray.AddModule("I3OMSelection<I3RecoPulseSeries>",name+'_selectICDOMS',
                   OmittedStrings = domlist.exclusiveIceCubeStrings,
                   OutputOMSelection = PRETAGIC+'Selection',
                   InputResponse = CleanedPulses,
                   OutputResponse = ICPULSES,
                   SelectInverse = True,
                   If = If
                   )

    tray.AddSegment(linefit.simple, name + "_imprv_LF",
		     inputResponse= ICPULSES,
		     fitName = PRETAGIC+LINEFIT,
		     If = If
		     )

    monopoleCV(tray, name + "_CV_IC",
               RecoPulses=ICPULSES, # same as in filter
               ParticleName= PRETAGIC+LINEFIT,
               tag="",
               pretag=PRETAGIC,
               remove=remove,
               If =  lambda f: If(f) and checkIfPulsesInFrame(f, ICPULSES)
               )

    remove.extend([PRETAGIC+"Selection", ICPULSES,
                   PRETAGIC+LINEFIT])
    # ------------------------------------------------------------------------------------
    # DeepCore

    tray.AddModule("I3OMSelection<I3RecoPulseSeries>",name+'_selectDCDOMs',
                   OmittedKeys= domlist.DeepCoreFiducialDOMs,
                   SelectInverse = True,
                   InputResponse = CleanedPulses,
                   OutputResponse = DCPULSES,
                   OutputOMSelection = PRETAGDC+'Selection',
                   If = If
                   )

    tray.AddModule(ChargeCleaning, name+"_ChargeCleaning" + POSTTAGSELECTEDPULSES,
                   InputRecoPulses  = DCPULSES,
                   OutputRecoPulses = DCSELECTEDPULSES,
                   ChargeFraction   = 0.5,
                   If = lambda f: If(f) and DCPULSES in f
                  )

    tray.AddSegment(linefit.simple, name + "_imprv_LFDC_"+DCSELECTEDPULSES,
         inputResponse= DCSELECTEDPULSES,
         fitName = PRETAGDC+ LINEFIT + "_" + DCSELECTEDPULSES,
         If = lambda f: If(f) and DCPULSES in f #I think this is a bug, should check for DCSELECTEDPULSES //FHL
         )

    monopoleCV(tray, name + "_CV_DC",
        RecoPulses=DCSELECTEDPULSES, # same as in filter
        ParticleName= PRETAGDC+ LINEFIT + "_" + DCSELECTEDPULSES,
        tag=DCSELECTEDPULSES,
        pretag=PRETAGDC,
        If = lambda f: If(f) and \
        DC_NHITS in f and \
        f[DC_NHITS]>0 and \
        (PRETAGDC+ LINEFIT + "_" + DCSELECTEDPULSES) in f
        )

    remove.extend([DCPULSES, PRETAGDC+"Selection", DCSELECTEDPULSES,
                   PRETAGDC+ LINEFIT + "_" + DCSELECTEDPULSES, DC_NHITS])


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

