import icecube
from icecube import icetray
from icecube.load_pybindings import load_pybindings
from .NoiseEngine import WithCleaners

icetray.load('NoiseEngine', False)

del icecube

