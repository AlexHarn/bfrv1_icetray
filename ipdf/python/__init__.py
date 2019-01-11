from icecube.load_pybindings import load_pybindings
from icecube import icetray 

icetray.load('ipdf', False)
load_pybindings(__name__,__path__)

# remove these from the ipdf namespace
del icetray
del load_pybindings
