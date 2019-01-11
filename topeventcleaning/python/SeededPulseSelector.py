from icecube.icetray import I3Module, I3ConditionalModule, I3Frame
from icecube import dataclasses
from icecube.dataclasses import I3RecoPulseSeriesMapMask, I3EventHeader
from icecube.icetray import I3Units
from . import tools

class SeededPulseSelector(I3ConditionalModule):
    """
    I3Module to do a simple selection of IceTop DOM pulses depending on the agreement between the pulses' time and the time of a given reconstruction (presumably InIce or MC)
    """
    def __init__(self, ctx):
        I3ConditionalModule.__init__(self, ctx)
        self.AddParameter("SLCPulses", "ID of input SLC pulse list", "OfflineIceTopSLCTankPulses")
        self.AddParameter("SLCPulsesOut", "ID for the resulting list of SLC selected pulses", "OfflineIceTopSelectedSLC")
        self.AddParameter("HLCPulses", "ID of input SLC pulse list", "OfflineIceTopHLCTankPulses")
        self.AddParameter("HLCPulsesOut", "ID for the resulting list of SLC selected pulses", "OfflineIceTopSelectedHLC")
        self.AddParameter("SLCAfterPulsesOut", "ID for the resulting list of SLC after-pulses", None)
        self.AddParameter("HLCAfterPulsesOut", "ID for the resulting list of HLC after-pulses", None)
        self.AddParameter("SLCRejectedPulsesOut", "ID for the resulting list of SLC rejected pulses", None)
        self.AddParameter("HLCRejectedPulsesOut", "ID for the resulting list of HLC rejected pulses", None)
        self.AddParameter("Seed", "ID of the I3Particle containing the shower geometry", "MPEFitMuE")
        self.AddParameter("Interval", "Only pulses arriving in this interval are accepted", (-200*I3Units.ns, 800*I3Units.ns))
        self.AddParameter("APTime", "All pulses beyond this time are called afterpulses", 6500*I3Units.ns)
        self.AddOutBox("OutBox")

        self.geometry = None

    def Configure(self):
        self.seed = self.GetParameter("Seed")
        self.hlc_name = self.GetParameter("HLCPulses")
        self.slc_name = self.GetParameter("SLCPulses")
        self.hlc_out_name = self.GetParameter("HLCPulsesOut")
        self.slc_out_name = self.GetParameter("SLCPulsesOut")
        self.hlc_ap_out_name = self.GetParameter("HLCAfterPulsesOut")
        self.slc_ap_out_name = self.GetParameter("SLCAfterPulsesOut")
        self.hlc_rej_out_name = self.GetParameter("HLCRejectedPulsesOut")
        self.slc_rej_out_name = self.GetParameter("SLCRejectedPulsesOut")
        self.interval = self.GetParameter("Interval")
        self.ap_time = self.GetParameter("APTime")

    def Physics(self, frame):

        if not 'I3Geometry' in frame:
            raise RuntimeError("Frame contains no I3Geometry!")
        self.geometry = frame['I3Geometry']

        if not frame.Has(self.seed):
            self.PushFrame(frame)
            return

        reco = frame[self.seed]

        # This is the main part.
        # tools.classify_from_seed takes a list of pulses, an I3Particle and a geometry object
        # and returns a dictionary with three keys: ok, after-pulses, and rejected
        if self.hlc_name in frame:
            hlc_pulses = tools.classify_from_seed(dataclasses.I3RecoPulseSeriesMap.from_frame(frame,self.hlc_name), reco, self.geometry,
                                                  min_time=self.interval[0],
                                                  max_time=self.interval[1],
                                                  afterpulse_time=self.ap_time)
        else:
            hlc_pulses = {'ok':{}, 'after-pulses':{}, 'rejected':{}}
        if self.slc_name in frame:
            slc_pulses = tools.classify_from_seed(dataclasses.I3RecoPulseSeriesMap.from_frame(frame,self.slc_name), reco, self.geometry,
                                                  min_time=self.interval[0],
                                                  max_time=self.interval[1],
                                                  afterpulse_time=self.ap_time)
        else:
            slc_pulses = {'ok':{}, 'after-pulses':{}, 'rejected':{}}

        # Create masks and put them in the frame
        if self.slc_name in frame:
            slc_ok_mask = dataclasses.I3RecoPulseSeriesMapMask(frame, self.slc_name)
            slc_ok_mask.set_none()
            for k in slc_pulses['ok']:
                slc_ok_mask.set(k[0], k[1], True)
            frame.Put(self.slc_out_name, slc_ok_mask)

        if self.hlc_name in frame:
            hlc_ok_mask = dataclasses.I3RecoPulseSeriesMapMask(frame, self.hlc_name)
            hlc_ok_mask.set_none()
            for k in hlc_pulses['ok']:
                hlc_ok_mask.set(k[0], k[1], True)
            frame.Put(self.hlc_out_name, hlc_ok_mask)

        # The remaining masks are created only if they are requested in the configuration
        if not self.hlc_ap_out_name is None and self.hlc_name in frame:
            hlc_ap_mask = dataclasses.I3RecoPulseSeriesMapMask(frame, self.hlc_name)
            hlc_ap_mask.set_none()
            for k in hlc_pulses['after-pulses']:
                hlc_ap_mask.set(k[0], k[1], True)
            frame.Put(self.hlc_ap_out_name, hlc_ap_mask)

        if not self.hlc_rej_out_name is None and self.hlc_name in frame:
            hlc_rej_mask = dataclasses.I3RecoPulseSeriesMapMask(frame, self.hlc_name)
            hlc_rej_mask.set_none()
            for k in hlc_pulses['rejected']:
                hlc_rej_mask.set(k[0], k[1], True)
            frame.Put(self.hlc_rej_out_name, hlc_rej_mask)

        if self.slc_name in frame and not self.slc_ap_out_name is None:
            slc_ap_mask = dataclasses.I3RecoPulseSeriesMapMask(frame, self.slc_name)
            slc_ap_mask.set_none()
            for k in slc_pulses['after-pulses']:
                slc_ap_mask.set(k[0], k[1], True)
            frame.Put(self.slc_ap_out_name, slc_ap_mask)

        if self.slc_name in frame and not self.slc_rej_out_name is None:
            slc_rej_mask = dataclasses.I3RecoPulseSeriesMapMask(frame, self.slc_name)
            slc_rej_mask.set_none()
            for k in slc_pulses['rejected']:
                slc_rej_mask.set(k[0], k[1], True)
            frame.Put(self.slc_rej_out_name, slc_rej_mask)

        self.PushFrame(frame)


