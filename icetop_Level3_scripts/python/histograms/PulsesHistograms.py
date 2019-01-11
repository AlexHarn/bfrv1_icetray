from icecube.production_histograms.histograms.histogram import Histogram
from icecube.production_histograms.histogram_modules.histogram_module import HistogramModule
from icecube import icetray, dataclasses
from icecube.icetop_Level3_scripts import icetop_globals
import math

class IndividualTankPulses(HistogramModule):
    def __init__(self, frame_key):
        HistogramModule.__init__(self)
        self.frame_key=frame_key
        self.append(Histogram( -2., 5., 100, self.frame_key+"Charge"))
        self.append(Histogram( 60, 65, 4, self.frame_key+"OM"))
        self.append(Histogram( 1, 82, 81, self.frame_key+"String"))
        self.append(Histogram( 0., 15000., 100, self.frame_key+"Time"))
        self.append(Histogram( 0, 500, 100, self.frame_key+"Width"))

    def Physics(self,frame):
        if self.frame_key not in frame:
           return
        pulses=dataclasses.I3RecoPulseSeriesMap.from_frame(frame, self.frame_key)
        for omkey, pulseseries in pulses:
            for pulse in pulseseries:
                if (pulse.charge==pulse.charge and pulse.charge!=0): # Let's exclude the nans and zero charges... those create problems. Why the zero charges are there is not clear yet to me.
                    self.histograms[self.frame_key+"Charge"].fill(math.log10(pulse.charge))
                    self.histograms[self.frame_key+"OM"].fill(omkey.om)
                    self.histograms[self.frame_key+"String"].fill(omkey.string)
                    self.histograms[self.frame_key+"Width"].fill(pulse.width)
                    self.histograms[self.frame_key+"Time"].fill(pulse.time)
                
