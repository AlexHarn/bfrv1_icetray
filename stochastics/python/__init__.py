from icecube.icetray import load
from icecube.load_pybindings import load_pybindings
load_pybindings(__name__, __path__)

import modules
load('libstochastics', False)

del load

try:
    import icecube.tableio
    import converters
except ImportError:
    pass
