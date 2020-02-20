#!/usr/bin/env python

from __future__ import print_function
from icecube import dataclasses, phys_services, PROPOSAL

p = dataclasses.I3Particle(dataclasses.I3Position(0,0,-850), dataclasses.I3Direction(0,0,1), 0)
p.location_type = p.InIce
p.type = p.TauMinus
p.energy = 1e5

prop = PROPOSAL.I3PropagatorServicePROPOSAL()
prop.SetRandomNumberGenerator(phys_services.I3GSLRandomService(0))

products = []
for _ in range(20):
    p.length = 0
    daughters = prop.Propagate(p)
    products.append([pp.type for pp in daughters])

try:
    assert any([p.MuMinus in daughters for daughters in products]), "taus decay to muons even outside active volume"
except AssertionError:
    print(products)
    raise