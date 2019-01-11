#!/usr/bin/env python

"""
Ensure that MonopodFit gets the right answer (for a fairly loose definition of "right")
"""

from I3Tray import *
import os, math
from icecube import icetray, dataio, dataclasses, millipede, photonics_service, phys_services, simclasses
from icecube.icetray import I3Units

from monopod_test import get_pxs, InjectSource, PyPyMillipedeModule
import copy

try:
	pxs = get_pxs()
except:
	print("Can't find full-size spline tables, skipping test")
	sys.exit(0)

tray = I3Tray()
tray.Add(InjectSource, pxs=pxs, energy=1e2*I3Units.TeV)

def nudge_seed(frame):
	seed = frame['Seed']
	del frame['Seed']
	# nudge seed slightly so that the minimizer starts on a slope
	seed.pos.x = seed.pos.x + 1
	seed.pos.y = seed.pos.y + 1
	seed.pos.z = seed.pos.z + 1
	sd = dataclasses.I3Direction(seed.dir.zenith+.2, seed.dir.azimuth+.2)
	seed.dir = sd
	frame['Seed'] = seed
	print('original seed')
	print(frame['Seed'])
	
tray.Add(nudge_seed, Streams=[icetray.I3Frame.Physics])

# icetray.logging.set_level_for_unit("I3GulliverLBFGSB", "TRACE")
# icetray.logging.set_level_for_unit("lilliput", "TRACE")

from icecube import gulliver
class ScipyMinimizer(gulliver.I3Minimizer):
	methods = {
		# name		   gradient hessian
		'Nelder-Mead': (False,	False),
		'Powell':	   (False,	False),
		'CG':		   (True,	False),
		'BFGS':		   (True,	False),
		'Newton-CG':   (True,	True),
		'Anneal':	   (False,	False),
		'L-BFGS-B':	   (True,	False),
		'TNC':		   (False,	False),
		'COBYLA':	   (False,	False),
		'SLSQP':	   (False,	False),
		'dogleg':	   (True,	True),
		'trust-ncg':   (True,	True),
	}
	def __init__(self, name, method='L-BFGS-B', options=dict()):
		super(ScipyMinimizer, self).__init__()
		if not method in self.methods:
			raise ValueError("Unknown algorithm %s" % method)
		if self.methods[method][1]:
			raise ValueError("%s uses hessians, which we don't know how to calculate" % method)
		self.options = options
		self.method = method
		self.name = name
		self.tolerance = 1e7
		self.max_iterations = 100
		self.plot = True
	def GetName(self):
		return self.name
	def UsesGradient(self):
		return self.methods[self.method][0]
	def Minimize(self, llh, parspecs):
		x0 = []
		bounds = []
		for spec in parspecs:
			dimbounds = [None, None]
			if spec.minval != spec.maxval:
				if isfinite(spec.minval):
					dimbounds[0] = spec.minval
				if isfinite(spec.maxval):
					dimbounds[1] = spec.maxval
			bounds.append(tuple(dimbounds))
			x0.append(spec.initval)
		
		xs = []
		fs = []
		dfs = []
		
		def fdf(x, grad):
			vec = icetray.vector_double()
			vec.extend(x)
			res = llh(vec, grad)
			if grad:
				f, df = res[0], array(res[1])
				# f = llh(vec, False)
				xs.append(x.copy())
				fs.append(f)
				dfs.append(df)
				icetray.logging.log_info("{x} f {f} df {df}".format(**locals()), unit="ScipyMinimizer")
				
			else:
				f, df = res, None
			
			if grad:
				return df
			else:
				return f
		f = lambda x: fdf(x, False)
		df = lambda x: fdf(x, True)
		
		def callback(x):
			pass
		
		res = minimize(f, x0, method=self.method, jac=df, bounds=bounds,
			tol=self.tolerance, options=self.options)
		
		import pylab, numpy
		from matplotlib.colors import Normalize
		from matplotlib.cm import jet as cmap
		xs = numpy.array(xs)
		fs = numpy.array(fs)
		dfs = numpy.array(dfs)
		dxs = numpy.concatenate((0.9*numpy.diff(xs[:,0]), [1]))
		colors = cmap(numpy.linspace(0, 1, len(xs)))
		print(xs[:,0])
		pylab.scatter(xs[:,0], fs, color=colors)
		for x, y, dx, dy in zip(xs[:,0], fs, dxs, dfs[:,0]):
			pylab.arrow(x, y, dx, dx*dy)
		# xfine = numpy.linspace(-2, 2, 501)
		xfine = numpy.linspace(.5, .7, 501)
		ffine = [float(f([x])) for x in xfine]
		pylab.plot(xfine, ffine)
		pylab.show()
		
		minres = gulliver.I3MinimizerResult(len(x0))
		minres.converged = res.success
		minres.minval = res.fun
		for i, x in enumerate(res.x):
			minres.par[i] = x
		
		return minres

tray.Add(millipede.MonopodFit, 'RecoCascade', Pulses='OraclePulses', CascadePhotonicsService=pxs, seed='Seed', MinTimeWidth=3, BinSigma=3.5, Minimizer="LBFGSB")
tray.Add('I3LogLikelihoodCalculator', 'SeedLikelihood', LogLikelihoodService='RecoCascade_likelihood', FitName='Seed')

import unittest
class SanityCheck(unittest.TestCase):
	key = 'RecoCascade'
	seedkey = 'Seed'

	def testKeys(self):
		self.assert_(self.key in self.frame, "The output actually shows up in the frame.")
		print(self.frame[self.key + 'FitParams'])
	def testEnergy(self):
		import math
		reco = self.frame[self.key]
		seed = self.frame[self.seedkey]
		print(reco)
		print('Reco Energy: %.2f GeV' % reco.energy)
		print('Seed Energy: %.2f GeV' % seed.energy)
		print('Reco vertex offset: %s' % (reco.pos-seed.pos))
		print('Reco opening angle: %.1f deg' % (math.acos(reco.dir*seed.dir)/I3Units.degree))
		self.assert_(abs(reco.energy - seed.energy)/seed.energy < 0.06, "Reco energy to within 6%")
	def testLikelihood(self):
		reco = self.frame[self.key+'FitParams']
		seed = self.frame[self.seedkey+'Likelihood']
		self.assertTrue(reco.logl < seed.logl+1e-2, "Best fit has a better likelihood (%f) than the seed (%f)" % (reco.logl, seed.logl))
			
	def testStatus(self):
		reco = self.frame[self.key]
		self.assertEqual(reco.fit_status, reco.OK, "Fit converged")

tray.AddModule('Dump', 'dump')
tray.AddModule(icetray.I3TestModuleFactory(SanityCheck), 'testy')

tray.Execute(3+10)

