from icecube import icetray, dataclasses, phys_services
from icecube.icetray.i3logging import log_fatal
    
class DOMselection_InIce(icetray.I3ConditionalModule):
    def __init__(self, ctx):
        icetray.I3ConditionalModule.__init__(self, ctx)
        self.AddParameter('ICpulses','IceCube RecoPulses name',0)
        self.AddParameter('OutputName','Name of output TimeWindowMap',0)
        self.AddParameter('Ratio','DOMs with a larger fraction of total event charge are cut',0.8)

        self.AddOutBox("OutBox")
       
    def Configure(self):
        self.pulseName = self.GetParameter('ICpulses')
        self.outputName = self.GetParameter('OutputName')
        self.ratio = self.GetParameter('Ratio')
            
    def Geometry(self,frame):
        if 'I3Geometry' in frame:
            geom = frame['I3Geometry']
            self.omgeo = geom.omgeo
        self.PushFrame(frame,"OutBox")

    def Physics(self, frame):
        if self.pulseName in frame:
            pulsemap = frame[self.pulseName]
            if pulsemap.__class__ == dataclasses.I3RecoPulseSeriesMapMask:
                pulsemap = pulsemap.apply(frame)
        else:
            log_fatal('Missing RecoPulses. Should not happen')
        
        tot_charge = 0
        avg_charge = 0
        # calc tot_charge
        if len(pulsemap)>0:
            for om,pulses in pulsemap:
                for pulse in pulses:
                    tot_charge +=pulse.charge
            av_charge = tot_charge/len(pulsemap)

        distance = 0
        unused_doms = dataclasses.I3VectorOMKey()
        
        for omkey in pulsemap.keys():
            pulses = pulsemap[omkey]
            intcharge = 0
            for pulse in pulses:
                intcharge += pulse.charge
            if(intcharge/tot_charge > self.ratio):
                icetray.logging.log_info('Charge of this om is larger than requested ratio : %f' % (intcharge/tot_charge))
                unused_doms.append(omkey)
        frame[self.outputName] = unused_doms
        self.PushFrame(frame,"OutBox")
    
