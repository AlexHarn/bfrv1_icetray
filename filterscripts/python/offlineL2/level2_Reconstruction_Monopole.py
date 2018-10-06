# written by Anna Pollmann <anna.pollmann@uni-wuppertal.de>
#
# This filter is supposed to select tracks with velocities
# 0.1c to 0.75c independent of their brightness

import I3Tray
from icecube import icetray, dataclasses
import decimal

def round_ceiling(val):
    # for some reason this code used a function called ceil which was actually a round with ceiling round (ROUND_CEILING)
    # instead of bankers round (or ROUND_HALF_EVEN) as in the normal python round function
    # not sure if this was planned or if this was an accident, need to contact original author,
    # changing to a bankers round now would alter the physics of the filter as the usecase at the moment is
    # 0.5 * N
    # //FHL
    return decimal.Decimal.from_float(val).quantize(1, rounding=decimal.ROUND_UP)

@icetray.traysegment
def CV(tray, name,RecoPulses,ParticleName,
        tag="",pretag="",
        If=lambda f: True
      ):

    from icecube.common_variables import hit_multiplicity
    from icecube.common_variables import track_characteristics
    from icecube.common_variables import time_characteristics

    hmv="HitMultiplicityValues"
    tcv="TrackCharacteristicsValues"
    tv="TimeCharacteristicsValues"

    ##Common Variables

    tray.AddSegment(hit_multiplicity.I3HitMultiplicityCalculatorSegment, name+'_'+pretag +hmv+tag,
		     PulseSeriesMapName                = RecoPulses,
		     OutputI3HitMultiplicityValuesName = pretag+hmv+tag,
		     BookIt                            = False,
		     If = If,
		     )

    tray.AddSegment(track_characteristics.I3TrackCharacteristicsCalculatorSegment, name+'_'+pretag+tcv+tag,
            PulseSeriesMapName              = RecoPulses,
            OutputI3TrackCharacteristicsValuesName = pretag+tcv+tag,
            ParticleName                    = ParticleName,
            TrackCylinderRadius             = 100/icetray.I3Units.m,
            If = If,

	        )

    tray.AddModule(time_characteristics.I3TimeCharacteristicsCalculator, name+'_'+pretag+tv+tag,
            PulseSeriesMapName              = RecoPulses,
            OutputI3TimeCharacteristicsValuesName = pretag+tv+tag,
            If = If,
    )



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

        if self.frac <= 0 or self.frac > 1:
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
        domCharges.sort(key=lambda item: item[1],reverse=True)#print domCharges

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

def checkFilterMask(frame, verbose=False):
        if "FilterMask" in frame:
            if frame["FilterMask"]["MonopoleFilter_16"].condition_passed:
                    return True
        return False

#############################################################################################

def checkIfInFrame(frame, name, threshold=None, verbose=False):
        if name in frame:
            if threshold==None:
                return True
            else:
                if frame[name] > threshold:
                    return True
        return False

#############################################################################################

def mpfilter(frame, verbose=False, softcuts=True):
    from icecube.filterscripts import filter_globals

    pretagIC="MM_IC_"
    pretagDC="MM_DC_"
    DCPulses=pretagDC+"Pulses"
    SelectedDCPulses=DCPulses+"_1P"+"_C05"

    hmv="HitMultiplicityValues"
    tcv="TrackCharacteristicsValues"
    tv="TimeCharacteristicsValues"

    # soft cuts = larger passing rate
    if softcuts:
        #print("Using soft selection")
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
        #print("Using hard selection")
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
    if verbose: print("IC Frames found:", (pretagIC+hmv) in frame, \
                                          (pretagIC+"LineFitI") in frame, \
                                          (pretagIC+tcv) in frame, \
                                          (pretagIC+tv) in frame)
    ICKeep=False
    if (pretagIC+hmv) in frame and (pretagIC+"LineFitI") in frame and \
            (pretagIC+tcv) in frame and (pretagIC+tv) in frame:

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

        if (n_doms > ICndomvalue) and \
                (status == 0) and \
                (speed > 0.0 ) and \
                (speed < ICspeedvalue ) and \
                (lengths < -IClength or lengths > IClength ) and \
                (gap < ICgap ) and \
                (time > ICtime ):
            ICKeep=True
    frame[filter_globals.MonopoleFilter+"_IC"] = icetray.I3Bool(ICKeep)

    # DC Filter
    if verbose: print("DC Frames found:", (pretagDC+hmv+SelectedDCPulses) in frame, (pretagDC+"LineFitI_"+SelectedDCPulses) in frame, (pretagDC+tv+SelectedDCPulses) in frame, (pretagDC+tcv+SelectedDCPulses) in frame)
    DCKeep=False
    if (pretagDC+hmv+SelectedDCPulses) in frame and (pretagDC+"LineFitI_"+SelectedDCPulses) in frame and \
            (pretagDC+tv+SelectedDCPulses) in frame and (pretagDC+tcv+SelectedDCPulses) in frame:

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


        if (n_doms > DCndomvalue) and \
                (status == 0) and \
                (speed > 0.0 ) and \
                (speed < DCspeedvalue ) and \
                (time > DCTime ) and \
                (gap < DCgap ) and \
                (fwhm > DCfwhm):
            DCKeep=True

    frame[filter_globals.MonopoleFilter+"_DC"] = icetray.I3Bool(DCKeep)

    if ICKeep or DCKeep:
        MMFilter=icetray.I3Bool(True)
    else:
        MMFilter=icetray.I3Bool(False)

    frame[filter_globals.MonopoleFilter+"_key"] = MMFilter

    return True


