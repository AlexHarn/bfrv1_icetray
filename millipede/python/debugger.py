from __future__ import print_function
from icecube import icetray, dataclasses, photonics_service
import numpy, os

class PhotoDebugger(object):
	def __init__(self, CascadePhotonicsService=None, MuonPhotonicsService=None):
		self.cascadep = CascadePhotonicsService
		self.muonp = CascadePhotonicsService
	
	def getPhotonDistribution(self, source, position, time_edges, geotime=False):
		"""
		Get photon distribution for a single source or a collection of sources.
		"""
		if isinstance(source, dataclasses.I3Particle):
			return self._getPhotonDistribution(source, position, time_edges, geotime)
		else:
			total = numpy.zeros(time_edges.size-1)
			geotimes = []
			for s in source:
				d = self._getPhotonDistribution(s, position, time_edges, geotime)
				if geotime:
					q, gt = d
					geotimes.append(gt)
				else:
					q = d
				total += q
			if geotime:
				return total, min(geotimes)
			else:
				return total
		
	def _getPhotonDistribution(self, source, position, time_edges, geotime=False):
		"""
		Get photon distribution for a single source.
		"""
		if source.energy == 0:
			if geotime:
				return numpy.zeros(time_edges.size-1), numpy.inf
			else:
				return numpy.zeros(time_edges.size-1)
		
		psource = photonics_service.PhotonicsSource(source)
		# psource.energy = 1.
		
		edges = numpy.asarray(time_edges)
		
		if source.is_cascade:
			pxs = self.cascadep
		elif source.is_track:
			pxs = self.muonp
		
		pxs.SelectModuleCoordinates(*position)
		meanamp, edistance, gt = pxs.SelectSource(psource)
		
		if meanamp <= 0:
			quantiles = numpy.zeros(edges.size - 1)
		else:
			quantiles = pxs.GetProbabilityQuantiles(edges, source.time + gt, False)
		# print quantiles.sum(), source.energy
		if geotime:
			return meanamp*quantiles, gt + source.time
		else:
			return meanamp*quantiles
		
def stepped_path(edges, bins):
	x = numpy.zeros((2*len(edges)))
	y = numpy.zeros((2*len(edges)))
	
	x[0::2], x[1::2] = edges, edges
	y[1:-1:2], y[2::2] = bins, bins
	
	return x, y
		
class Plotsy(icetray.I3Module):
	def __init__(self, ctx):
		super(Plotsy, self).__init__(ctx)
		
		self.AddOutBox("OutBox")
		self.AddParameter("CascadePhotonicsService", "", None)
		self.AddParameter("Pulses", "", None)
		self.AddParameter("Hypothesis", "", None)
		self.AddParameter("ExcludedDOMs", "Set of keys containing lists of OMKeys",
				["BadDomsList", "CalibrationErrata", "SaturationWindows"])
		
	def Configure(self):
		
		self.pdf = PhotoDebugger(self.GetParameter("CascadePhotonicsService"))
		self.pulses = self.GetParameter("Pulses")
		self.hypo = self.GetParameter("Hypothesis")
		self.baddoms = self.GetParameter("ExcludedDOMs")
		
	def Physics(self, frame):
		import pylab, operator
		
		geomap = frame['I3Geometry'].omgeo
		baddoms = [frame[key] for key in self.baddoms]
		# flatten the baddoms list
		tmplist = []
		for oms in baddoms:
			if isinstance(oms, dataclasses.I3TimeWindowSeriesMap):
				tmplist += oms.keys()
			else:
				tmplist += oms
		baddoms = tmplist
		header = frame['I3EventHeader']
		pulsemap = dataclasses.I3RecoPulseSeriesMap.from_frame(frame, self.pulses)
		hypo = frame[self.hypo]
		if isinstance(hypo, dataclasses.I3Particle):
			hypo = [hypo]
		qtot = [(om, sum([p.charge for p in pulses])) for om, pulses in pulsemap.items() if not om in baddoms]
		qtot.sort(key=operator.itemgetter(1))
		for om, qt in qtot[-10::][::-1]:
			pulses = pulsemap[om]
			tmin = pulses[0].time
			tmax = pulses[-1].time
			edges = numpy.arange(tmin-100, tmax+100, 100.*icetray.I3Units.ns)
			dt = numpy.diff(edges)
			
			fig = pylab.figure()
			contribs = []
			geotimes = []
			labels = []
			for source in hypo:
				if source.energy > 0:
					contrib, geotime = self.pdf.getPhotonDistribution(source, geomap[om].position, edges, geotime=True)
					contribs.append(contrib)
					geotimes.append(geotime)
					labels.append('%.1e GeV %s t=%.1f ns' % (source.energy, source.type, source.time))
			contribs.append(sum(contribs))
			geotimes.append(min(geotimes))
			labels.append('Total')
			
			for contrib, geotime, label in zip(contribs, geotimes, labels):
				x, y = stepped_path(edges, contrib)
				pylab.plot(x, y, label=label)
				
			dQ = numpy.histogram([p.time for p in pulses], bins=edges, weights=[p.charge for p in pulses])[0]
			pylab.scatter(0.5*(edges[1:]+edges[:-1]), dQ, s=20, c='b', marker='+', label=self.pulses)
				
			pylab.legend(loc='best', prop=dict(size='small'))
			pylab.xlabel('Time [$\\mu$ s]')
			pylab.ylabel('Predicted photon flux [PE/%.1f ns]' % dt[0])
			pylab.title('Event %d/%d %s: OM%s' % (header.run_id, header.event_id, om, self.hypo), size='small')
			
		pylab.show()
		
		self.PushFrame(frame)

