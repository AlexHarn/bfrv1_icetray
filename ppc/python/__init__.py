from __future__ import print_function
from icecube.icetray import load
from icecube import dataclasses

load('libppc', False)
del load

#helper function to build the tau_dnde vector needed /fhl
def tau_dnde_builder(it):
    tmp = []
    for i in it:
        if len(i) != 2:
            raise ValueError("All elements must have exactly two parameters (tau and dN/dE)!")
        if i[0] <= 0:
            raise ValueError("tau must be greater than 0")
        tmp.append(dataclasses.make_pair(i[0],i[1]))
    return dataclasses.I3VectorDoubleDouble(tmp)
