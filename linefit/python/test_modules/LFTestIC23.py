from icecube import icetray, dataclasses, linefit
from icecube.linefit.test_modules.utility import ensure_distance


class LFTestIC23(icetray.I3Module):
    def __init__(self, context):
        icetray.I3Module.__init__(self, context)
        self.AddOutBox("OutBox")

    def Configure(self):
        pass

    def Physics(self, frame):
        track = frame['LineFit']
        params = frame['LineFitParams']
        ensure_distance(track.dir.zenith, 0.72534, 0.001, "Zenith failed")
        ensure_distance(track.dir.azimuth, 0.05270, 0.001,"Azimuth failed")
        ensure_distance(track.pos.x, -140.9839, 0.001, "X failed")
        ensure_distance(track.pos.y, 15.8069, 0.001, "Y failed")
        ensure_distance(track.pos.z, 353.8000, 0.001, "Z failed")
        ensure_distance(track.time, 6385.304, 0.1, "T failed")
        ensure_distance(params.LFVel, 0.25617, 0.0001, "LFvel failed")
        ensure_distance(params.LFVelX, -0.16970, 0.0001, "LFvelX failed")
        ensure_distance(params.LFVelY, -0.00895, 0.0001, "LFvelY failed")
        ensure_distance(params.LFVelZ, -0.19168, 0.0001, "LFvelZ failed")
        self.PushFrame(frame)