class DOMPlot(object):

	def __init__(self, ax, wfs, om, pos, cal, stat, pdf, simkludge=False, **kwargs):
		self.ax = ax
		self.om = om
		self.pos = pos
		self.cal = cal
		self.stat = stat
		self.pdf = pdf
		self.simkludge=simkludge
		self.trange = None
		
		self.add_waveforms(wfs, **kwargs)
					
	def set_label(self, text, **kwargs):
		from mpl_toolkits.axes_grid.anchored_artists import AnchoredText
		at = AnchoredText(text, loc=2, prop=dict(size=5), frameon=True)
		# at.patch.set_boxstyle("round,pad=0.,rounding_size=0.2")
		self.ax.add_artist(at)
	
	@staticmethod
	def get_time_range(wfs, sources=None, pos=None):
		from icecube.wavereform.plotter import WaveformPlotter
		trange = WaveformPlotter.get_time_range(wfs)
		
		if sources and pos:
			if isinstance(sources, dataclasses.I3Particle):
				sources = [sources]
			min_time = float('inf')
			for source in sources:
				t0 = source.time + source.pos.calc_distance(pos)/(dataclasses.I3Constants.c/dataclasses.I3Constants.n_ice_group) - 50
				if t0 < min_time:
					min_time = t0
			trange = (min(trange[0], min_time), trange[1])
		return trange
		
	@staticmethod
	def wflabel(wf):
		return ('%s%d/%d' % (wf.digitizer, wf.source_index, wf.channel))
	
	def add_exclusions(self, windows):
		import matplotlib.transforms as mtransforms
		from icecube.icetray import I3Units
		trans = mtransforms.blended_transform_factory(self.ax.transData, self.ax.transAxes)		
		lo, hi = self.ax.get_xlim()
		edges = numpy.arange(lo*I3Units.microsecond, hi*I3Units.microsecond, 1)
		mask = numpy.zeros(edges.shape, dtype=bool)
		for w in windows:
			mask[(edges < w.stop)&(edges >= w.start)] = True
		if mask.any():
			self.ax.fill_between(edges/icetray.I3Units.microsecond, 0, 1, where=mask,
			    facecolor='red', edgecolor=None, alpha=0.3, transform=trans, label='nonlinear PMT response')
	
	def add_waveforms(self, wfs, **kwargs):
		from icecube.wavereform.plotter import WaveformPlotter
				
		for wf in wfs:
			WaveformPlotter.plot_wf(self.ax, wf, label=self.wflabel(wf), **kwargs)
			
		self.wfs = list(wfs)
		
	def add_source(self, source, **plot_kwargs):
		from icecube.wavereform.plotter import WaveformPlotter
		
		trange = self.get_time_range(self.wfs, source, self.pos)
		edges = numpy.arange(trange[0], trange[1], 1.)
		
		contrib, geotime = self.pdf.getPhotonDistribution(source, self.pos, edges, geotime=True)
		if contrib.sum() == 0:
			return
			
		# XXX HACK: scale to account for cable shadow and amplification eff.
		contrib *= 0.9
		contrib *= 0.85
		
		pulses = WaveformPlotsy.potemkin_pulses(edges, contrib)
		color = plot_kwargs.pop('color', None)
		if not color:
			color = next(self.ax._get_lines.color_cycle)
		label = plot_kwargs.pop('label', '')
		for wf in self.wfs:
			WaveformPlotter.plot_pulses(self.ax, trange, wf, pulses, self.cal, self.stat,
			    use_domsimulator_hacks=self.simkludge,
			    color=color, label='%s (%s)' % (label, self.wflabel(wf)), **plot_kwargs)
		self.ax.axvline(geotime/icetray.I3Units.microsecond, color=color, **plot_kwargs)
		
		
