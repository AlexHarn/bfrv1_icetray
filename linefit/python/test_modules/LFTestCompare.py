from icecube import icetray, dataclasses, linefit
from icecube.icetray import I3Units
from icecube.linefit.test_modules.utility import ensure_distance


class LFTestCompare(icetray.I3Module):
    def __init__(self, context):
        icetray.I3Module.__init__(self, context)
        self.AddOutBox("OutBox")

    def Configure(self):
        pass

    def Physics(self, frame):
        track = frame['LineFit']
        params = frame['LineFitParams']
        ensure_distance(track.dir.zenith, 160.381*I3Units.deg, 0.001, "Zenith failed")
        ensure_distance(track.dir.azimuth, 169.214*I3Units.deg, 0.001,"Azimuth failed")
        ensure_distance(track.pos.x, -48.472, 0.001, "X failed")
        ensure_distance(track.pos.y, 6.142, 0.001, "Y failed")
        ensure_distance(track.pos.z, 5.567, 0.001, "Z failed")
        ensure_distance(track.time, 1833.0, 0.1, "T failed")
        ensure_distance(params.LFVel, 0.1435, 0.0001, "LFvel failed")
        ensure_distance(params.LFVelX, 0.0473, 0.0001, "LFvelX failed")
        ensure_distance(params.LFVelY, -0.0090, 0.0001, "LFvelY failed")
        ensure_distance(params.LFVelZ, 0.1352, 0.0001, "LFvelZ failed")
        self.PushFrame(frame)

