#!/usr/bin/env python

from I3Tray import *
import os, math
import numpy
from icecube import icetray, dataio, dataclasses, millipede, photonics_service, phys_services, simclasses

pxs = photonics_service.I3DummyPhotonicsService()

tray = I3Tray()
tray.context['I3RandomService'] = phys_services.I3GSLRandomService(42)
tray.AddModule('I3InfiniteSource', 'reader', Prefix=os.getenv('I3_TESTDATA') + '/sim/GeoCalibDetectorStatus_IC86.55697_corrected.i3.gz')
tray.AddModule(lambda fr: False, 'dropold', Streams=[icetray.I3Frame.Physics])

energy = 100*I3Units.TeV # Nice and bright

def cascade(fr):
	part = dataclasses.I3Particle()
	part.time = 50
	part.type = dataclasses.I3Particle.NuE
	part.pos.x = -50000; part.pos.y = -500000; part.pos.z = -500000
	part.dir = dataclasses.I3Direction(0.5, 0.8)
	part.energy = energy
	
	tree = dataclasses.I3MCTree()
	prim = part
	tree.add_primary(prim)

	part = dataclasses.I3Particle()
	part.time = 50
	part.type = dataclasses.I3Particle.EMinus
	part.shape = dataclasses.I3Particle.Cascade
	part.location_type = dataclasses.I3Particle.InIce
	part.pos.x = 0; part.pos.y = 0; part.pos.z = 0
	part.dir = dataclasses.I3Direction(0.5, 0.8)
	part.energy = energy
	tree.append_child(prim, part)

	part = dataclasses.I3Particle()
	part.time = 250
	part.type = dataclasses.I3Particle.EMinus
	part.shape = dataclasses.I3Particle.Cascade
	part.location_type = dataclasses.I3Particle.InIce
	part.pos.x = -150; part.pos.y = 200; part.pos.z = 160
	part.dir = dataclasses.I3Direction(1.7, 0.8)
	part.energy = energy*1.5
	tree.append_child(prim, part)
	
	fr['I3MCTree'] = tree

	header = dataclasses.I3EventHeader()
	fr['I3EventHeader'] = header
	
	timewindow = dataclasses.I3TimeWindow(0, 10000)
	fr['OraclePulsesTimeRange'] = timewindow
	
tray.AddModule(cascade, 'cascade', Streams=[icetray.I3Frame.DAQ])
from monopod_test import HoboNoiseGenerator
tray.Add(HoboNoiseGenerator)
tray.AddModule('I3PhotonicsHitMaker', 'hm', CascadeService=pxs, TrackService=pxs, HCList='ParticleList')

def OracleFE(fr, mchits='I3MCPESeriesMap', mcpulses='OraclePulses'):
	if not mchits in fr:
		return

	reco = dataclasses.I3RecoPulseSeriesMap()
	for i in fr[mchits]:
		try:
			meanq = dataclasses.mean_spe_charge(fr['I3DetectorStatus'].dom_status[i[0]], fr['I3Calibration'].dom_cal[i[0]])
		except KeyError:
			continue
		reco[i[0]] = dataclasses.I3RecoPulseSeries()
		pulses = []
		for mchit in i[1]:
			pulse = dataclasses.I3RecoPulse()
			pulse.width = 3.3
			pulse.charge = mchit.npe*meanq
			pulse.time = mchit.time
			pulses.append(pulse)
		pulses.sort(key=lambda p: p.time)
		k = 0
		while k < len(pulses):
			pulse = pulses[k]
			k += 1
			while k < len(pulses) and pulses[k].time - pulse.time < pulse.width:
				pulse.charge += pulses[k].charge
				k += 1
			reco[i[0]].append(pulse)
	print('NChan: %d' % len(reco))
	fr[mcpulses] = reco
tray.AddModule(OracleFE, 'oracle', Streams=[icetray.I3Frame.DAQ])
tray.AddModule('I3NullSplitter', 'splitter')
tray.AddModule('PyMillipede', 'pym', Pulses='OraclePulses', Output='Millipede', CascadePhotonicsService=pxs, MuonPhotonicsService=pxs, Hypothesis=lambda fr: fr['ParticleList'], PhotonsPerBin=-1)
tray.AddModule('MillipedeFisherMatrixCalculator', Pulses='OraclePulses', Seed='Millipede', Output='MillipedeFisherMatrix', CascadePhotonicsService=pxs, MuonPhotonicsService=pxs)

import unittest
class SanityCheck(unittest.TestCase):
	key = 'Millipede'
	seedkey = 'ParticleList'
	matrixKey = 'MillipedeFisherMatrix'

	def testKeys(self):
		self.assert_(self.key in self.frame, "The output actually shows up in the frame.")
		self.assert_(self.matrixKey in self.frame, "The fisher matrix shows up in the frame.")
	def testEnergy(self):
		reco = self.frame[self.key]
		seed = self.frame[self.seedkey]
		self.assert_(len(list(zip(seed,reco))) > 1, "More than one particle")

		for s,r in zip(seed,reco):
			print('Reco Energy: %.2f GeV' % r.energy)
			print('Seed Energy: %.2f GeV' % s.energy)
			self.assert_(abs(r.energy - s.energy)/s.energy < 0.1, "Reco energy to within 10%")
    	
	def testLikelihoods(self):
		# Make sure the likelihood of the bit (which is supposed to
		# be maximal) is better than the seed
		seedllh = self.frame[self.key + 'SeedParams'].logl
		fitllh = self.frame[self.key + 'FitParams'].logl
		print('Seed LLH: %.2f' % seedllh)
		print('Fit LLH: %.2f' % fitllh)
		self.assert_(fitllh <= seedllh, "Maximum likelihood is maximal")

	def testFisherMatrix(self):
		fisherMatrix = numpy.asmatrix(self.frame[self.matrixKey])
		self.assert_(fisherMatrix.shape == (2, 2), "Fisher matrix is 2x2")
		# The diagonal elements should be positive
		self.assert_(fisherMatrix.diagonal().min() > 0,
					"Entries of fisher matrix diagonal are positive")
		try:
			inv = numpy.linalg.inv(fisherMatrix)
			# The bound on the energy estimate should be better than 10%
			seed = self.frame[self.seedkey]
			variances = inv.diagonal().tolist()[0]
			for s, var in zip(seed, variances):
				self.assert_(math.sqrt(var) / s.energy < 0.1,
							 "Bound on energy estimate within 10%")
		except numpy.linalg.LinAlgError:
			self.fail("Unable to invert fisher matrix")

tray.AddModule('Dump', 'dump')
tray.AddModule(icetray.I3TestModuleFactory(SanityCheck), 'testy')

tray.Execute(6)

