from icecube import dataclasses
from icecube.icetray import I3ConditionalModule, I3Frame
from icecube.icetray.i3logging import log_info, log_warn, log_debug, log_trace

class FilterWaveforms(I3ConditionalModule):
    """
    """
    def __init__(self, ctx):
        I3ConditionalModule.__init__(self, ctx)
        self.AddOutBox("OutBox")
        self.AddParameter('Pulses', 'Pulses', 'OfflineIceTopHLCTankPulses')

    def _prescale_(self, n):
        # Keep at least 1 % of the waveforms, then keep the number of kept waveforms stable as function of the energy (rising power law with exponential 3), up to when 50 stations are hit. From there on, keep all.                                                                                                                                                                                                  
        import math
        pres=(0.99/math.pow(50.0,3))*math.pow(n,3)+0.01
        if pres>1.0:
            pres=1.0
        return float(pres)

    def Configure(self):
        self.pulses = self.GetParameter('Pulses')

    def DAQ(self, frame):
        from icecube.icetop_Level3_scripts.functions import count_stations
        import numpy
        if not self.pulses in frame:
            self.PushFrame(frame)
            return

        stations = count_stations(frame[self.pulses])
        prescale = self._prescale_(stations)
        if numpy.random.random() <= prescale:
            frame['IceTopWaveformWeight'] = dataclasses.I3Double(1./prescale)
        else:
            frame['IceTopWaveformWeight'] = dataclasses.I3Double(0.)
        self.PushFrame(frame)
