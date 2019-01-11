from icecube import icetray, gulliver
from icecube.load_pybindings import load_pybindings

icetray.load("lilliput", False)
icetray.load("gulliver-modules", False)

load_pybindings(__name__, __path__)
