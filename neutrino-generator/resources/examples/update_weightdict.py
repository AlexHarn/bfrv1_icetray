#########################################################
#
# K.Hoshina (hoshina@icecuve.wisc.edu)
#
# This module updates weightdict names and corrects 
# direction weight for older production (i.e. IC79)
#

from icecube import icetray, dataclasses 
import copy
import math
import random 
import sys

#icetray.I3Logger.global_logger = icetray.I3PrintfLogger()
#icetray.set_log_level(icetray.I3LogLevel.LOG_WARN)

class update_weightdict(icetray.I3ConditionalModule):

    def __init__(self, context):

        icetray.I3ConditionalModule.__init__(self, context)

        self.AddParameter( 'WeightDictName',
                           'Name of WeightDict',
                           'I3MCWeightDict')

        self.AddParameter( 'ZenithWeightParam',
                           'Parameter of ZenithWeight',
                           '1.0')

        self.AddParameter( 'ProductionTag',
                           'Production Tag : IC79, IC86',
                           'IC79')

        self.AddOutBox("OutBox")

        self.dictname = "I3MCWeightDict"


    def Configure(self):

        self.weightdictname = self.GetParameter('WeightDictName')
        self.zenithweight = float(self.GetParameter('ZenithWeightParam'))
        self.productiontag= self.GetParameter('ProductionTag')

    def CalcSimpleSlopeWeight(self, min, max, zen) :
        if self.zenithweight == 1.0 :
            return 1.0

        x = zen - min
        l = max - min
        yl = (2 - 2*self.zenithweight)/l * x + self.zenithweight;
        return 1.0 / yl

    def FixIC79(self, frame):

        if not frame.Has(self.dictname) :
            self.PushFrame(frame,"OutBox");
            return True

        weights = copy.copy(frame.Get( self.dictname) )

        # get primary particle
        mctree = frame["I3MCTree"]
        primaries = mctree.get_primaries()

        if len(primaries) > 1 :
            log_warn("your frame contains multiple neutrinos! TotalWeight may be nonsense.")

        prim_nu = primaries[0]
        prim_e  = prim_nu.energy
        prim_zen = prim_nu.dir.zenith
        prim_coszen = math.cos(prim_zen)
        prim_type = prim_nu.type

        if not 'PrimaryNeutrinoType' in weights :
            weights["PrimaryNeutrinoType"] = prim_type
        
        mincoszen = math.cos(weights["MaxZenith"])
        maxcoszen = math.cos(weights["MinZenith"])

        # calc zenith weight
        zenithw = self.CalcSimpleSlopeWeight(mincoszen, maxcoszen, prim_coszen)

        # get propagation weight, which is convoluted value of zenith weight and propagation weight for IC79 gen.
        oldpropw = weights["TotalPropagationProbability"]
        propw = oldpropw / zenithw
        
        # update propagation weight
        weights["PropagationWeight"] = propw
        weights["DirectionWeight"] = zenithw

        # rename other weights
        weights["PrimaryNeutrinoZenith"] = prim_zen
        weights["InjectionAreaCGS"] = weights["InjectionSurfaceR"]**2 * math.pi * 10000 # m2 to cm2
        weights["SolidAngle"] = (maxcoszen - mincoszen) * (weights["MaxAzimuth"] - weights["MinAzimuth"])

        if 'TotalInteractionProbabilityWeight' in weights :
            weights["TotalWeight"] = weights["TotalInteractionProbabilityWeight"]
            weights["InteractionWeight"] = weights["TotalInteractionProbability"]

        return weights

    def FixIC86(self, frame):

        if not frame.Has(self.dictname) :
            self.PushFrame(frame,"OutBox");
            return True

        weights = copy.copy(frame.Get( self.dictname) )

        if not 'InjectionAreaCGS' in weights:
            weights["InjectionAreaCGS"] = weights["InjectionAreaNormCGS"]

        if not 'SolidAngle' in weights:
            zenithmin = weights["MinZenith"]
            zenithmax = weights["MaxZenith"]
            azimuthmin = weights["MinAzimuth"]
            azimuthmax = weights["MaxAzimuth"]
            maxcoszen = math.cos(zenithmin);
            mincoszen = math.cos(zenithmax);
            solidAngle = (maxcoszen - mincoszen)*(azimuthmax - azimuthmin);
            weights['SolidAngle'] = solidAngle
        
        return weights

    def DAQ(self, frame):

        weights = {}

        if (self.productiontag == "IC79") :
            weights = self.FixIC79(frame)
        elif (self.productiontag == "IC86") :
            weights = self.FixIC86(frame)

        frame.Delete(self.dictname)
        frame.Put(self.dictname, weights)

        self.PushFrame(frame,"OutBox");
        return True

    def Finish(self):
        return True