class PulseyDOMPlot(DOMPlot):
	
	# def __init__(self, *args, **kwargs):
	# 	super(PulseyDOMPlot, self).__init__(*args, **kwargs)
	
	binwidth = 50.
	
	def add_waveforms(self, pulses, **kwargs):
		import dashi, pylab
		dashi.visual()
		
		tic = icetray.I3Units.microsecond
		times = numpy.array([p.time for p in pulses])
		weights = [p.charge for p in pulses]
		
		self._trange = (times.min()-100, times.min()+500)
		
		bins = numpy.arange(self._trange[0]/tic, self._trange[1]/tic, self.binwidth/tic)
		h = dashi.factory.hist1d(times/tic, bins=bins, weights=weights)
		# we want the pure counting error on the bin content, not
		# the statistical error in the weighted sum
		h._h_squaredweights[:] = h._h_bincontent
		
		pylab.sca(self.ax)
		h.scatter(color='k', label='Exp. data')
			
		# self.wfs = list(wfs)
		
	def add_simsource(self, source, hits, **plot_kwargs):
		import dashi, pylab
		
		times = numpy.array([p.time for p in hits[self.om]])
		weights = numpy.array([p.weight for p in hits[self.om]])
		
		tic = icetray.I3Units.microsecond
		bins = numpy.arange(self._trange[0]/tic, self._trange[1]/tic, self.binwidth/tic)

		h = dashi.factory.hist1d(times/tic, bins=bins, weights=weights)
		# we want the pure counting error on the bin content, not
		# the statistical error in the weighted sum
		h._h_squaredweights[:] = h._h_bincontent
				
		pylab.sca(self.ax)
		h.line(**plot_kwargs)
		
	def add_source(self, source, **plot_kwargs):
		from icecube.wavereform.plotter import WaveformPlotter
		
		edges = numpy.arange(self._trange[0], self._trange[1], 1.)
		
		contrib, geotime = self.pdf.getPhotonDistribution(source, self.pos, edges, geotime=True)
		if contrib.sum() == 0:
			return
			
		# XXX HACK: scale to account for cable shadow and amplification eff.
		contrib *= 0.9
		contrib *= 0.85
		
		tic = icetray.I3Units.microsecond
		centers = 0.5*(edges[1:]+edges[:-1])
		
		color = plot_kwargs.pop('color', None)
		if not color:
			color = next(self.ax._get_lines.color_cycle)
		label = plot_kwargs.pop('label', '')
		
		self.ax.plot(centers/tic, self.binwidth*contrib, label=label, color=color)
		# self.ax.axvline(geotime/tic, color=color, **plot_kwargs)
		
