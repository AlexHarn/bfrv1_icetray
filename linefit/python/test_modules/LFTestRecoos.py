from icecube import icetray, dataclasses
import math

class LFTestRecoos(icetray.I3Module):
    def __init__(self, context):
        icetray.I3Module.__init__(self, context)
        self.AddOutBox("OutBox")

    def Physics(self, frame):
        f2ktrack = frame['F2kTrack04']
        lftrack = frame['LineFit']
        if math.fabs(lftrack.zenith - f2ktrack.zenith) > 0.001:
            print("IceTray LineFit and Recoos LineFit zenith differ.")
            assert()
        if math.fabs(lftrack.azimuth - f2ktrack.azimuth) > 0.001:
            print("IceTray LineFit and Recoos LineFit azimuth differ.")
            assert()
        self.PushFrame(frame)

