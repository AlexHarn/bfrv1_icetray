from icecube.load_pybindings import load_pybindings
load_pybindings(__name__, __path__)

from .perturber import Perturber

from .Absorption import Absorption
from .AnisotropyScale import AnisotropyScale
from .DOMEfficiency import DOMEfficiency
from .HoleIceForward_MSU import HoleIceForward_MSU
from .HoleIceForward_Unified import HoleIceForward_Unified
from .Scattering import Scattering

# make a dict of all perturber classes
all_perturbers = {
    "Absorption"             : Absorption(),
    "AnisotropyScale"        : AnisotropyScale(),
    "DOMEfficiency"          : DOMEfficiency(),
    "HoleIceForward_MSU"     : HoleIceForward_MSU(),
    "HoleIceForward_Unified" : HoleIceForward_Unified(),
    "Scattering"             : Scattering()
}
