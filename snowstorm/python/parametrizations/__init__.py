from .Absorption import *
from .AnisotropyScale import *
from .DOMEfficiency import *
from .HoleIceForward_MSU import *
from .HoleIceForward_Unified import *
from .Scattering import *


# make a dict of all parametrizations
all_parametrizations = {
    "Absorption"             : Absorption(),
    "AnisotropyScale"        : AnisotropyScale(),
    "DOMEfficiency"          : DOMEfficiency(),
    "HoleIceForward_MSU"     : HoleIceForward_MSU(),
    "HoleIceForward_Unified" : HoleIceForward_Unified(),
    "Scattering"             : Scattering()
}