class WaveformPlotsy(Plotsy):
	"""
	For more visual pop, plot refolded pulses instead of raw pulse times
	"""
	def __init__(self, ctx):
		super(WaveformPlotsy, self).__init__(ctx)
		
		self.AddParameter("Waveforms", "", None)
		self.AddParameter("SaturationWindows", "", None)
		
		self.AddParameter("DebugStore", "", None)
		self.AddParameter("FilenameBase", "", None)
		self.AddParameter("NPlots", "", 30)
		
		
	def Configure(self):
		super(WaveformPlotsy, self).Configure()
		
		self.waveforms = self.GetParameter("Waveforms")
		self.saturation_windows = self.GetParameter("SaturationWindows")
		self.debug_store = self.GetParameter("DebugStore")
		self.fname_base = self.GetParameter("FilenameBase")
		self.nplots = self.GetParameter("NPlots")
	
	@staticmethod
	def potemkin_pulses(edges, amplitudes):
		centers = 0.5*(edges[1:]+edges[:-1])
		# centers += 1.5*3.3
		series = dataclasses.I3RecoPulseSeries()
		for a, c in zip(amplitudes, centers):
			if a > 0:
				p = dataclasses.I3RecoPulse()
				p.time = c
				p.charge = a
				series.append(p)
		return series
		
	@staticmethod
	def makeDOMPlot(ax, wfs, sources, pdf, pos, cal, stat, labels=None, simkludge=False):
		from icecube.wavereform.plotter import WaveformPlotter
		
		trange = WaveformPlotter.get_time_range(wfs)
		
		min_time = float('inf')
		for source in sources:
			t0 = source.time + source.pos.calc_distance(pos)/(dataclasses.I3Constants.c/dataclasses.I3Constants.n_ice_group) - 50
			if t0 < min_time:
				min_time = t0
		trange = (min(trange[0], min_time), trange[1])
		edges = numpy.arange(trange[0], trange[1], 1*icetray.I3Units.ns)
		
		contribs = []
		geotimes = []

		for source in sources:
			contrib, geotime = pdf.getPhotonDistribution(source, pos, edges, geotime=True)
			if contrib.sum() == 0:
				continue
			# XXX HACK: scale to account for cable shadow and amplification eff.
			contrib *= 0.9
			contrib *= 0.85
			contribs.append(contrib)
			geotimes.append(geotime)
			d = source.pos.calc_distance(pos)
		
		if labels is None:
			labels = [None]*len(contribs)		
		
		wflabel = lambda wf: ('%s%d/%d' % (wf.digitizer, wf.source_index, wf.channel))
		
		for wf in wfs:
			WaveformPlotter.plot_wf(ax, wf, label=wflabel(wf))
			#WaveformPlotter.plot_pulses(ax, trange, wf, pulses, cal, stat,
			#    use_domsimulator_hacks=simkludge, label=self.pulses, ls=':', color='k')
			
		from itertools import cycle
		colors = cycle(['b', 'r', 'g', 'c', 'm', 'y', 'k'])
		for contrib, geotime, label, color in zip(contribs, geotimes, labels, colors):
			pulses = WaveformPlotsy.potemkin_pulses(edges, contrib)
			for wf in wfs:
				WaveformPlotter.plot_pulses(ax, trange, wf, pulses, cal, stat,
				    use_domsimulator_hacks=simkludge,
				    color=color, label='%s (%s)' % (label, wflabel(wf)))
			ax.axvline(geotime/icetray.I3Units.microsecond, color=color)
		
	def Physics(self, frame):
		from icecube.wavereform.plotter import WaveformPlotter
		import pylab, operator
		
		geomap = frame['I3Geometry'].omgeo
		# baddoms = frame['BadDomsListSLC']
		baddoms = []
		header = frame['I3EventHeader']
		
		calibration = frame['I3Calibration']
		status = frame['I3DetectorStatus']
		
		simkludge = True if 'I3MCTree' in frame else False
		
		try:
			saturation_windows = frame[self.saturation_windows]
		except KeyError:
			from icecube.millipede import I3TimeWindowSeriesMap
			saturation_windows = I3TimeWindowSeriesMap()
		wfmap = frame[self.waveforms]
		pulsemap = dataclasses.I3RecoPulseSeriesMap.from_frame(frame, self.pulses)
		hypo = frame[self.hypo]
		if isinstance(hypo, dataclasses.I3Particle):
			hypo = [hypo]
		# qtot = [(om, sum([p.charge for p in pulses])) for om, pulses in pulsemap.items() if not om in baddoms]
		qtot = [(om, sum([p.charge for p in pulses])) for om, pulses in pulsemap.items()]
		
		qtot.sort(key=operator.itemgetter(1))
		if self.nplots > 0:
			sl = slice(-self.nplots,None,None)
		else:
			sl = slice(None)
		qtot.sort(key=lambda pair: geomap[pair[0]].position.calc_distance(hypo[0].pos))
		qtot = qtot[::-1][sl][::-1]
		# qtot.sort(key=operator.itemgetter(0))
		# qtot.sort()
		for om, qt in qtot:
			if not any([(p.flags & p.PulseFlags.LC) for p in pulsemap[om]]):
				continue
			pulses = pulsemap[om]
			waveforms = wfmap[om]
			
			cal = calibration.dom_cal[om]
			stat = status.dom_status[om]
			
			select = lambda wf: wf.digitizer == wf.ATWD and not ((wf.status & wf.SATURATED) or (wf.status & wf.UNDERSHOT))
			
			wfs = [filter(select, waveforms)[0]]
			trange = WaveformPlotter.get_time_range(wfs)
			
			ompos = geomap[om].position
			geotimes = [source.time + dataclasses.I3Constants.n_ice_group*ompos.calc_distance(source.pos)/dataclasses.I3Constants.c for source in hypo]
			
			trange = (min(trange[0], min(geotimes)), trange[1])
			
			edges = numpy.arange(trange[0], trange[1], 1*icetray.I3Units.ns)
			
			fig = pylab.figure()
			ax = pylab.gca()
			contribs = []
			geotimes = []
			labels = []
			ompos = geomap[om].position
			for source in hypo:
				if source.energy > 0:
					contrib, geotime = self.pdf.getPhotonDistribution(source, geomap[om].position, edges, geotime=True)
					# XXX HACK: scale to account for cable shadow and amplification eff.
					contrib *= 0.9
					contrib *= 0.85
					contribs.append(contrib)
					geotimes.append(geotime)
					d = source.pos.calc_distance(ompos)
					labels.append('%.1e GeV %s t=%.1f ns (%.1f m away)' % (source.energy, source.type, source.time, d))
			contribs.append(sum(contribs))
			geotimes.append(min(geotimes))
			labels.append('Total')

			wflabel = lambda wf: ('%s%d/%d' % (wf.digitizer, wf.source_index, wf.channel))
			
			if self.debug_store is not None:
				if om in self.debug_store.mappy:
					pylab.subplot(2,1,1)
					ax = pylab.gca()
				else:
					ax = pylab.gca()
			
			for wf in wfs:
				WaveformPlotter.plot_wf(ax, wf, label=wflabel(wf))
				WaveformPlotter.plot_pulses(ax, trange, wf, pulses, cal, stat,
				    use_domsimulator_hacks=simkludge, label=self.pulses, ls=':', color='k')
			realpulses = [p for p in pulses if p.charge > 1]
			# print [(p.charge, p.time) for p in pulses[:10]]
			pylab.axvline(realpulses[0].time/icetray.I3Units.microsecond, ls=':', color='k')
			
			from itertools import cycle
			colors = cycle(['b', 'r', 'g', 'c', 'm', 'y', 'k'])
			for contrib, geotime, label, color in zip(contribs, geotimes, labels, colors):
				pulses = self.potemkin_pulses(edges, contrib)
				for wf in wfs:
					WaveformPlotter.plot_pulses(ax, trange, wf, pulses, cal, stat,
					    use_domsimulator_hacks=simkludge,
					    color=color, label='%s (%s)' % (label, wflabel(wf)))
				pylab.axvline(geotime/icetray.I3Units.microsecond, color=color)
			
			import matplotlib.transforms as mtransforms
			trans = mtransforms.blended_transform_factory(ax.transData, ax.transAxes)		
			if om in saturation_windows:
				windows = saturation_windows[om]
				mask = numpy.zeros(edges.shape, dtype=bool)
				for w in windows:
					mask[(edges < w.stop)&(edges >= w.start)] = True
				if mask.any():
					pylab.fill_between(edges/icetray.I3Units.microsecond, 0, 1, where=mask,
					    facecolor='red', edgecolor=None, alpha=0.3, transform=trans, label='nonlinear PMT response')

			# pylab.legend(loc='best', prop=dict(size='small'))
			pylab.xlabel('Time [$\\mu$ s]')
			pylab.ylabel('Digitizer voltage [-mV]')
			title = 'Event %d/%d OM%s: %s' % (header.run_id, header.event_id, om, self.hypo)
			pylab.title(title, size='small')
			
			if self.debug_store is not None and om in self.debug_store.mappy:
				xlim = pylab.xlim()
				pylab.subplot(2,1,2)
				ax = pylab.gca()
				
				# for p in pulses:
				# 	pylab.axvline(p.time/icetray.I3Units.microsecond, color='k', ls=':')
					
				domcache = self.debug_store.mappy[om]
				e = numpy.asarray(domcache['time_bin_edges']) / icetray.I3Units.microsecond
				q = numpy.asarray(domcache['charges'])
				# q /= numpy.diff(e)
				x, y = stepped_path(e, q)
				pylab.plot(x, y, color='k', label='Observed')
				
				# print om, e
				
				invalid = numpy.logical_not(domcache['valid'])
				
				total = 0
				for source, basis in zip(hypo, domcache['bases']):
					basis = numpy.asarray(basis)
					total = total+source.energy*basis
					# basis[invalid] = 0
					x, y = stepped_path(e, source.energy*basis)
					pylab.plot(x, y)
					
				x, y = stepped_path(e, total)
				pylab.plot(x, y, color='r', lw=2)
				pylab.xlim(xlim)
				pylab.ylabel('Collected PE')
				pylab.legend(prop=dict(size='small'))
				
			if self.fname_base is not None:
				dirname = os.path.dirname(self.fname_base)
				basename = os.path.basename(self.fname_base)
				fname = os.path.join(dirname, ('OM%.2d%.2d_' % (om.string, om.om)) + basename + '.png')
				pylab.savefig(fname)
				print('Wrote %s' % fname)
				
		if self.fname_base is None:	
			pylab.show()
		
		self.PushFrame(frame)		

