from icecube import icetray
from icecube import gulliver

from icecube.load_pybindings import load_pybindings
load_pybindings(__name__, __path__)

try:
	from icecube.millipede import debugger
except:
	print('Error importing millipede debugger, skipping')
	pass

# Add numpy-backed views into MillipedeDOMCache
try:
	import numpy
	MillipedeDOMCache.time_bin_edges = property(lambda self: numpy.array(self._get_time_bin_edges(), copy=False))
	MillipedeDOMCache.charges = property(lambda self: numpy.array(self._get_charges(), copy=False))
except ImportError:
	pass

MillipedeHighEnergyMuon = icetray.module_altconfig('MuMillipede', 
    PhotonsPerBin=15, MuonRegularization=0, ShowerRegularization=1e-9,
    MuonSpacing=0, ShowerSpacing=10)

MillipedeLowEnergyMuon = icetray.module_altconfig('MuMillipede',
    PhotonsPerBin=10, MuonRegularization=2, ShowerRegularization=0,
    MuonSpacing=15, ShowerSpacing=0)

@icetray.traysegment
def photorec(tray, name, PhotonicsService='I3PhotonicsService',
    RecoParticleName='', RecoPulsesName='RecoPulseSeries',
    ResultParticleName='', **kwargs):
	'''Uses the Monopod module to emulate the behavior of I3PhotorecEnergyEstimator'''
	tray.AddModule('Monopod', name, MuonPhotonicsService=PhotonicsService,
	    PhotonsPerBin=-1, Pulses=RecoPulsesName, Seed=RecoParticleName,
	    **kwargs)

@icetray.traysegment
def AtmCscdEnergyReco(tray, name, PhotonicsServiceName='I3PhotonicsService',
    CascadeVertex='', InputRecoPulses='RecoPulseSeries',
    Output='', **kwargs):
	'''Uses the Monopod module to emulate the behavior of ACER'''
	
	if 'BadDOMListName' in kwargs and not 'ExcludedDOMs' in kwargs:
		from icecube.icetray import i3inspect
		exclusions = i3inspect.get_configuration('Monopod')['ExcludedDOMs']
		kwargs['ExcludedDOMs'] = list(exclusions) + [kwargs.pop('BadDOMListName')]
	
	deprecated = ['CascadeVertexAzimuthAngle', 'CascadeVertexZenithAngle', 'DiagnosticOutputFileName', 'DiagnosticOutputTag', 'FullOutput', 'IgnoreAMANDA', 'InputRecoHits', 'KeysToClean', 'MCTreeName', 'NoiseRate', 'NoiseReadoutWindow', 'SaveAllLlhTerms', 'UseMCTruthEnergy', 'UseMCTruthVertex', 'UseSeedVertexDirection', 'WeightDictField', 'WeightDictObjectName', 'WriteDiagnosticRootFile']
	for kw in kwargs.keys():
		if kw in deprecated:
			raise ValueError("The option %s=%s has no meaning for this segment. Are you sure you need it?" % (kw, kwargs[kw]))
	tray.AddModule('Monopod', name, CascadePhotonicsService=PhotonicsServiceName,
	    PhotonsPerBin=-1, Pulses=InputRecoPulses, Seed=CascadeVertex, Output=Output,
	    **kwargs)

