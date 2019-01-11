from icecube.icetray import I3PacketModule, I3Frame, I3Units

class RateTracker(I3PacketModule):
    def __init__(self, ctx):
        I3PacketModule.__init__(self, ctx, I3Frame.DAQ)
        self.AddOutBox("OutBox")
        
        self.last_time = None
        self.livetime = 0
        self.events_seen = 0
        self.events_passed = 0

    def Configure(self):
        pass

    def on_stream(self, frame):
        return frame.Stop == frame.Physics and frame['I3EventHeader'].sub_event_stream == 'InIceSplit'

    def FramePacket(self, frames):
        
        header = frames[0]['I3EventHeader']
        if self.last_time is None:
            self.last_time = header.start_time
            
        t = header.end_time
        dt = t - self.last_time
        if dt < 30*I3Units.second:
            self.livetime += dt
        self.last_time = t

        self.events_seen += 1
        if sum([1 for fr in frames if self.on_stream(fr)]) > 0:
            self.events_passed += 1
        for frame in frames:
            self.PushFrame(frame)
    
    def Finish(self):
        dt = self.livetime/I3Units.second
        print '='*80
        if self.events_passed is not None and dt is not None:
            print 'Passed %d/%d events (reduced by %g)' % (self.events_passed, self.events_seen, self.events_seen/float(self.events_passed))
            print 'Rates: %g Hz / %g Hz' % (self.events_passed/dt, self.events_seen/dt)
            print 'Total livetime: %.1f s' % (dt)
        else:
            print 'None events survived the cuts\n'
        print '='*80
