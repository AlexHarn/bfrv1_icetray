from icecube import icetray, dataclasses, linefit
import math

class LFTest(icetray.I3Module):
    def __init__(self, context):
        icetray.I3Module.__init__(self, context)
        self.AddOutBox("OutBox")

    def Configure(self):
        pass

    def Physics(self, frame):
        pi = math.pi
        track = frame['LineFit']
        params = frame['LineFitParams']
        if track.dir.zenith < -pi or track.dir.zenith > pi:
            print("Zenith test FAILED")
            assert()
        if track.dir.azimuth < -2*pi or track.dir.azimuth > 2*pi:
            print("Azimuth test FAILED")
            assert()
        if math.fabs(params.LFVelX**2 + params.LFVelY**2 + params.LFVelZ**2 - params.LFVel**2) > 1.e-8:
            print("LF Velocity test FAILED")
            assert()
        self.PushFrame(frame)

