'''
Snippet to map the SnowStorm Parameters from its branch dependent objects
to easy-accesible and easy-table-writable objects in the frame
'''
from icecube.dataclasses import I3VectorDouble
from icecube import snowstorm

def map_parameters(frame):
    for i, key in enumerate(frame["SnowstormParametrizations"]):
        parameters = frame["SnowstormParameters"]
        parameter_range = frame["SnowstormParameterRanges"][i]
        frame[key] = I3VectorDouble(parameters[parameter_range.first:parameter_range.second])
