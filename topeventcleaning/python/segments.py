from icecube import icetray
from icecube.icetray import I3Units


@icetray.traysegment
def SelectPulsesFromSeed(tray, name, Seed,
                         Interval=(-200*I3Units.ns, 800*I3Units.ns),
                         APTime = -6500*I3Units.ns,
                         HLCPulses = 'OfflineIceTopHLCTankPulses',
                         SLCPulses = 'OfflineIceTopSLCTankPulses',
                         debug=True,
                         tag=None,
                         If= lambda fr: True):
    """
    This segment produces a few pulse containers, based on their
    agreement between pulse times and the arrival time of a plane
    shower front specified by an I3Particle that must be in the
    frame. The name of this I3Particle is passed as the 'Seed'
    parameter to this segment.

    Note this segment's 'tag' parameter. By default this is is equal
    to the name of the segment. The frame objects produced by this
    segment are:

    * <tag>SelectedSLC
    * <tag>SelectedHLC
    * <tag>RejectedSLC    (only if debug==True)
    * <tag>RejectedHLC    (only if debug==True)
    * <tag>AfterPulsesSLC (only if debug==True)
    * <tag>AfterPulsesHLC (only if debug==True)

    If these frame objects exist in the frame, they are removed.
    """

    from icecube import topeventcleaning

    if tag is None:
        tag = name

    SLCPulsesOut = tag + 'SelectedSLC'
    HLCPulsesOut = tag + 'SelectedHLC'
    if debug:
        SLCAfterPulsesOut = tag + 'AfterPulsesSLC'
        HLCAfterPulsesOut = tag + 'AfterPulsesHLC'
        SLCRejectedPulsesOut = tag + 'RejectedSLC'
        HLCRejectedPulsesOut = tag + 'RejectedHLC'
    else:
        SLCAfterPulsesOut = ''
        HLCAfterPulsesOut = ''
        SLCRejectedPulsesOut = ''
        HLCRejectedPulsesOut = ''

    tray.AddModule("Delete",name+"_remove_previous", 
                   Keys=[SLCPulsesOut, SLCAfterPulsesOut, SLCRejectedPulsesOut, HLCPulsesOut, HLCAfterPulsesOut, HLCRejectedPulsesOut],
                   If=If
                   )

    tray.AddModule(topeventcleaning.SeededPulseSelector, name + 'SeededPulseSelector',
                   SLCPulses = SLCPulses,
                   HLCPulses = HLCPulses,
                   SLCPulsesOut = SLCPulsesOut,
                   HLCPulsesOut = HLCPulsesOut,
                   SLCAfterPulsesOut = SLCAfterPulsesOut,
                   HLCAfterPulsesOut = HLCAfterPulsesOut,
                   SLCRejectedPulsesOut = SLCRejectedPulsesOut,
                   HLCRejectedPulsesOut = HLCRejectedPulsesOut,
                   Seed=Seed,
                   APTime=APTime,
                   Interval=Interval,
                   If = If
                   )


