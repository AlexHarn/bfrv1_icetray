from icecube import icetray, dataclasses
from icecube.dipolefit.test_modules.utility import ensure_distance

class DFTestRecoos(icetray.I3Module):
    def __init__(self, context):
        icetray.I3Module.__init__(self, context)
        self.AddOutBox("OutBox")

    def Configure(self):
        pass

    def Physics(self, frame):
        rdmcresult = frame['F2kTrack12']
        dftrack = frame['DipoleFit']
        ensure_distance(dftrack.zenith, rdmcresult.zenith, 0.001,
                        "Zenith: Old Recoos DipoleFit and IceTray DipoleFit differ")
        ensure_distance(dftrack.azimuth, rdmcresult.azimuth,0.001,
                        "Azimuth: Old Recoos DipoleFit and IceTray DipoleFit differ")
        self.PushFrame(frame)

