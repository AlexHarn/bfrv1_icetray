from icecube import icetray 
from icecube.load_pybindings import load_pybindings

icetray.load('paraboloid', False)
load_pybindings(__name__,__path__)

try:
    import icecube.tableio
    import converters
except ImportError:
    pass



