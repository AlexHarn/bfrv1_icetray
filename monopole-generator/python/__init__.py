# $Id$

import sys
import collections
import math
from icecube import icetray

try:
    icetray.load('libmonopole-generator', False)
except RuntimeError:
    sys.stderr.write("ERROR: Could not load libmonopole-generator (%s)." % sys.exc_info()[1])

try:
    icetray.load('libmonopole-propagator', False)
except RuntimeError:
    sys.stderr.write("ERROR: Could not load libmonopole-propagator (%s)." % sys.exc_info()[1])



class MonopoleRenormalizeWeights(icetray.I3ConditionalModule):
    """Collect all frames before propagating them down the tray to apply file wide proper weighting
    This means you have all frames in memory for a moment and hence should be done right after the monopole generator
    then the frames are nearly empty. With that, having 10k frames is not a problem and gives you a very easy way to do
    waiting down the line."""
    def __init__(self,context):
        icetray.I3ConditionalModule.__init__(self, context)
        self.frames = collections.deque()
        self.totweight = 0

    def Configure(self):
        pass

    def Physics(self,frame):
        pass

    def DAQ(self,frame):
        self.frames.append(frame)
        self.totweight += frame["MPInfoDict"]["Weight"]

    def Finish(self):
        while self.frames:
            frame = self.frames.popleft()
            frame["MPInfoDict"]["WeightFilewideNormalized"] = frame["MPInfoDict"]["Weight"] / self.totweight
            frame["MPInfoDict"]["IntegratedWeight"] = self.totweight
            self.PushFrame(frame)


def calculate_weight(MPInfoDict_WeightFilewideNormalized, monogen_DiskRadius):
    """Returns weight so you only have to multiply the weight with the expected flux in cm^-2 s^-1 sr^-1
    and divide by the number of files in a speed region (i.e. if you have overlapping simulation, the overlapped region needs to be
    divided by the number of overlapping datasets). If you are not using the MonopoleRenormalizeWeights module, you
    need to adjust your weights by the integrated generated weight you will find in your logs."""
    #see Anna Pollmanns thesis, p84, for an explanation of the formula
    return 4 * math.pi ** 2 * 10000 * monogen_DiskRadius**2 * MPInfoDict_WeightFilewideNormalized

#del icetray
del sys