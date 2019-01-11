from icecube.load_pybindings import load_pybindings
load_pybindings(__name__,__path__)
del load_pybindings

from .segments import *

#try:
#  from icecube.shovelart import Scenario
#  from artists import ExplodingSpheres
#  Scenario.registerArtist(ExplodingSpheres)
#except:
#  pass
