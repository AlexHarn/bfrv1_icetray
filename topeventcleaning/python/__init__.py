from icecube.icetray import load

from icecube.topeventcleaning import modules
from icecube.topeventcleaning.segments import SelectPulsesFromSeed
from icecube.topeventcleaning.SeededPulseSelector import SeededPulseSelector
load('libtopeventcleaning', False)

del load
