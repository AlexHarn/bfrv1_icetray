from icecube.icetray import load
load('corsika-reader', False)
del load
from .ReadCorsika import ReadCorsika

