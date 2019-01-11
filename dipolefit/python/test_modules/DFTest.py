from icecube import icetray, dataclasses, dipolefit
from icecube.dipolefit.test_modules.utility import ensure_distance
import math


class DFTest(icetray.I3Module):
    def __init__(self, context):
        icetray.I3Module.__init__(self, context)
        self.AddOutBox("OutBox")

    def Configure(self):
        pass

    def Physics(self, frame):
        pi = math.pi
        track = frame['DipoleFit']
        params = frame['DipoleFitParams']
        if track.dir.zenith < -pi or track.dir.zenith > pi:
            print("Zenith test FAILED")
            assert()
        if track.dir.azimuth < -2*pi or track.dir.azimuth > 2*pi:
            print("Azimuth test FAILED")
            assert()
        ensure_distance(params.MagnetX**2+params.MagnetY**2+params.MagnetZ**2,
                        params.Magnet**2, 1e-8)
        self.PushFrame(frame)

