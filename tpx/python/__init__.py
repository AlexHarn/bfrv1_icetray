from icecube.load_pybindings import load_pybindings
import icecube.dataclasses # be nice and pull in our dependencies

load_pybindings(__name__,__path__)

from .I3IceTopSLCCalibrator import I3IceTopSLCCalibrator
from . import segments

del load_pybindings
del icecube
