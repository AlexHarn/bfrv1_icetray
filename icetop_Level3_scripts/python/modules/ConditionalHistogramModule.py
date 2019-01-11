from icecube.production_histograms.histogram_modules.histogram_module import HistogramModule
from icecube.production_histograms.histograms.histogram import Histogram

from math import *

class ConditionalHistogramModule(HistogramModule):
    """
    Module to handle histogram filling and quality cuts.
    """
    def __init__(self, name, histograms, condition='True'):
        HistogramModule.__init__(self)
        self.name = name
        self.condition = condition
        for h in histograms:
            if isinstance(h,Histogram):
                h.name = name + '_' + h.name
                self.append(h)
            if isinstance(h,HistogramModule):
                for k in h.histograms.keys():
                    h.histograms[k].name = name + '_'+h.histograms[k].name
                self.append(h)

    def _Process_(self, frame):
        from icecube.icetray.i3logging import log_debug, log_info
        from icecube import icetray, dataclasses, recclasses, simclasses
        from I3Tray import I3Units
        try:
            condition = eval(self.condition)
            return condition
        except Exception as e:
            icetray.logging.log_info("%s" % str(e))
            icetray.logging.log_info("Module Name : %s" % self.name)
            icetray.logging.log_info("Calling condition : %s" % self.condition)
            icetray.logging.log_info("Exception : %s" % str(e))
        return False

    def Physics(self, frame):
        if self._Process_(frame):
            for k,h in self.histograms.iteritems():
                h.Physics(frame)

    """
    def DAQ(self, frame):
        if self._Process_(frame):
            for k,h in self.histograms.iteritems(): h.DAQ(frame)
    def Geometry(self, frame):
        if self._Process_(frame):
            for k,h in self.histograms.iteritems(): h.Geometry(frame)
    def Calibration(self, frame):
        if self._Process_(frame):
            for k,h in self.histograms.iteritems(): h.Calibration(frame)
    def DetectorStatus(self, frame):
        if self._Process_(frame):
            for k,h in self.histograms.iteritems(): h.DetectorStatus(frame)


"""