@icetray.traysegment
def HighEnergyExclusions(tray, name, Pulses, ExcludeDeepCore='DeepCoreDOMs', ExcludeSaturatedDOMs='SaturatedDOMs',
    ExcludeBrightDOMs='BrightDOMs', BrightDOMThreshold=10,
    SaturationWindows='SaturationWindows', BadDomsList='BadDomsList', CalibrationErrata='CalibrationErrata'):
	"""
	Work around systematic errors in the modelling of the detector response by removing certain classes
	of DOMs from consideration that would otherwise over-contribute to Millipede likelihoods for events
	above a few hundred TeV.
	
	The options beginning with "Exclude" may be set to None or False to disable the relevant exclusion.
	
	:param Pulses: the name of the pulse map to be used for reconstruction
	:param ExcludeDeepCore: remove DeepCore strings from consideration
	:param ExcludeSaturatedDOMs: exclude saturated DOMs entirely, not just during the times when their output current is above the linearity limit
	:param ExcludeBrightDOMs: exclude DOMs that collect a total charge a factor greater than the mean charge
	:param BrightDOMTreshold: threshold factor for bright DOMs
	:param BadDomsList: list of DOMs that can't produce useful data
	:param SaturationWindows: times during which PMTs were nonlinear
	
	:returns: a list of exclusions that can be passed to Millipede modules
	"""
	from icecube import icetray, dataclasses
	
	def log(message):
		icetray.logging.log_info(message, unit="MillipedeHighEnergyExclusions")
	
	exclusions = [CalibrationErrata, BadDomsList]
	if ExcludeDeepCore:
		def DeepCoreFlagger(frame):
			dc_strings = set(range(79,87))
			dc_doms = dataclasses.I3VectorOMKey()
			for om in frame['I3Geometry'].omgeo.keys():
				if om.string in dc_strings:
					dc_doms.append(om)
			frame[ExcludeDeepCore] = dc_doms
		tray.AddModule(DeepCoreFlagger, name+'DeepCoreFlagger', Streams=[icetray.I3Frame.Geometry])
		exclusions.append(ExcludeDeepCore)
	
	if ExcludeSaturatedDOMs:
		def SaturationExtender(frame):
			if SaturationWindows in frame:
				frame[ExcludeSaturatedDOMs] = dataclasses.I3VectorOMKey(frame[SaturationWindows].keys())
				log("Excluding %d saturated DOMs: %s" % (len(frame[ExcludeSaturatedDOMs]), list(frame[ExcludeSaturatedDOMs])))
				
		tray.AddModule(SaturationExtender, name+'SaturationExtender', Streams=[icetray.I3Frame.DAQ])
		exclusions.append(ExcludeSaturatedDOMs)
	else:
		exclusions.append(SaturationWindows)
	
	if ExcludeBrightDOMs:
		def BrightDOMFlagger(frame):
			bright_doms = dataclasses.I3VectorOMKey()
			pmap = dataclasses.I3RecoPulseSeriesMap.from_frame(frame, Pulses)
			qtot = sum((sum(p.charge for p in pulses) for pulses in pmap.values()))
			max_q = BrightDOMThreshold*qtot/len(pmap)
			for om, pulses in pmap.items():
				q = sum(p.charge for p in pulses)
				if q > max_q:
					bright_doms.append(om)
			if len(bright_doms):
				frame[ExcludeBrightDOMs] = bright_doms
				log("Excluding %d bright DOMs: %s" % (len(bright_doms), list(bright_doms)))
				
		tray.AddModule(BrightDOMFlagger, name+'BrightDOMFlagger')
		exclusions.append(ExcludeBrightDOMs)
	
	return exclusions

import sys
from inspect import getargspec as _getargspec
if sys.version_info[0] <= 2 and sys.version_info[1] < 6:
	class ArgSpec(object):
		def __init__(self, tup):
			self.args, self.varargs, self.keywords, self.defaults = tup
		@classmethod
		def getargspec(cls, target):
			return cls(_getargspec(target))
	getargspec = ArgSpec.getargspec
else:
	getargspec = _getargspec

