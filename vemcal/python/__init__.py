from icecube import icetray
icetray.load("libvemcal", False)
from icecube.load_pybindings import load_pybindings
load_pybindings(__name__,__path__)