#############################################################################################

@icetray.traysegment
def MonopoleL2(tray, name,
                pulses= "SplitInIcePulses",
                seededRTConfig = "",
                If = lambda f: True
                ):



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

    C=0.299792458

    pretagIC="MM_IC_"
    pretagDC="MM_DC_"
    CleanedPulses='MM_Cleaned_'+pulses
    ICPulses=pretagIC+'Pulses'
    DCPulses=pretagDC+"Pulses"
    SelectedDCPulses=DCPulses+"_1P"+"_C05"




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
                   If = lambda f: If(f) and checkFilterMask(f)
    )


    # no time window cleaning to get more than 6000 ns

    # ------------------------------------------------------------------------------------
    # IceCube

    tray.AddModule("I3OMSelection<I3RecoPulseSeries>",name+'selectICDOMS',
                   OmittedStrings = list(domlist.exclusiveIceCubeStrings),
                   OutputOMSelection = pretagIC+'Selection',
                   InputResponse = CleanedPulses,
                   OutputResponse = ICPulses,
                   SelectInverse = True,
                   If = lambda f: If(f) and checkFilterMask(f)
                   )

    tray.AddSegment(linefit.simple, name + "_imprv_LF",
		     inputResponse= ICPulses,
		     fitName = pretagIC+"LineFitI",
		     If = lambda f: If(f) and checkFilterMask(f)
		     )

    CV(tray,"CV_IC",
            RecoPulses=ICPulses, # same as in filter
            ParticleName= pretagIC+"LineFitI",
            tag="",
            pretag=pretagIC,
            If = lambda f: If(f) and checkFilterMask(f)
            )

    # ------------------------------------------------------------------------------------
    # DeepCore

    tray.AddModule("I3OMSelection<I3RecoPulseSeries>",name+'selectDCDOMs',
                   OmittedKeys= domlist.DeepCoreFiducialDOMs,
                   SelectInverse = True,
                   InputResponse = CleanedPulses,
                   OutputResponse = DCPulses,
                   OutputOMSelection = pretagDC+'Selection',
                   If = lambda f: If(f) and checkFilterMask(f)
                   )

    tray.AddModule(ChargeCleaning, name+"ChargeCleaning_First_05",
                   InputRecoPulses  = DCPulses,
                   OutputRecoPulses = SelectedDCPulses,
                   ChargeFraction   = 0.5,
                   If = lambda f: If(f) and checkIfInFrame(f, DCPulses) and checkFilterMask(f)
                  )

    tray.AddSegment(linefit.simple, name + "_imprv_LFDC"+SelectedDCPulses,
         inputResponse= SelectedDCPulses,
         fitName = pretagDC+"LineFitI_"+SelectedDCPulses,
         If = lambda f: If(f) and checkIfInFrame(f, DCPulses) and checkFilterMask(f)
         )

    CV(tray,"CV_DC",
        RecoPulses=SelectedDCPulses, # same as in filter
        ParticleName= pretagDC+"LineFitI_"+SelectedDCPulses,
        tag=SelectedDCPulses,
        pretag=pretagDC,
        If = lambda f: If(f) and \
        checkFilterMask(f) and \
        checkIfInFrame(f, "MM_DC_nHits" , threshold=0) and \
        checkIfInFrame(f, pretagDC+"LineFitI_"+SelectedDCPulses)
        )

    # ------------------------------------------------------------------------------------
    # restore information which filter selection was fulfilled: IC or DC
    tray.AddModule(mpfilter, name+"_filter",
                   If = lambda f: If(f) and checkFilterMask(f)
                   )
