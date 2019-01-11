#!/usr/bin/env python

from I3Tray import *
import os, math
from icecube import icetray, dataio, dataclasses, millipede, photonics_service, phys_services, simclasses

from monopod_test import get_pxs, InjectSource, PyPyMillipedeModule

try:
	pxs = get_pxs()
except:
	print("Can't find full-size spline tables, skipping test")
	sys.exit(0)

# icetray.logging.set_level_for_unit('millipede', 'TRACE');

tray = I3Tray()
tray.Add(InjectSource, pxs=pxs, energy=1e2*I3Units.TeV)

class SolverTest(PyPyMillipedeModule):
	def DoStuff(self, frame):
		seed = frame['Seed']
		sources = dataclasses.ListI3Particle([seed])
		
		response_matrix = self.millipede.GetResponseMatrix(sources)
		
		seed_llh = self.millipede.FitStatistics(sources, response_matrix)
		self.millipede.SolveEnergyLosses(sources, response_matrix)
		fit_llh = self.millipede.FitStatistics(sources, response_matrix)
		# allow for the finite tolerance of the energy solver
		tol = 1e-2
		assert seed_llh-fit_llh < tol, "LLH for best fit (%f for %g GeV) is better than seed (%f for %g GeV)" % (fit_llh, sources[0].energy, seed_llh, seed.energy)

tray.AddModule('Dump', 'dump')
tray.AddModule(SolverTest, 'monopod', Pulses='OraclePulses', CascadePhotonicsService=pxs, PhotonsPerBin=5)	


tray.Execute(3+10)

