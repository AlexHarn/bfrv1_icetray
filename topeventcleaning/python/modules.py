from icecube.icetray import I3PacketModule, I3Frame
from icecube.dataclasses import I3RecoPulseSeriesMapMask, I3EventHeader

class KeepOnlyLargestEvent(I3PacketModule):
    def __init__(self, ctx):
        I3PacketModule.__init__(self, ctx, I3Frame.DAQ)
        self.AddParameter("Pulses", "Pulse series to look for", "")
        self.AddOutBox("OutBox")
    
    def Configure(self):
        self.pulses_name = self.GetParameter("Pulses")
        if len(self.pulses_name) == 0:
            raise RuntimeError("You have to specify a pulse series!")
    
    def FramePacket(self, frames):
        if len(frames) <= 1:       # only Q frame
            for frame in frames:
                self.PushFrame(frame)
            return
        
        self.PushFrame(frames[0])    # push Q frame
        
        largest_event = 0
        largest_size = 0
        for frame in frames[1:]:
            if not self.pulses_name in frame:
                self.PushFrame(frame)
                continue

            event_size = self._get_event_size(frame)
            if event_size > largest_size:
                largest_size = event_size
                largest_event = frame

        if largest_event != 0:
            self.PushFrame(largest_event)
    
    def _get_event_size(self, frame):
        pulses = frame[self.pulses_name].apply(frame)
        npulses = 0
        for pulse_series in pulses.values():
            npulses += len(pulse_series)
        
        return npulses


class MergeIIIT(I3PacketModule):
    def __init__(self, ctx):
        I3PacketModule.__init__(self, ctx, I3Frame.DAQ)
        self.AddParameter("IceTopReco", "Name of the IceTop reconstruction to look for", "")
        self.AddParameter("InIceReco", "Name of the InIce reconstruction to look for", "")
        self.AddOutBox("OutBox")

    def Configure(self):
        self._icetop_reco = self.GetParameter("IceTopReco")
        self._inice_reco = self.GetParameter("InIceReco")
        if len(self._icetop_reco) == 0 or len(self._inice_reco) == 0:
            raise RuntimeError("You have to specify both an IceTop and an InIce reconstruction")

    def FramePacket(self, frames):
        if len(frames) <= 1:     # only q frame
            for frame in frames:
                self.PushFrame(frame)
            return

        daq = frames[0]
        self.PushFrame(daq)

        icetop_frame = 0
        inice_frame = 0

        for frame in frames[1:]:
            if self._icetop_reco in frame:
                if icetop_frame != 0:
                    raise RuntimeError("Sorry, but this module is pretty dumb. It cannot treat cases where the IceTop reconstruction appears in more than one P frame.")
                icetop_frame = frame
            if self._inice_reco in frame:
                if inice_frame != 0:
                    raise RuntimeError("Sorry, but this module is pretty dumb. It cannot treat cases where the InIce reconstruction appears in more than one P frame.")
                inice_frame = frame
                
            self.PushFrame(frame)

        if icetop_frame != 0 and inice_frame != 0:
            new_frame = I3Frame(I3Frame.Physics)
            new_frame.merge(inice_frame)
            eh = I3EventHeader(new_frame['I3EventHeader'])
            new_frame.Delete('I3EventHeader')
            eh.sub_event_id = 0
            eh.sub_event_stream = self.name
            new_frame.merge(icetop_frame)
            new_frame.Delete('I3EventHeader')
            new_frame['I3EventHeader'] = eh
            self.PushFrame(new_frame)


class EnsureStreamExists(I3PacketModule):
    def __init__(self, ctx):
        I3PacketModule.__init__(self, ctx, I3Frame.DAQ)
        self.AddParameter("SubEventStream", "SubEventStream to inspect", "")
        self.AddOutBox("OutBox")
    
    def Configure(self):
        self.sub_event_stream = self.GetParameter("SubEventStream")
        if len(self.sub_event_stream) == 0:
            raise RuntimeError("You have to specify a sub-event stream!")
    
    def FramePacket(self, frames):
        self.PushFrame(frames[0])

        daq = frames[0]
        p_frames = frames[1:]

        good = False
        for frame in p_frames:
            if 'I3EventHeader' in frame and \
                    frame['I3EventHeader'].sub_event_stream == self.sub_event_stream:
                good = True
            self.PushFrame(frame)

        if not good:
            frame = I3Frame(I3Frame.Physics)
            frame.stop = I3Frame.Physics
            eh = I3EventHeader(daq['I3EventHeader'])
            eh.sub_event_id = 0
            eh.sub_event_stream = self.sub_event_stream
            frame['I3EventHeader'] = eh
            self.PushFrame(frame)