def millipedefit(parametrization_segment):
	"""
	Decorator to turn a segment containing a parametrization into a full-blown Millipede fit segment.
	"""
	import inspect
	argspec = getargspec(parametrization_segment)
	if len(argspec.args) < 2:
		raise ValueError("Parametrization segment must take at least 2 arguments (tray and name)")
	if argspec.keywords is None:
		raise ValueError("Parametrization segment must accept additional keyword arguments")
	def MillipedeFit(tray, name, Pulses, Seed, Iterations=1, Photonics="I3PhotonicsService", Minimizer="MIGRAD",
	    BadDOMs=["BadDomsList"], **kwargs):
		"""
		:param Pulses:          the I3RecoPulseSeriesMap to run on. The data should have no hit
		                        cleaning applied.
	
		:param Seed:            a good first guess. For amplitude-only fits (PhotonsPerBin=-1) this may be
		                        the output of a rough reconstruction like CscdLlhVertexFit; for fits with
		                        timing it is better to first run one iteration of this fit without timing
		                        and use its output as the seed.
		:param Iterations:      if > 1, perform in iterative fit by seeding with this number of directions.
	
		:param Minimizer:       the algorithm to use, either SIMPLEX or MIGRAD. The default is recommended,
		                        as it can use analytic gradients to converge more quickly.
	
		:param Photonics:       the I3PhotonicsService to query for cascade light
		                        yields. This can be either a name-in-the-context of an instance.
	
		:param BadDOMs:         DOMs to exclude from the fit.
		
		Remaining keyword arguments will be passed to MillipedeLikelihoodFactory.
		"""
		from icecube.icetray import load, I3Units
		load("libgulliver-modules", False)
		from icecube import gulliver, lilliput, millipede	
		import math

		tag = name
		outputs = [tag, tag + 'FitParams']

		seeder = "%s_seedprep" % tag
		minimizer = "%s_minimizer" % tag
		likelihood = "%s_likelihood" % tag
		paramer = "%s_parametrization" % tag
		fitter = tag

		If = kwargs.pop("If", None)

		# Pass multiple seeds through
		seed_kwargs = dict(InputReadout=Pulses, TimeShiftType="TNone", PositionShiftType="None")
		if isinstance(Seed, str):
			seed_kwargs['FirstGuess'] = Seed
		else:
			seed_kwargs['FirstGuesses'] = list(Seed)

		tray.AddService("I3BasicSeedServiceFactory", seeder, **seed_kwargs)

		Minimizer = Minimizer.upper()
		if Minimizer == "SIMPLEX":
			tray.AddService("I3GulliverMinuitFactory", minimizer,
				MaxIterations=2000,
				Tolerance=0.1,
				Algorithm="SIMPLEX",
			)
		elif Minimizer == "MIGRAD":
			tray.AddService("I3GulliverMinuit2Factory", minimizer,
				MaxIterations=1000,
				Tolerance=0.1,
				Algorithm="MIGRAD",
				WithGradients=True,
				FlatnessCheck=False,
				IgnoreEDM=True, # Don't report convergence failures
				CheckGradient=False, # Don't die on gradient errors
				MinuitStrategy=0, # Don't try to check local curvature
			)
		elif Minimizer == "LBFGSB":
			tray.AddService("I3GulliverLBFGSBFactory", minimizer,
			    MaxIterations=1000,
				Tolerance=1e-3,
				GradientTolerance=1,
			)
		else:
			raise ValueError("Unknown minimizer '%s'!" % Minimizer)
	
		# Strip off any keyword arguments defined in the paramer segment
		paramer_config = dict()
		for k in argspec.args[2:]:
			if k in kwargs:
				paramer_config[k] = kwargs.pop(k)
		# pass them to the paramer segment anyway for informational purposes
		paramer_config.update(kwargs)
		# hypothesis-specific parameterization may also need to check the seed
		paramer_config['Seed'] = Seed
	
		millipede_config = dict(CascadePhotonicsService=Photonics,
		    Pulses=Pulses,
		    ExcludedDOMs=list(set(['CalibrationErrata', 'SaturationWindows'] + BadDOMs)))
		millipede_config.update(kwargs)
		tray.AddService('MillipedeLikelihoodFactory', likelihood, **millipede_config)
		
		# Set up the parametrization (the only part that changes between fit segments)
		parametrization_segment(tray, paramer, **paramer_config)
	
		if Iterations == 1:
			tray.AddModule("I3SimpleFitter", tag,
				SeedService=seeder,
				Parametrization=paramer,
				LogLikelihood=likelihood,
				Minimizer=minimizer,
				NonStdName=tag+"Particles",
				If=If,
			)
		else:
			# NB: SOBOL is a magic argument, not actually the name
			# of an I3RandomService in the context.
			tray.AddModule("I3IterativeFitter", tag,
				SeedService=seeder,
				Parametrization=paramer,
				LogLikelihood=likelihood,
				Minimizer=minimizer,
				NonStdName=tag+"Particles",
				RandomService="SOBOL",
				NIterations=Iterations,
				If=If,
			)
	
		# Augment the I3LogLikelihoodFitParams from the
		# fitter module with the MillipedeFitParams from
		# the likelihood.
		def Millipedeify(frame):
			gtag = '%sFitParams' % tag
			mtag = '%s_%s' % (tag, likelihood)
			if not mtag in frame:
				if gtag in frame:
					frame.Delete(gtag)
				return
			gulliparams = frame[gtag]
			monoparams = frame[mtag]
			for a in ('logl', 'rlogl', 'ndof', 'nmini'):
				setattr(monoparams, a, getattr(gulliparams, a))
			frame.Delete(gtag)
			frame.Rename(mtag, gtag)
		
		tray.AddModule(Millipedeify, tag+"ReplaceFitParams")

		return outputs
	
	# inform icetray-inspect of the inner segment's arguments
	req = len(argspec.args)-len(argspec.defaults)
	MillipedeFit.additional_kwargs = dict([(argspec.args[req+i], argspec.defaults[i]) for i in range(len(argspec.defaults))])
	MillipedeFit.__doc__ = inspect.getdoc(parametrization_segment) + "\n\n" + inspect.getdoc(MillipedeFit)
	return MillipedeFit

