from icecube import icetray, dataclasses, dipolefit
from icecube.dipolefit.test_modules.utility import ensure_distance


class DFTestIC23(icetray.I3Module):
    def __init__(self, context):
        icetray.I3Module.__init__(self, context)
        self.AddOutBox("OutBox")

    def Configure(self):
        pass

    def Physics(self, frame):
        track = frame['DipoleFit']
        params = frame['DipoleFitParams']
        ensure_distance(track.dir.zenith, 0.74422, 0.001, "Zenith failed")
        ensure_distance(track.dir.azimuth, 0.05270, 0.001,"Azimuth failed")
        ensure_distance(track.pos.x, -140.984, 0.001, "X failed")
        ensure_distance(track.pos.y, 15.807, 0.001, "Y failed")
        ensure_distance(track.pos.z, 353.800, 0.001, "Z failed")
        ensure_distance(track.time, 6385.304, 0.01, "T failed")
        ensure_distance(params.Magnet, 0.59794, 0.0001, "Magnet failed")
        ensure_distance(params.MagnetX, -0.40448, 0.0001, "MagnetX failed")
        ensure_distance(params.MagnetY, -0.021339, 0.0001, "MagnetY failed")
        ensure_distance(params.MagnetZ, -0.43985, 0.0001, "MagnetZ failed")
        ensure_distance(params.AmpSum, 5, 0.1, "ampSum failed")
        ensure_distance(params.NHits, 10, 0.1, "nHits failed")
        ensure_distance(params.NPairs, 5, 0.1, "nPairs failed")
        ensure_distance(params.MaxAmp, 1, 0.1, "maxAmp failed")
        self.PushFrame(frame)

