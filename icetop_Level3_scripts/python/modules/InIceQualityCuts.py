from icecube import dataclasses, icetray, phys_services
import math

class InIceQualityCuts(icetray.I3ConditionalModule):
    def __init__(self, context):
        # millipede has complicated dependencies, it is not always available;
        # delay import until someone actually creates an instance
        from icecube import millipede
        icetray.I3ConditionalModule.__init__(self,context)
        self.AddParameter('millipedeName','Name of the reconstructed Millipede cascades',0)
        self.AddParameter('millipedeParamsName','Name of the Millipede FitParams',0)
        self.AddParameter('stochasticsName','Name of the stochastics fit',0)
        self.AddParameter('minNonZeroCasc','Remove events with less than n reconstructed nonzero energy cascades',0)
        self.AddParameter('maxRlogl','Remove events with log10(rlogl) of Millipede fit larger than this value',0)
        self.AddParameter('minQTotRatio','Remove events with log10(Predicted total charge/measured total charge) larger than this value',0)
        self.AddOutBox("OutBox")

    def Configure(self):
        self.milliname = self.GetParameter('millipedeName')
        self.milli_paramname = self.GetParameter('millipedeParamsName')
        self.stochname = self.GetParameter('stochasticsName')
        self.min_nonzero = self.GetParameter('minNonZeroCasc')
        self.max_rlogl = self.GetParameter('maxRlogl')
        self.min_qtot_ratio = self.GetParameter('minQTotRatio')

    def Physics(self, frame):
        if self.milliname in frame:
            milli = frame[self.milliname]
            # this is an I3Vector<I3Particle>                                                                                                                                               
            milli_param = frame[self.milli_paramname]
            milli_rlogl_bool=icetray.I3Bool(math.log10(milli_param.rlogl) < self.max_rlogl)
            milli_min_qtot_ratio_bool=icetray.I3Bool( math.log10(milli_param.predicted_qtotal/milli_param.qtotal) > self.min_qtot_ratio)
            milli_ncasc_bool= icetray.I3Bool(len([part for part in milli if part.energy > 0]) >= self.min_nonzero) # Number of cascades cut.
        else:
            milli_rlogl_bool=icetray.I3Bool(False)   
            milli_min_qtot_ratio_bool=icetray.I3Bool(False)   
            milli_ncasc_bool=icetray.I3Bool(False)   
        if self.stochname in frame:
            stoch = frame[self.stochname]
            stoch_succeeded=icetray.I3Bool(stoch.status == dataclasses.I3Particle.OK)
        else:
            stoch_succeeded=icetray.I3Bool(False)
        frame["milli_rlogl_passed"]=milli_rlogl_bool
        frame["milli_qtot_ratio_passed"]=milli_min_qtot_ratio_bool
        frame["milli_ncasc_passed"]=milli_rlogl_bool
        frame["stoch_reco_passed"]=stoch_succeeded
        
        self.PushFrame(frame,"OutBox")
