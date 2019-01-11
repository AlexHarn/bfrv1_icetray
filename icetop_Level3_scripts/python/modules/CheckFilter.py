from icecube import dataclasses, icetray
from icecube.icetray import I3ConditionalModule, I3Frame
from icecube.icetop_Level3_scripts import icetop_globals
from icecube.icetray.i3logging import log_info, log_warn, log_debug, log_trace

# All trigger result names in filter masks for several campaigns with their corresponding prescale:
# Note that they are ordered from most to least restrictive (low-to-high prescale).
# We also threw SDST and normal events on one pile here! Since SDST events have no prescale, we use a prescale of 1 everywhere where SDST could possibly be used.
icetop_filters = {'IC79':[('IceTopSTA8', 1), ('IceTopSTA3', 3)],
                  'IC86.2011':[('IceTopSTA8', 1), ('IceTopSTA3', 3)],
                  'IC86.2012':[('IceTopSTA5', 1), ('IceTopSTA3', 1)],
                  'IC86.2013':[('IceTopSTA5', 1), ('IceTopSTA3', 1)],
                  'IC86.2014':[('IceTopSTA5', 1), ('IceTopSTA3', 1)],
                  'IC86.2015':[('IceTopSTA5', 1), ('IceTopSTA3', 1)],
                  'IC86.2016':[('IceTopSTA5', 1), ('IceTopSTA3', 1)]}

infill_filters = {'IC79':[],
                  'IC86.2011':[('IceTop_InFill_STA3', 1)],
                  'IC86.2012':[('IceTop_InFill_STA3', 1)],
                  'IC86.2013':[('IceTop_InFill_STA3', 1)],
                  'IC86.2014':[('IceTop_InFill_STA3', 1)],
                  'IC86.2015':[('IceTop_InFill_STA3', 1)],
                  'IC86.2016':[('IceTop_InFill_STA3', 1)]}

class CheckFilter(I3ConditionalModule):
    """
    I3Module to check the filter mask.
    """
    def __init__(self, ctx):
        I3ConditionalModule.__init__(self, ctx)
        self.AddOutBox("OutBox")
        self.AddParameter("Detector", "Detector configuration (IC79 or IC86.20*).", None)
        self.AddParameter("Filter", "Flag to skip events that do not pass any of the conditions", False)
        self.AddParameter("isMC","Data or MC?",False)
    def Configure(self):
        from icecube.icetop_Level3_scripts.icetop_globals import names
        self.detector = self.GetParameter("Detector")
        self.filt = self.GetParameter("Filter")
        self.mc=self.GetParameter("isMC")

        self.icetop_filters = icetop_filters[self.detector]
        self.infill_filters = infill_filters[self.detector]

    def Physics(self, frame):
        icetop_passed, icetop_prescale = self._icetop_standard_(frame)
        if self.mc:
            icetop_prescale=1
        if 'IceTop_StandardFilter' not in frame:
            frame['IceTop_StandardFilter'] = icetray.I3Bool(icetop_passed)
            frame['IceTop_EventPrescale'] = icetray.I3Int(icetop_prescale)
        
        infill_passed, infill_prescale = self._infill_(frame)
        if self.mc:
            infill_prescale=1
        if 'IceTop_InFillFilter' not in frame:
            frame['IceTop_InFillFilter'] = icetray.I3Bool(infill_passed)
        
        if icetop_passed or infill_passed or not self.filt:
            self.PushFrame(frame)

    def _icetop_standard_(self, frame):
        for result, result_prescale in self.icetop_filters:
            if result in frame:
                if frame[result]:
                    return True, result_prescale
        return False, 0

    def _infill_(self,frame):
        for result, result_prescale in self.infill_filters:
            if result in frame:
                if frame[result]:
                    return True, result_prescale
        return False, 0
