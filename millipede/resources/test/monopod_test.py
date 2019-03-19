#!/usr/bin/env python

from I3Tray import *
import os, math
from icecube import icetray, dataio, dataclasses, millipede, photonics_service, phys_services, simclasses

# Monkey-patch for Python < 2.6
if not hasattr(math, "isnan"):
	def isnan(num):
		return num != num
	math.isnan = isnan

class HoboNoiseGenerator(icetray.I3Module):
	def __init__(self, ctx):
		icetray.I3Module.__init__(self, ctx)
		self.AddOutBox("OutBox")
	def Configure(self):
		self.rng = self.context["I3RandomService"]

	def DAQ(self, frame):
		mchits = simclasses.I3MCPESeriesMap()
		twindow = frame['OraclePulsesTimeRange']
		geometry = frame['I3Geometry']
		calibration = frame['I3Calibration']
		detector_status = frame['I3DetectorStatus']
		for om, cal in calibration.dom_cal:
			if not om in detector_status.dom_status:
				continue
			ds = detector_status.dom_status[om]
			if ds.pmt_hv < 100*I3Units.V or math.isnan(ds.pmt_hv):
				continue
			if geometry.omgeo[om].omtype != dataclasses.I3OMGeo.IceCube:
				continue
			rate = cal.dom_noise_rate
			
			mean = rate*(twindow.stop-twindow.start)
			times = sorted([self.rng.uniform(twindow.start, twindow.stop) for i in range(self.rng.poisson(mean))])
			hits = simclasses.I3MCPESeries()
			for t in times:
				hit = simclasses.I3MCPE()
				hit.npe = 1
				hit.time = t
				hits.append(hit)
			mchits[om] = hits
		frame['I3MCPESeriesMap'] = mchits
		self.PushFrame(frame)

@icetray.traysegment
def InjectSource(tray, name, pxs, energy=1e2*I3Units.TeV):
	
	tray.context['I3RandomService'] = phys_services.I3GSLRandomService(42)
	
	tray.AddModule('I3InfiniteSource', 'reader', Prefix=os.getenv('I3_TESTDATA') + '/sim/GeoCalibDetectorStatus_IC86.55697_corrected_V2.i3.gz')
	def set_sane_noise_rate(frame):
		for k, domcal in frame['I3Calibration'].dom_cal.items():
			if not domcal.dom_noise_rate > 0:
				domcal.dom_noise_rate = 850*I3Units.hertz
				frame['I3Calibration'].dom_cal[k] = domcal
	tray.Add(set_sane_noise_rate, Streams=[icetray.I3Frame.Calibration])
	tray.AddModule(lambda fr: False, 'dropold', Streams=[icetray.I3Frame.Physics])
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
		part.length = 0
		part.fit_status = dataclasses.I3Particle.OK
		tree.append_child(prim, part)
		fr['I3MCTree'] = tree
		fr['Seed'] = part

		header = dataclasses.I3EventHeader()
		fr['I3EventHeader'] = header
	
		timewindow = dataclasses.I3TimeWindow(0, 10000)
		fr['OraclePulsesTimeRange'] = timewindow

	tray.AddModule(cascade, 'cascade', Streams=[icetray.I3Frame.DAQ])
	tray.Add(HoboNoiseGenerator)
	tray.AddModule('I3PhotonicsHitMaker', 'hm', CascadeService=pxs, TrackService=pxs, HCList='ParticleList')

	def OracleFE(fr, mchits='I3MCPESeriesMap', mcpulses='OraclePulses'):
		if not mchits in fr:
			return

		reco = dataclasses.I3RecoPulseSeriesMap()
		qtot = 0
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
				qtot += pulse.charge
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
		# print('NChan: %d' % len(reco))
		# print('QTot: %f' % qtot)
		fr[mcpulses] = reco
	tray.AddModule(OracleFE, 'oracle', Streams=[icetray.I3Frame.DAQ])
	tray.AddModule('I3NullSplitter', 'splitter')

class PyPyMillipedeModule(icetray.I3Module):
	"""
	Shim module for poking at the Millipede core from Python
	"""
	def __init__(self, ctx):
		icetray.I3Module.__init__(self, ctx)
		self.AddOutBox("OutBox")
		self.millipede = millipede.PyPyMillipede(ctx)
		# bridge contexts
		config = self.millipede.GetConfiguration()
		for k in config.keys():
			self.AddParameter(k, config.descriptions[k], self.millipede.GetParameter(k))
	def Configure(self):
		config = self.millipede.GetConfiguration()
		for k in config.keys():
			self.millipede.SetParameter(k, self.GetParameter(k))
	def Physics(self, frame):
		self.millipede.DatamapFromFrame(frame)
		self.DoStuff(frame)
		self.PushFrame(frame)

def get_pxs():
	base = os.path.expandvars('$I3_DATA/photon-tables/splines/ems_mie_z20_a10.%s.fits')
	if not os.path.exists(base % "abs"):
		raise errno.ENOENT((base % "abs") + " does not exist!")
	if not os.path.exists(base % "prob"):
		raise errno.ENOENT((base % "prob") + " does not exist!")
	
	return photonics_service.I3PhotoSplineService(base % 'abs', base % 'prob', 0.)

if __name__ == "__main__":
	tray = I3Tray()

	pxs = photonics_service.I3DummyPhotonicsService()

	tray.Add(InjectSource, pxs=pxs, energy=100*I3Units.TeV)

	tray.AddModule('Monopod', 'monopod', Pulses='OraclePulses', Output='RecoCascade', CascadePhotonicsService=pxs, seed='Seed', PhotonsPerBin=5)	

	import unittest
	class SanityCheck(unittest.TestCase):
		key = 'RecoCascade'
		seedkey = 'Seed'

		def testKeys(self):
			self.assert_(self.key in self.frame, "The output actually shows up in the frame.")
			# print(self.frame[self.key + 'FitParams'])
		def testEnergy(self):
			reco = self.frame[self.key]
			seed = self.frame[self.seedkey]
			print('Reco Energy:  %.2f GeV' % reco.energy)
			print('Seed Energy:  %.2f GeV' % seed.energy)
			print('      Error: %+.2f%%' % (100*((reco.energy - seed.energy)/seed.energy)))
			
			self.assert_(abs(reco.energy - seed.energy)/seed.energy < 0.1, "Reco energy to within 10%")

	# tray.AddModule('Dump', 'dump')
	tray.AddModule(icetray.I3TestModuleFactory(SanityCheck), 'testy')
	
	tray.Execute(3+10)
	