@icetray.traysegment
@millipedefit
def MonopodFit(tray, name, Parametrization="Simple", StepT=15, StepD=5, StepZenith=5, StepAzimuth=5, StepDir=0.3, **kwargs):
	"""
	Perform a Gulliver likelihood fit for the position, time, direction, and energy of a single cascade.
	
	:param Parametrization: the type of parametrization to use. The Simple parametrization is a brain-dead
	                        pass-through of x,y,z,t,zenith,azimuth and has singularities at the poles; the
	                        HalfSphere parametrization avoids these at the expense of only covering one
	                        hemisphere, and is thus better suited for iterative fits.
	
	:param StepT:         	step size in t in nanoseconds. Set to zero for amplitude-only fits (PhotonsPerBin=-1).
	
	:param StepD:         	step size in x, y, z in meters.
	
	:param StepZenith:      step size in zenith in degree (only for simple parametrization).
	
	:param StepAzimuth:     step size in azimuth in degree (only for simple parametrization).
	
	:param StepDir:         step size in direction in radian (only for halfsphere parametrization).
	
	"""
	from icecube.icetray import I3Units
	
	vertexBounds = [-200*I3Units.m, 200*I3Units.m]
	if kwargs.get('PhotonsPerBin', 15) < 0:
		StepT = 0
	
	if Parametrization.lower() == "simple":
		tray.AddService('I3SimpleParametrizationFactory', name,
			StepX=StepD*I3Units.m,
			StepY=StepD*I3Units.m,
			StepZ=StepD*I3Units.m,
			RelativeBoundsX=vertexBounds,
			RelativeBoundsY=vertexBounds,
			RelativeBoundsZ=vertexBounds,
			StepT=StepT*I3Units.ns,
			StepZenith=StepZenith*I3Units.degree,
			BoundsZenith=[0, 180*I3Units.degree],
			StepAzimuth=StepAzimuth*I3Units.degree,
			BoundsAzimuth=[0, 360*I3Units.degree],
			# Monopod fits for energy analytically
			)
	elif Parametrization.lower() == "halfsphere":
		tray.AddService('I3HalfSphereParametrizationFactory', name,
		    DirectionStepSize=StepDir, TimeStepSize=StepT, VertexStepSize=StepD*I3Units.m,
		)
	else:
		raise ValueError("Unknown parametrization '%s'!" % Parametrization)
	
	# If the seed is track-shaped, MillipedeLikelihood will try to use the
	# [non-existant] muon tables to look up the light yield.
	Seed = kwargs.get('Seed', '')
	if isinstance(Seed, str):
		Seed = [Seed]
	def seatbelt(frame):
		for k in Seed:
			if k in frame:
				part = frame[k]
				if part.is_cascade:
					assert 'CascadePhotonicsService' in kwargs, "MonopodFit configured with a cascade seed, but no cascade photonics service configured"
				elif part.is_track:
					assert 'MuonPhotonicsService' in kwargs, "MonopodFit configured with a track seed, but no muon photonics service configured"
				return
	tray.Add(seatbelt)

