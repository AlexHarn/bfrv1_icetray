#!/usr/bin/env python

"""
Demo: feeding the same steps through CLSim and PPC
"""

from __future__ import print_function

from icecube import ppc, clsim, phys_services, dataclasses
from os.path import expandvars, join, isfile
from os import environ
import tempfile, shutil

import numpy

# DetectorParams = clsim.traysegments.common.setupDetector(expandvars('$I3_DATA/GCD/GeoCalibDetectorStatus_IC86_Merged.i3.gz'), DOMOversizeFactor=16)
DetectorParams = clsim.traysegments.common.setupDetector(expandvars('$I3_DATA/GCD/GeoCalibDetectorStatus_IC86_Merged.i3.gz'), DOMOversizeFactor=10)


rng = phys_services.I3GSLRandomService(0)

from icecube.ppc import MakeCLSimPropagator
ppcer = MakeCLSimPropagator(DetectorParams, UseGPUs=False, UseCPUs=True)

clsimer = clsim.traysegments.common.setupPropagators(rng, DetectorParams, UseCPUs=True)[0]

print('---> ppc granularity %d, bunch size %d' % (ppcer.workgroupSize, ppcer.maxNumWorkitems))
print('---> clsim granularity %d, bunch size %d' % (clsimer.workgroupSize, clsimer.maxNumWorkitems))

try:
    from math import gcd
except ImportError:
    from fractions import gcd
lcm = lambda a,b: a*b/gcd(a,b)
granularity = int(lcm(ppcer.workgroupSize, clsimer.workgroupSize))

stepGenerator = clsim.I3CLSimLightSourceToStepConverterAsync()

stepGenerator.SetLightSourceParameterizationSeries(DetectorParams['ParameterizationList'])
stepGenerator.SetMediumProperties(DetectorParams['MediumProperties'])
stepGenerator.SetRandomService(rng)
stepGenerator.SetWlenBias(DetectorParams['WavelengthGenerationBias'])
stepGenerator.SetMaxBunchSize(clsimer.maxNumWorkitems)
stepGenerator.SetBunchSizeGranularity(granularity)
stepGenerator.Initialize()

p = dataclasses.I3Particle()
p.type = p.EMinus
p.energy = 1e4
p.time = 0
p.pos = dataclasses.I3Position(0,0,-400)
p.dir = dataclasses.I3Direction(0,0)

for i in range(1):
	stepGenerator.EnqueueLightSource(clsim.I3CLSimLightSource(p), 0)
stepGenerator.EnqueueBarrier()

from collections import defaultdict
photons = defaultdict(clsim.I3CLSimPhotonSeries)

i = 0
while True:
	steps, markers, particleHistories, barrierWasReset = stepGenerator.GetConversionResultWithBarrierInfoAndMarkers()

	print('---> sending %d photons in bunch %d' % (sum((s.num for s in steps)), i))
	ppcer.EnqueueSteps(steps, i)
	clsimer.EnqueueSteps(steps, i)
	i += 1
	
	result_ppc =  ppcer.GetConversionResult()
	result_clsim =  clsimer.GetConversionResult()
	
	photons['ppc'].extend(result_ppc.photons)
	photons['clsim'].extend(result_clsim.photons)

	n_ppc = len(result_ppc.photons)
	n_clsim = len(result_clsim.photons)
	print('---> got {ppc: %d, clsim: %d} photons in bunch %d (ppc/clsim=%.2f)' % (n_ppc, n_clsim, result_ppc.identifier, n_ppc/float(n_clsim)))
	
	if barrierWasReset:
		break

# numpy.savez('photons', **photons)
