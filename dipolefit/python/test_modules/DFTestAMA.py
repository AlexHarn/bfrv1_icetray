from icecube import icetray, dataclasses, dipolefit
from icecube.dipolefit.test_modules.utility import ensure_distance


class DFTestAMA(icetray.I3Module):
    def __init__(self, context):
        icetray.I3Module.__init__(self, context)
        self.AddOutBox("OutBox")

    def Configure(self):
        pass

    def Physics(self, frame):
        track = frame['DipoleFit']
        params = frame['DipoleFitParams']
        ensure_distance(track.dir.zenith, 2.790, 0.001, "Zenith failed")
        ensure_distance(track.dir.azimuth, 2.871, 0.001,"Azimuth failed")
        ensure_distance(track.pos.x, -48.472, 0.001, "X failed")
        ensure_distance(track.pos.y, 6.142, 0.001, "Y failed")
        ensure_distance(track.pos.z, 5.567, 0.001, "Z failed")
        ensure_distance(track.time, 1833.02, 0.01, "T failed")
        ensure_distance(params.Magnet, 0.8384, 0.0001, "Magnet failed")
        ensure_distance(params.MagnetX, 0.2782, 0.0001, "MagnetX failed")
        ensure_distance(params.MagnetY, -0.0772, 0.0001, "MagnetY failed")
        ensure_distance(params.MagnetZ, 0.7871, 0.0001, "MagnetZ failed")
        ensure_distance(params.AmpSum, 34, 0.1, "ampSum failed")
        ensure_distance(params.NHits, 68, 0.1, "nHits failed")
        ensure_distance(params.NPairs, 34, 0.1, "nPairs failed")
        ensure_distance(params.MaxAmp, 1, 0.1, "maxAmp failed")
        self.PushFrame(frame)

