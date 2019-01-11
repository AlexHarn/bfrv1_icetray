from icecube import icetray, dataclasses
import math


try:
    math.isnan
except AttributeError:
    def isNaN(num):
        return num != num
    math.isnan = isNaN


class LFTestMinHit(icetray.I3Module):
    def __init__(self, context):
        icetray.I3Module.__init__(self, context)
        self.AddOutBox("OutBox")

    def Configure(self):
        pass

    def Physics(self, frame):
        track = frame['LineFit']
        if not math.isnan(track.dir.zenith):
            print("Zenith test FAILED")
            assert()
        if not math.isnan(track.dir.azimuth):
            print("Azimuth test FAILED")
            assert()
        self.PushFrame(frame)