@icetray.traysegment
@millipedefit
def TaupedeFit(tray, name, StepT=15, StepD=5, StepZenith=5, StepAzimuth=5, StepL=10, LengthBounds=[0, 2000], **kwargs):
	"""
	Perform a Gulliver likelihood fit for the position, time, direction,
	decay length, and energies of a tau double-bang event.

	:param StepT:           step size in t in nanoseconds. Set to zero for amplitude-only fits (PhotonsPerBin=-1).

	:param StepD:           step size in x, y, z in meters.

	:param StepZenith:      step size in zenith in degree

	:param StepAzimuth:     step size in azimuth in degree

	:param StepL:           step size in tau track length in meters.
    
	:param LengthBounds:    boundary for tau track length in meters.
	
	"""
	from icecube.icetray import I3Units
	
	vertexBounds = [-200*I3Units.m, 200*I3Units.m]
	if StepL <= 0:
		LengthBounds = [0, 0]
	if kwargs.get('PhotonsPerBin', 15) < 0:
		StepT = 0

	tray.AddService('TauMillipedeParametrizationFactory', name,
		StepX=StepD*I3Units.m,
		StepY=StepD*I3Units.m,
		StepZ=StepD*I3Units.m,
		RelativeBoundsX=vertexBounds,
		RelativeBoundsY=vertexBounds,
		RelativeBoundsZ=vertexBounds,
		StepT=StepT*I3Units.ns,
		StepLinL=StepL*I3Units.m,
		BoundsLinL=LengthBounds,
		StepZenith=StepZenith*I3Units.degree,
		StepAzimuth=StepAzimuth*I3Units.degree,
		# Taupede fits for energy analytically
		)

@icetray.traysegment
@millipedefit
def MuMillipedeFit(tray, name, MuonSpacing=0, ShowerSpacing=15, StepT=15, StepD=5, StepZenith=5, StepAzimuth=5, Boundary=600, **kwargs):
	"""
	Perform a Gulliver likelihood fit for the positions, times, directions, and energies of a string
	of equally spaced cascades and tracks.

	:param MuonSpacing:     spacing between track segments. 
	
	:param CascadeSpacing:     spacing between cascades. 
	
	:param StepT:           step size in t in nanoseconds. Set to zero for amplitude-only fits (PhotonsPerBin=-1).

	:param StepD:           step size in x, y, z in meters.

	:param StepZenith:      step size in zenith in degree

	:param StepAzimuth:     step size in azimuth in degree

	"""
	from icecube.icetray import I3Units
	
	if kwargs.get('PhotonsPerBin', 15) < 0:
		StepT = 0
	if kwargs.get('MuonPhotonicsService', None) is None:
		MuonSpacing = 0
	if kwargs.get('CascadePhotonicsService', None) is None and kwargs.get('PhotonicsService', None) is None:
		ShowerSpacing = 0
	
	tray.AddService('MuMillipedeParametrizationFactory', name,
		Boundary=Boundary,
		MuonSpacing=MuonSpacing,
		ShowerSpacing=ShowerSpacing,
		StepX=StepD*I3Units.m,
		StepY=StepD*I3Units.m,
		StepZ=StepD*I3Units.m,
		StepT=StepT*I3Units.ns,
		StepZenith=StepZenith*I3Units.degree,
		StepAzimuth=StepAzimuth*I3Units.degree,
		)

