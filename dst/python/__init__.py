from icecube.load_pybindings import load_pybindings
import icecube.icetray # be nice and pull in our dependencies
from .extractor import *

load_pybindings(__name__,__path__)
#icecube.icetray.load('dst', True)

