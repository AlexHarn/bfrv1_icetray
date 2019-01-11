from icecube.load_pybindings import load_pybindings
from icecube import icetray
from icecube import dataclasses
load_pybindings(__name__,__path__)
del icetray
del load_pybindings
del dataclasses