##################################################################################################                                                                                                         
## ReadoutWindow from InIcePulses needs to be based on the TWcleaned pulses from coinc-twc      ##                                                                                                         
##################################################################################################                                                                                                         
from icecube import icetray , dataclasses

class AddReadoutTimeWindow(icetray.I3ConditionalModule):
    def __init__(self, context):
        icetray.I3ConditionalModule.__init__(self, context)
        self.AddOutBox("OutBox")
        self.AddParameter("Pulses", "Name of pulse series map to write time range for", "CleanCoincOfflinePulses")
        
    def Configure(self):
        self.Pulses = self.GetParameter("Pulses")
        
    def Physics(self, fr):
        pulses = fr[self.Pulses]
        pulses = pulses.apply(fr)  # knowing this is Mask                                                                                                                                             
        if len(pulses)>0:
            times = []
            for omk, pulse_series in pulses:
                for pulse in pulse_series:
                    times.append(pulse.time)
            times = sorted(times)
            timediff = times[-1]-times[0]
            fr[self.Pulses + 'TimeRange'] = dataclasses.I3TimeWindow(times[0] - 25.*icetray.I3Units.ns, times[-1]+50.*icetray.I3Units.ns)
        if 'CalibratedWaveformRange' in fr:
            twr = fr['CalibratedWaveformRange']
            fr['CalibratedWaveformRange_length'] = dataclasses.I3Double(twr.length)

        self.PushFrame(fr)
