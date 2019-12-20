from icecube import dataclasses
from icecube.icetray import load, traysegment
from I3Tray import *
@traysegment
def TopologicalCounter(tray, name, pulses='OfflinePulsesHLC', InIceCscd = lambda frame: True):


    """
    Count the number of topological clusters in a given pulse series map,
    using only the first HLC pulse in each DOM.
    
    Returns a list containing the name of the I3Int containing the split count.
    
    :param Pulses: name of the pulse series to split
    """

    from icecube import TopologicalSplitter
       
    todo = lambda frame: name+'Count' not in frame
       
    FirstPulses = name+'FirstPulses'
        
    def FirstPulseCleaning(frame):
        """Select the first pulse in each DOM"""
        if not todo(frame):
            return
        if not frame.Has(FirstPulses):
            pulsemap = dataclasses.I3RecoPulseSeriesMap.from_frame(frame, pulses)
            mask = dataclasses.I3RecoPulseSeriesMapMask(frame, pulses)
            mask.set_none()
            for om, pulse in pulsemap.iteritems():
                if len(pulse) > 0:
                    mask.set(om, 0, True)
            frame[FirstPulses] = mask

    tray.AddModule(FirstPulseCleaning, FirstPulses)      

    tray.AddModule('ttrigger<I3RecoPulse>', name, If=InIceCscd,
                   TimeWindow=4*I3Units.microsecond, XYDist=400*I3Units.m,
                   ZDomDist=30, TimeCone=1*I3Units.microsecond, Multiplicity=4,
                   InputNames=[FirstPulses], OutputName=name, SaveSplitCount=True)

