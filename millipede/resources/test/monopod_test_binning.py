#!/usr/bin/env python

"""
Check Bayesian Blocks binning in Millipede against a reference implementation
"""

from I3Tray import *
import os, math
from icecube import icetray, dataio, dataclasses, millipede, photonics_service, phys_services, simclasses

from monopod_test import get_pxs, InjectSource, PyPyMillipedeModule
import copy

try:
	pxs = get_pxs()
except:
	print("Can't find full-size spline tables, skipping test")
	sys.exit(0)

try:
	import numpy as np
except ImportError:
	print("This test requires Numpy, skipping test")
	sys.exit(0)

tray = I3Tray()
tray.Add(InjectSource, pxs=pxs, energy=1e2*I3Units.TeV)

def bayesian_blocks(t, weights=None, ncp_prior=4, no_empty_bins=False):
	"""Bayesian Blocks Implementation

	By Jake Vanderplas.	 License: BSD
	Based on algorithm outlined in http://adsabs.harvard.edu/abs/2012arXiv1207.5578S

	Parameters
	----------
	t : ndarray, length N
		data to be histogrammed

	Returns
	-------
	bins : ndarray
		array containing the (N+1) bin edges

	Notes
	-----
	This is an incomplete implementation: it may fail for some
	datasets.  Alternate fitness functions and prior forms can
	be found in the paper listed above.
	"""
	if weights is not None:
		assert(len(t) == len(weights) + 1)
		assert((np.diff(t) > 0).all())
		if no_empty_bins:
			t = numpy.concatenate((t[:-1][weights > 0], [t[-1]]))
			weights = weights[weights > 0]
		
		edges = np.asarray(t)
		# t = t[nn_vec>0]
		# nn_vec = nn_vec[nn_vec>0]
		nn_vec = np.asarray(weights)
		N = nn_vec.size
		block_length = edges[-1] - edges
	else:
		# copy and sort the array
		t = np.sort(t)
		N = t.size
		
		# create length-(N + 1) array of cell edges
		edges = np.concatenate([t[:1],
								0.5 * (t[1:] + t[:-1]),
								t[-1:]])
		block_length = t[-1] - edges

	best = np.zeros(N, dtype=float)
	last = np.zeros(N, dtype=int)

	#-----------------------------------------------------------------
	# Start with first data cell; add one cell at each iteration
	#-----------------------------------------------------------------
	for K in range(N):
		# Compute the width and count of the final bin for all possible
		# locations of the K^th changepoint
		width = block_length[:K + 1] - block_length[K + 1]
		
		count_vec = np.cumsum(nn_vec[:K + 1][::-1])[::-1]

		# evaluate fitness function for these possibilities
		fit_vec = count_vec * (np.log(count_vec) - np.log(width))
		fit_vec = np.where(count_vec > 0, count_vec * (np.log(count_vec) - np.log(width)), 0)
		fit_vec -= ncp_prior  # 4 comes from the prior on the number of changepoints
		fit_vec[1:] += best[:K]
		
		# find the max of the fitness: this is the K^th changepoint
		i_max = np.argmax(fit_vec)
		last[K] = i_max
		best[K] = fit_vec[i_max]

	#-----------------------------------------------------------------
	# Recover changepoints by iteratively peeling off the last block
	#-----------------------------------------------------------------
	change_points =	 np.zeros(N+1, dtype=int)
	i_cp = N+1
	ind = N
	while True:
		i_cp -= 1
		change_points[i_cp] = ind
		if ind == 0:
			break
		ind = last[ind - 1]
	change_points = change_points[i_cp:]

	return edges[change_points]

def prebin_pulses(pulses, readout_window):
	"""
	Put each pulse in a separate bin, mimicking the first pass
	in MillipedeDOMCacheMap::UpdateData()
	"""
	edges = [readout_window.start]
	charges = [0]
	for p in pulses:
		# Add one bin containing the pulse followed by
		# a second that continues to the next pulse
		edges.append(p.time)
		charges.append(p.charge)
		edges.append(p.time+p.width)
		charges.append(0)
	edges.append(readout_window.stop)
	
	# Clean any zero or negative width bins resulting from pulse width round-off error.
	lag = 0
	for q in range(len(charges)):
		if edges[q-lag+1] <= edges[q-lag]:
			assert(charges[q-lag] == 0)
			edges.remove(q-lag)
			charges.remove(q-lag)
			lag += 1
	return np.asarray(charges), np.asarray(edges)

import numpy

def stepped_path(edges, bins):
	x = numpy.zeros((2*len(edges)))
	y = numpy.zeros((2*len(edges)))
	
	x[0::2], x[1::2] = edges, edges
	y[1:-1:2], y[2::2] = bins, bins
	
	return x, y

class BinTester(PyPyMillipedeModule):
	def DoStuff(self, frame):
		pulsemap = frame['OraclePulses']
		readout_window = frame['OraclePulsesTimeRange']
		sigma = self.GetParameter('BinSigma')
		ncp_prior = 0.5*sigma**2
		for dom, cache in self.millipede.domCache.items():
			if len(pulsemap[dom]) > 0:
				charges, edges = prebin_pulses(pulsemap[dom], readout_window)
				block_edges = bayesian_blocks(edges, charges, ncp_prior)
				block_bins = numpy.histogram([p.time for p in pulsemap[dom]], weights=[p.charge for p in pulsemap[dom]], bins=block_edges)[0]
				
				assert(len(block_edges) == len(cache.time_bin_edges))
				assert((abs(block_edges - cache.time_bin_edges) < 1e-2).all())
				assert(len(block_bins) == len(cache.charges))
				assert(cache.charges.sum() == charges.sum())
				assert((cache.charges == block_bins).all())

tray.Add(BinTester, Pulses='OraclePulses', CascadePhotonicsService=pxs, BinSigma=2.5, MinTimeWidth=0)

tray.AddModule('Dump', 'dump')

tray.Execute(3+3)

