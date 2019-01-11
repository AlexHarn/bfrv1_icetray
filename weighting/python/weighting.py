from __future__ import print_function

import numpy
import copy
import itertools
import warnings
import sys

from icecube.icetray import I3Units

if sys.version_info.major > 2:
	from .enum3 import enum
else:
	from .enum import enum

# some enums for CORSIKA->PDG compatibility
class ParticleType(enum):
	PPlus       =   14
	He4Nucleus  =  402
	N14Nucleus  = 1407
	O16Nucleus  = 1608
	Al27Nucleus = 2713
	Fe56Nucleus = 5626
	NuE         =   66
	NuEBar      =   67
	NuMu        =   68
	NuMuBar     =   69
	NuTau       =  133
	NuTauBar    =  134

class PDGCode(enum):
	PPlus       =       2212
	He4Nucleus  = 1000020040
	N14Nucleus  = 1000070140
	O16Nucleus  = 1000080160
	Al27Nucleus = 1000130270
	Fe56Nucleus = 1000260560
	NuE         =         12
	NuEBar      =        -12
	NuMu        =         14
	NuMuBar     =        -14
	NuTau       =         16
	NuTauBar    =        -16

PDGCode.from_corsika = classmethod(lambda cls, i: getattr(cls, ParticleType.values[i].name))
ParticleType.from_pdg = classmethod(lambda cls, i: getattr(cls, PDGCode.values[i].name))

class GenerationProbability(object):
	"""
	A probability distribution from which MC events are drawn.
	
	Generation probabilities may be multiplied by an integer to express
	a joint generation probability for multiple identical simulation runs
	or added together to express a set of distinct runs.
	"""
	def __init__(self, emin, emax, nevents=1, area=1, particle_type=None):
		"""
		:param emin: minimum energy for which this probability is non-zero
		:param emax: maximum energy for which this probability is non-zero
		:param nevents: number of generated events
		:param area: integrated area-times-solid-angle of the sampling surface
		:param particle_type: particle type for which this probability is valid. if None,
		                      then it is valid for all particle types.
		"""
		self.emin = emin
		self.emax = emax
		self.nevents = nevents
		self.area = area
		self.particle_type = particle_type
	
	def to_PDG(self):
		"Translate to a form that understands PDG codes instead of CORSIKA"
		if self.particle_type is not None and getattr(PDGCode, self.particle_type.name) == self.particle_type:
			return self
		new = copy.deepcopy(self)
		if new.particle_type is not None:
			new.particle_type = PDGCode.from_corsika(new.particle_type)
		return new
	
	def __imul__(self, factor):
		self.nevents *= factor
		return self
	
	def __mul__(self, factor):
		t = copy.deepcopy(self)
		t *= factor
		return t
	
	def __rmul__(self, factor):
		return self*factor
	
	def __idiv__(self, factor):
		self.nevents /= factor
		return self
		
	def __div__(self, factor):
		t = copy.deepcopy(self)
		t /= factor
		return t
		
	def __iadd__(self, other):
		if isinstance(other, type(self)):
			if self.is_compatible(other):
				self.nevents += other.nevents
				return self
			else:
				return GenerationProbabilityCollection([copy.deepcopy(self), other])
		else:
			raise TypeError("Can't add a %s to this %s" % (type(other).__name__, type(self).__name__))
	
	def __add__(self, other):
		if isinstance(other, type(self)):
			if self.is_compatible(other):
				t = copy.deepcopy(self)
				t += other
				return t
			else:
				return GenerationProbabilityCollection([copy.deepcopy(self), other])
		else:
			return GenerationProbabilityCollection([copy.deepcopy(self), other])
	
	def __call__(self, E, particle_type=None, cos_theta=None):
		"""
		Calculate the generated fluence of particles in this simulation scheme
		
		:param E: the energy of particle in GeV
		:param particle_type: the particle type for which to return a fluence
		:returns: a fluence in units of :math:`m^{-2} sr^{-1} GeV^{-1}`
		"""
		E = numpy.asarray(E)
		if hasattr(self.area, "__call__"):
			area = self.area(cos_theta)
		else:
			area = self.area
		scalar = numpy.ndim(E)==0 and numpy.ndim(cos_theta)==0 and numpy.ndim(particle_type)==0
		E = numpy.atleast_1d(E)
		area = numpy.atleast_1d(area)
		try: numpy.broadcast(E, particle_type, cos_theta)
		except: raise Exception("E, particle_type and cos_theta can not be broadcasted. Shapes: %s, %s, %s"%(str(E.shape), str(particle_type.shape), str(cos_theta.shape)))
		mask = (E>=self.emin)&(E<self.emax)&(area>0)
		if particle_type is not None:
			mask &= (particle_type==self.particle_type)
		r = numpy.where(mask, self.nevents*self.generation_probability(E)/area, 0)
		if scalar: return r[0]
		return r
	        
	def is_compatible(self, other):
		raise NotImplementedError("should be overridden")
	
	def __eq__(self, other):
		return self.nevents == other.nevents and self.is_compatible(other)
	
	def __ne__(self, other):
		return not (self == other)
	
	def generation_probability(self, E):
		raise NotImplementedError("should be overridden")

class Null(object):
	"""
	An identity object, useful as a starting point for accumulators, e.g.::

		total = Null()
		for i in range(10):
			total += SomeClassThatImplementsAddition(i)
	"""
	def __add__(self, other):
		return other
	def __radd__(self, other):
		return other
	def __iadd__(self, other):
		return other
	def __mul__(self, f):
		return self
	def __rmul__(self, f):
		return self
	def __imul__(self, f):
		return self
	def __eq__(self, other):
		return isinstance(other, Null) or 0 == other
	def __ne__(self, other):
		return not (self == other)

class GenerationProbabilityCollection(object):
	"""
	A collection of generation spectra, possibly for different particle types.
	"""
	def __init__(self, spectra):
		"""
		:param spectra: a collection of GenerationProbabilities.
		"""
		from collections import defaultdict
		self.spectra = defaultdict(list)
		for dist in spectra:
			self.spectra[dist.particle_type].append(dist)
	
	def to_PDG(self):
		spectra = []
		for v in self.spectra.values():
			spectra += [s.to_PDG() for s in v]
		return self.__class__(spectra)
		
	def __call__(self, E, particle_type=None, cos_theta=None):
		if particle_type is None:
			return sum([prob(E, cos_theta=cos_theta) for spectra in self.spectra.values() for prob in spectra])
		else:
			if numpy.ndim(particle_type) == 0:
				particle_type = numpy.ones_like(E)*particle_type
			if cos_theta is None:
				try:  E, particle_type = numpy.broadcast_arrays(E, particle_type)
				except: raise Exception("E and particle_type can not be broadcasted. Shapes: %s, %s"%(str(E.shape), str(particle_type.shape)))
			else:
				try:  E, particle_type, cos_theta = numpy.broadcast_arrays(E, particle_type, cos_theta)
				except: raise Exception("E, particle_type and cos_theta can not be broadcasted. Shapes: %s, %s, %s"%(str(E.shape), str(particle_type.shape), str(cos_theta.shape)))
			E = numpy.asarray(E)
			if numpy.ndim(particle_type) == 0:
				self.spectra[int(particle_type)]
				return sum([prob(E, cos_theta=cos_theta) for prob in self.spectra[int(particle_type)]])
			count = numpy.zeros(E.shape)
			for ptype in numpy.unique(particle_type):
				mask = particle_type==ptype
				if numpy.any(mask):
					Em = E[mask]
					if cos_theta is not None:
						ctm = cos_theta[mask]
					else:
						ctm = cos_theta
					count[mask] += sum([prob(Em, cos_theta=ctm) for prob in self.spectra[ptype]])
			return count
	
	def __imul__(self, factor):
		for spectra in self.spectra.values():
			for prob in spectra:
				prob *= factor
		return self
	
	def __idiv__(self, factor):
		self *= (1./factor)
		return self
	
	def __mul__(self, factor):
		clone = copy.deepcopy(self)
		clone *= factor
		return clone
	
	def __div__(self, factor):
		return self*(1./factor)
	
	def __rmul__(self, factor):
		return self*factor
	
	def __iadd__(self, other):
		if isinstance(other, type(self)):
			for pt, ospectra in other.spectra.items():
				for ospec in ospectra:
					for spec in self.spectra[pt]:
						if spec.is_compatible(ospec):
							spec += ospec
							break
					else:
						self.spectra[pt].append(ospec)
			return self
		else:
			for spec in self.spectra[other.particle_type]:
				if spec.is_compatible(other):
					spec += other
					break
			else:
				self.spectra[other.particle_type].append(other)
			return self
	
	def __add__(self, other):
		t = copy.deepcopy(self)
		t += other
		return t
	
	def __eq__(self, other):
		# must handle the same set of particle types
		if set(self.spectra.keys()) != set(other.spectra.keys()):
			return False
		for k in self.spectra:
			s1 = self.spectra[k]
			s2 = other.spectra[k]
			# must have the same number of unique spectra
			if len(s1) != len(s2):
				return False
			# exactly one match for each spectrum
			for p1 in s1:
				if sum(p1 == p2 for p2 in s2) != 1:
					return False
		return True
	
	def __ne__(self, other):
		return not self == other
	
class PowerLaw(GenerationProbability):
	"""
	Power-law spectra are easy.
	"""
	def __init__(self, eslope, *args, **kwargs):
		"""
		:param eslope: index of the power law :math:`E^{-\gamma}`
		"""
		super(PowerLaw, self).__init__(*args, **kwargs)
		self.eslope = eslope
		self.gen_norm = self.norm(self.emin, self.emax, self.eslope)
	
	def subset(self, emin, emax):
		"""
		Return a copy with the same normalization but a smaller range
		"""
		if emin < self.emin or emax > self.emax:
			raise ValueError("Energy range must be a subset of parent energy range")
		fraction = self.norm(emin, emax, self.eslope)/self.gen_norm
		new = copy.copy(self)
		new.emin = emin
		new.emax = emax
		new.gen_norm = self.gen_norm*fraction
		new.nevents = self.nevents*fraction
		return new
	
	def partition(self, total_events, nfiles, scaling_power=1):
		"""
		Divide the energy range into chunks of with roughly equal total energy,
		preserving the total number of events. This allows for splitting a
		simulation run into chunks with similar total running time.
		
		:param total_events: total number of events that should be generated
		:param nfiles: number of chunks to emit
		:param scaling_power: the power of energy that the simulation running
		                      time scales with
		
		:returns: a tuple (nevents, edges). nevents[i] gives the number of
		          events to generate in the interval [edges[i], edges[i+1])
		"""
		
		# Divide the spectrum into chunks with equal total energy
		total_events = int(total_events)
		nfiles = int(nfiles)
		
		g = float(self.eslope + scaling_power)
		if g == -1:
			bins = numpy.logspace(numpy.log10(self.emin), numpy.log10(self.emax), nfiles+1)
		# elif g == 0.:
		# 	bins = numpy.linspace(self.emin, self.emax, nfiles+1)
		else:
			c = numpy.linspace(0, self.emax**(g+1)-self.emin**(g+1), nfiles+1)
			bins = (c + self.emin**(g+1))**(1./(g+1))
		
		norm = numpy.vectorize(self.norm)
		
		# Find the number of events that should end up in each chunk
		events = total_events*self.norm(bins[:-1], bins[1:], self.eslope) / self.norm(self.emin, self.emax, self.eslope)
		
		# Round event numbers up to the nearest integer, and then subtract the 
		# remainder from the n largest entries to preserve the total number of
		# events.
		rounded_events = numpy.ceil(events).astype(int)
		remainder = events.sum() - rounded_events.sum()
		n = int(round(remainder))
		order = numpy.argsort(rounded_events)[::-1]
		rounded_events[order[:abs(n)]] += numpy.copysign(1, n).astype(int)
		numpy.testing.assert_equal(total_events, rounded_events.sum(), "Rounding preserves the sum")
		
		# Now,re-solve for the boundaries given the event numbers in each bin
		p = rounded_events[:-1].cumsum().astype(float)/total_events
		bins = numpy.concatenate(([self.emin], self.invert(p), [self.emax]))
		
		# Check that we've gotten the number of events correct
		check_events = total_events*self.norm(bins[:-1], bins[1:], self.eslope) / self.norm(self.emin, self.emax, self.eslope)
		numpy.testing.assert_allclose(check_events, rounded_events, atol=1)
		
		return rounded_events, bins
	
	@staticmethod
	def norm(emin, emax, eslope):
		if eslope == -1:
			return numpy.log(emax/emin)
		else:
			g = eslope+1
			return (emax**g - emin**g)/g
	
	def __repr__(self):
		return "PowerLaw(%.2f, emin=%.2e, emax=%.2e, nevents=%.2e)" % (self.eslope, self.emin, self.emax, self.nevents)
	
	def generation_probability(self, E):
		return E**(self.eslope)/self.gen_norm
	
	def invert(self, p):
		"""
		Return CDF^{-1}(p)
		"""
		if self.eslope == -1:
			return self.emin*((self.emax/self.emin)**p)
		else:
			return (p*(self.emax**(self.eslope+1) - self.emin**(self.eslope+1)) + self.emin**(self.eslope+1))**(1./(self.eslope+1))
	
	def is_compatible(self, other):
		if isinstance(other, type(self)):
			return self.emin == other.emin and self.emax == other.emax and self.eslope == other.eslope and self.particle_type == other.particle_type
		else:
			return False

class HoerandelComponent(GenerationProbability):
	"""
	A power law with a rigidity-dependent knee.
	"""
	def __init__(self, z, eslope, *args, **kwargs):
		"""
		:param z: charge of the p
		
		"""
		super(HoerandelComponent, self).__init__(*args, **kwargs)
		self.z = z
		self.gamma = eslope
		self.gen_norm = self.fluxsum(self.emin, self.emax, z, self.gamma)
	
	def generation_probability(self, E):
		return self.fluxdiff(E, self.z, self.gamma)/self.gen_norm
	
	def is_compatible(self, other):
		if isinstance(other, type(self)):
			return self.emin == other.emin and self.emax == other.emax and self.gamma == other.gamma and self.particle_type == other.particle_type and self.z == other.z
		else:
			return False
	
	@staticmethod	
	def fluxdiff(e, z, gamma, delta_gamma=2.1, eps_cutoff=1.9, E_knee=4.49*I3Units.PeV):
		"""
		Differential (unnormalized) Hoerandel flux
		"""
		return e**(-gamma)*(1+(e/(E_knee*z))**eps_cutoff)**(-delta_gamma/eps_cutoff)
	
	@staticmethod
	def fluxsum(emin, emax, z, gamma, delta_gamma=2.1, eps_cutoff=1.9, E_knee=4.49*I3Units.PeV):
		"""
		Integral Hoerandel flux
		"""
		# the Gauss hypergeometric function. whee!
		from scipy.special import hyp2f1
		antideriv = lambda e: ((e**(1-gamma))/(1-gamma))*hyp2f1(delta_gamma/eps_cutoff, (1-gamma)/eps_cutoff, (1-gamma)/eps_cutoff+1, -(e/(E_knee*z))**eps_cutoff)
		return antideriv(emax) - antideriv(emin)

def FiveComponent(nevents, emin, emax, normalization=[10., 5., 3., 2., 1.], gamma=[-2.]*5,
    LowerCutoffType='EnergyPerNucleon', UpperCutoffType='EnergyPerParticle', height=1600, radius=800,
    ZenithBias='VOLUMECORR', ZenithRange=[0*I3Units.degree, 90*I3Units.degree]):
	"""
	Construct a normalization term for 5-component dCORSIKA simulation.
	
	:param normalization: relative normalizations of the 5 components
	:param gamma: power law index for each component
	:param LowerCutoffType: 
	:param spric: make lower energy proportional to primary mass
	:param height: full height of the target cylinder in meters
	:param radius: radius of the target cylinder in meters
	
	.. note:: The return value of the GenerationProbability will be in units of :math:`GeV^{-1} sr^{-1} m^{-2}`
	"""
	pt = [getattr(ParticleType, p) for p in ('PPlus', 'He4Nucleus', 'N14Nucleus', 'Al27Nucleus', 'Fe56Nucleus')]
	if LowerCutoffType=='EnergyPerParticle':
		lower_energy_scale = [1.]*5
	else:
		lower_energy_scale = [[1, p/100][p>100] for p in pt]
	if UpperCutoffType=='EnergyPerParticle':
		upper_energy_scale = [1.]*5
	else:
		upper_energy_scale = [[1, p/100][p>100] for p in pt]
	# DCORSIKA does this integral in TeV, so we have to do so as well.
	fluxsums = numpy.array([n*PowerLaw.norm(emin*mlo/I3Units.TeV, emax*mhi/I3Units.TeV, g) for mlo, mhi, g, n in zip(lower_energy_scale, upper_energy_scale, gamma, normalization)])
	nshower = nevents*fluxsums/fluxsums.sum()
	area = UprightCylinderAcceptance(height, radius, numpy.cos(ZenithRange[1]), numpy.cos(ZenithRange[0]))
	if ZenithBias.upper() == 'VOLUMECORR':
		# showers were generated with a zenith angle distribution proportional
		# to the projected area of the target surface, density of showers is
		# the same at all angles
		area = area.average
	elif ZenithBias.upper() == 'VOLUMEDET':
		# showers were generated with an isotropic zenith angle distribution,
		# but distributed over a different area at each zenith angle.
		pass
	else:
		raise ValueError("Unknown ZenithBias '{}'".format(ZenithBias))
	return GenerationProbabilityCollection([PowerLaw(g, emin*mlo, emax*mhi, nevents=n, area=area, particle_type=p) for mlo, mhi, g, n, p in zip(lower_energy_scale, upper_energy_scale, gamma, nshower, pt)]).to_PDG()

def Hoerandel(nevents, emin, emax, dslope=0, height=1600, radius=800):
	"""
	Construct a normalization term for dCORSIKA simulation run in RANPRI=2 mode,
	where it produces a "natural" spectrum with all elements from H to Fe
	according to the parameterizaton of Hoerandel_. In order to combine this
	with 5-component simulation we use only H, He, N, Al, and Fe, returning a
	generation probability of 0 for all other elements.
	
	:param dslope: multiply all fluxes by E^dslope
	:param height: full height of the target cylinder in meters
	:param radius: radius of the target cylinder in meters
	
	.. note:: The return value of the GenerationProbability will be in units of :math:`GeV^{-1} sr^{-1} m^{-2}`
	"""
	# Ripped from dCORSIKA source (with RANPRI=2)
	mass_number = numpy.round([1.00797, 4.0026, 6.939, 9.0122, 10.811, 12.0112, 14.0067, 15.9994, 18.9984, 20.183, 22.9898, 24.312, 26.9815, 28.086, 30.984, 32.064, 35.453, 39.948, 39.102, 40.08, 44.956, 47.9, 50.942, 51.996, 54.938, 55.847])
	fluxes = numpy.array([0.0873, 0.0571, 0.00208, 0.000474, 0.000895, 0.0106, 0.00235, 0.0157, 0.000328, 0.0046, 0.000754, 0.00801, 0.00115, 0.00796, 0.00027, 0.00229, 0.000294, 0.000836, 0.000536, 0.00147, 0.000304, 0.00113, 0.000631, 0.00136, 0.00135, 0.0204])
	gammas = numpy.array([2.71, 2.64, 2.54, 2.75, 2.95, 2.66, 2.72, 2.68, 2.69, 2.64, 2.66, 2.64, 2.66, 2.75, 2.69, 2.55, 2.68, 2.64, 2.65, 2.7, 2.64, 2.61, 2.63, 2.67, 2.46, 2.59])
	
	# Integrate each power law (without a knee) to get the total flux
	# This calculation is done in TeV (it matters!)
	emin = emin*mass_number/I3Units.TeV
	# Generation spectrum includes the slope change
	gamma = gammas + dslope
	fluxsums = fluxes*(emin**(1-gamma))/(gamma-1)
	nshower = nevents*fluxsums/fluxsums.sum()
	
	components = []
	area = numpy.pi**2*radius*(radius+height)
	for pt in [getattr(ParticleType, p) for p in ('PPlus', 'He4Nucleus', 'N14Nucleus', 'Al27Nucleus', 'Fe56Nucleus')]:
		if pt < 100:
			z = 1
		else:
			z = pt % 100
		components.append(HoerandelComponent(z, gamma[z-1], emin[z-1]*I3Units.TeV, emax,
		    nevents=nshower[z-1], area=area, particle_type=pt))
	return GenerationProbabilityCollection(components).to_PDG()

class UprightCylinderAcceptance(object):
	"""
	Acceptance (A * Omega) of an upright cylinder
	"""
	def __init__(self, length, radius, cosMin=-1, cosMax=1):
		self.length = length
		self.radius = radius
		self._solid_angle = 2*numpy.pi*(cosMax-cosMin)
		self._side = 2*self.radius*self.length
		self._cap = numpy.pi*self.radius**2
		self.average = self.etendue(cosMin, cosMax)
	
	def __call__(self, ct):
		return self._solid_angle*(self._cap*abs(ct) + self._side*numpy.sqrt(1-ct**2))
	
	@staticmethod
	def _integrate_area(a, b, cap, sides):
		return (cap*(b**2-a**2) + sides*(numpy.arccos(a) - numpy.arccos(b) - numpy.sqrt(1-a**2)*a + numpy.sqrt(1-b**2)*b))/2.
	
	def etendue(self, cosMin=-1., cosMax=1.):
		"""
		Integrate A * d\Omega over the given range of zenith angles
		
		:param cosMin: cosine of the maximum zenith angle
		:param cosMax: cosine of the minimum zenith angle
		:returns: a product of area and solid angle. Divide by
				  2*pi*(cosMax-cosMin) to obtain the average projected area in
				  this zenith angle range
		"""
		
		sides = self._side
		cap = self._cap
		
		if (cosMin >= 0 and cosMax >= 0):
			area = self._integrate_area(cosMin, cosMax, cap, sides)
		elif (cosMin < 0 and cosMax <= 0):
			area = self._integrate_area(-cosMax, -cosMin, cap, sides)
		elif (cosMin < 0 and cosMax > 0):
			area = self._integrate_area(0, -cosMin, cap, sides) \
				+ self._integrate_area(0, cosMax, cap, sides)
		else:
			area = numpy.nan
			raise ValueError("Can't deal with zenith range [%.1e, %.1e]" % (cosMin, cosMax))
		return 2*numpy.pi*area
	
	def average(self):
		pass
	

class Circle:
	def __init__(self, radius):
		self.radius = radius
	def __call__(self, cosTheta):
		return 2*numpy.pi**2*self.radius**2*cosTheta
	def __repr__(self):
		return 'Circle({})'.format(self.radius)

class Cylinder:
	def __init__(self, height, radius):
		self.height = height
		self.radius = radius
	def __call__(self, cosTheta):
		return 2*numpy.pi*(numpy.pi*self.radius**2*cosTheta + 2*self.radius*self.height*numpy.sqrt(1-cosTheta**2))
	def __repr__(self):
		return 'Cylinder({},{})'.format(self.height, self.radius)

class Sphere:
	def __init__(self, radius):
		self.radius = radius
	def __call__(self, cosTheta):
		return 2*numpy.pi**2*self.radius**2
	def __repr__(self):
		return 'Sphere({})'.format(self.radius)

class AngularGenerationDistribution(object):
	def __init__(self, cosMin, cosMax, generated_surface, resampled_surface=None):
		"""This class handles the angular dependency of weights. It can be
		passed as the area parameter of a GenerationProbability.
		
		Normally, one generates CORSIKA events according to an angular
		distribution, given by the geometric acceptance of a detector
		with shape such as a sphere, a cylinder, a plane, etc.
		When events are simulated, they are then spread over a surface,
		which usually has the same dimensions. This class calculates
		the weights that need to be applied to simulated events, including
		cases where these two surfaces differ.
		
		Arguments
		----------
		cosMin: minimum cos(zenith) defining the sampled solid angle range.
		cosMax: maximum cos(zenith) defining the sampled solid angle range.
		generated_surface: callable object that returns area Vs. cos(zenith).
		resampled_surface: callable object that returns area Vs. cos(zenith).
		                   Default is to take generated_surface.
		
		Detailed description
		--------------------
		
		These two requirements completely determine the function:
		 - The value returned must be proportional to the ratio
		   of two geometric acceptances.
		 - The mean of the returned weights must yield the integral of
		   the geometric acceptance of the resample cylinder, when
		   calculated over a sample drawn from the original distribution
		   (given by the generation cylinder),
		Consequently, if the two acceptances are the same, the result is
		a constant and equal to the total geometric acceptance.
		"""
		# sanity check
		assert abs(cosMin) <=1 and abs(cosMax) <=1, "unphysical cos(zenith) range: ({},{})".format(cosMin, cosMax)
		# set data members
		self.costhmin, self.costhmax = sorted((cosMin, cosMax))
		self.generated_surface = generated_surface
		self.resampled_surface = self.generated_surface if resampled_surface is None else resampled_surface
		# calculate the weight function
		import scipy.integrate
		self.acceptance = scipy.integrate.quad(lambda x,c=self.generated_surface: c(x), self.costhmin, self.costhmax)[0]

	def etendue(self):
		"""
		Integral of the cross-sectional area over an agular range (A * Omega).
		"""
		return self.acceptance

	def __call__(self, cth):
		"""
		geometric weight (NOT exactly the cross-sectional area).
		"""
		# all this juggling is just to guarantee one can handle different input shapes, including ()
		shape = numpy.array(cth).shape
		cth = numpy.atleast_1d(cth)
		norm = numpy.where( (cth>=self.costhmin)*(cth<=self.costhmax), self.etendue(), 0. )
		if self.generated_surface != self.resampled_surface:
			ok = (cth>=self.costhmin)*(cth<=self.costhmax)
			norm[ok] *= self.resampled_surface(cth[ok])/self.generated_surface(cth[ok])
		return norm.reshape(shape)

	def __repr__(self):
		return 'AngularGenerationDistribution({}, {}, {}, {})'.format(self.costhmin, self.costhmax, repr(self.generated_surface), repr(self.resampled_surface))

def NeutrinoGenerator(NEvents, FromEnergy, ToEnergy, GammaIndex, NeutrinoFlavor,
    ZenithMin=0, ZenithMax=numpy.pi, AzimuthMin=0, AzimuthMax=2*numpy.pi,
    InjectionMode='Circle', **kwargs):
	"""
	Construct a GenerationProbability appropriate for NeutrinoGenerator simulation. The arguments
	have the same meaning as those to the I3NeutrinoGenerator.
	
	.. warning:: The return value of the GenerationProbability will be in units of :math:`GeV^{-1} sr^{-1} m^{-2}`
	             rather than :math:`cm^{-2}`. Make sure that you use fluxes in the appropriate units!
	"""
	
	if InjectionMode == 'Circle':
		# Legacy NeutrinoGenerator samples on a cylindrical surface whose axis
		# passes through the origin and is parallel to the incoming neutrino.
		etendue = (numpy.cos(ZenithMin)-numpy.cos(ZenithMax))*(AzimuthMax-AzimuthMin)*numpy.pi*kwargs.get('InjectionRadius', 1200*I3Units.m)**2
	else:
		# Modern NeutrinoGenerator samples on an upright cylindrical surface
		l = kwargs.get('CylinderHeight', 1900*I3Units.m)
		r = kwargs.get('CylinderRadius', 950*I3Units.m)
		# differential acceptance (dA\Omega/d \cos\theta)
		etendue = UprightCylinderAcceptance(l, r)
	probs = []
	for p in (NeutrinoFlavor, NeutrinoFlavor+'Bar'):
		pt = getattr(ParticleType, p)
		# Half of the flux comes as neutrinos, half as anti-neutrinos
		probs.append(PowerLaw(-GammaIndex, FromEnergy, ToEnergy, (NEvents/2.), etendue, particle_type=pt))
	return GenerationProbabilityCollection(probs).to_PDG()

def _import_mysql():
	"Import the flavor of the month"
	import importlib
	for impl in 'MySQLdb', 'mysql.connector', 'pymysql':
		try:
			mysql = importlib.import_module(impl)
			return mysql
		except ImportError:
			pass

	raise ImportError('No MySQL bindings found!')

import ast
class SimprodFunction(ast.NodeTransformer):
	"""
	A callable version of simprod's $eval()
	"""
	ALLOWED_FUNCS = set(['int'])
	ALLOWED_NODE_TYPES = set(['Expression', 'Call', 'Name', 'Load', 'Num',
	    'BinOp', 'Add', 'Sub', 'Div', 'Mult', 'Mod'])
	
	def __init__(self, codestring, steering=dict()):
		super(SimprodFunction, self).__init__()
		# turn simprod syntax into valid Python
		codestring = codestring.replace('$', '').replace('::', '_')
		self.code = compile(self.visit(ast.parse(codestring, mode='eval')), '<AST>', 'eval')
		self.steering = dict([(k.replace('::', '_'), v) for k,v in steering.items()])
		
	def __call__(self, **kwargs):
		"""
		Evaluate the parsed expression. Keyword arguments not provided will
		be taken from the steering dictionary, if present.
		"""
		kw = dict(self.steering)
		kw.update(kwargs)
		return eval(self.code, kw)
	
	def visit_Call(self, node):
		if node.func.id == 'steering' or node.func.id == 'args':
			return ast.copy_location(ast.Name(node.args[0].id, ast.Load()), node)
		elif node.func.id == 'eval':
			return self.generic_visit(node.args[0])
		else:
			if node.func.id not in self.ALLOWED_FUNCS:
				raise RuntimeError("Invalid expression: %s() not allowed" % (node.func.id))
			return self.generic_visit(node)
		
	def generic_visit(self, node):
		nodetype = type(node).__name__
		if nodetype not in self.ALLOWED_NODE_TYPES:
			raise RuntimeError("Invalid expression: %s not allowed" % nodetype)
		return super(SimprodFunction, self).generic_visit(node)

def from_simprod(dataset_id, use_muongun=False, database='vm-i3simprod.icecube.wisc.edu'):
	"""
	Extreme laziness: parse weighting scheme out of the simulation production database
	
	.. note:: This requires MySQL bindings (e.g. mysql-connector-python)
	
	:param dataset_id: the number of the SimProd dataset
	:param use_muongun: default False
	:param database: database name (default to IceProd v1)
	:returns: an instance of :py:class:`GenerationProbability`
	          that, when called, will return the number of particles injected into the simulation
	          per :math:`\textrm{GeV} \, \textrm{m}^2` \, \textrm{sr}`.
	"""
	import re
	mysql = _import_mysql()
	
	try:
		db = mysql.connect(host=database, user='i3simprod-ro', passwd='Twed9~Dret', db='i3simprod')
	except mysql.OperationalError as e:
		reason = e.args[1]
		reason += " This might happen if you tried to connect to the simprod database from many cluster jobs in parallel. Don't do that. Instead, query the generator for your dataset once, and pass it to your jobs in a file."
		raise mysql.OperationalError(e.args[0], reason)
	cursor = db.cursor()
	
	if isinstance(dataset_id, str):
		raise UnboundLocalError
	cursor.execute("SELECT COUNT(*) FROM dataset WHERE dataset_id=%s", (dataset_id,))
	if cursor.fetchone()[0] == 0:
		raise ValueError("Dataset %s does not exist in the simprod database" % repr(dataset_id))
	
	# In case this is a post-processed set, chase the chain back until we hit the real generated set
	while True:
		cursor.execute("SELECT description FROM dataset WHERE dataset_id=%s", (dataset_id,))
		match = re.match(r'.*(from|of) dataset (\d{4})', cursor.fetchone()[0], re.IGNORECASE)
		if match:
			dataset_id = int(match.group(2))
		else:
			break
	
	# query category and number of completed files
	cursor.execute("SELECT category FROM dataset JOIN simcat ON dataset.simcat_id=simcat.simcat_id and dataset.dataset_id=%s", (dataset_id,))
	row = cursor.fetchone()
	category = row[0]
	
	typemap=dict(string=str, int=int, double=float, float=float, bool=bool)
	def get_steering(cursor, dataset_id):
		cursor.execute("SELECT name, type, value FROM steering_parameter WHERE dataset_id=%s", (dataset_id,))
		steering = {}
		for name, typus, value in cursor.fetchall():
			try:
				steering[name] = typemap[typus](value)
			except ValueError:
				steering[name] = value
				pass
		return steering
	steering = get_steering(cursor, dataset_id)
	
	if category == 'Test':
		if steering['mctype'] == 'corsika':
			category = 'CORSIKA-in-ice'
		elif steering['mctype'].startswith('nugen'):
			category = 'neutrino-generator'
	
	def _coerce_tray_parameter(row):
		if not row:
			return None
		if row[1] in typemap:
			try:
				return typemap[row[1]](row[2])
			except ValueError:
				# not a literal, must be a function
				return SimprodFunction(row[2], get_steering(cursor, dataset_id))
		else:
			cursor.execute("SELECT value FROM carray_element WHERE cparameter_id=%s", (row[0],))
			return [float(v[0]) for v in cursor.fetchall()]
	
	def get_tray_parameter(dataset_id, key, klass=None):
		if klass is None:
			cursor.execute("SELECT cparameter_id, type, value FROM cparameter WHERE dataset_id=%s AND name=%s ORDER BY tray_index ASC", (dataset_id, key))
		else:
			cursor.execute("SELECT cparameter_id, type, value FROM cparameter INNER JOIN (module_pivot, module) ON (module_pivot.module_id=module.module_id AND cparameter.module_pivot_id=module_pivot.module_pivot_id) WHERE module_pivot.dataset_id=%s AND cparameter.name=%s AND module.class=%s ORDER BY cparameter.tray_index ASC", (dataset_id, key, klass))
		values = list(map(_coerce_tray_parameter, cursor.fetchall()))
		if len(values) == 0:
			return None
		elif len(values) == 1:
			return values[0]
		else:
			return values
	
	def get_metaproject(dataset_id, tray_name, tray_index=None):
		"""
		Get metaproject version for a tray by name, or if that fails, by index
		"""
		cursor.execute("SELECT metaproject.name, metaproject.major_version, metaproject.minor_version, metaproject.patch_version FROM tray JOIN metaproject_pivot ON tray.tray_index=metaproject_pivot.tray_index AND tray.dataset_id=metaproject_pivot.dataset_id JOIN metaproject ON metaproject_pivot.metaproject_id=metaproject.metaproject_id WHERE tray.dataset_id=%s AND tray.name=%s", (dataset_id, tray_name))
		row = cursor.fetchone()
		if row is None and tray_index is not None:
			cursor.execute("SELECT metaproject.name, metaproject.major_version, metaproject.minor_version, metaproject.patch_version FROM tray JOIN metaproject_pivot ON tray.tray_index=metaproject_pivot.tray_index AND tray.dataset_id=metaproject_pivot.dataset_id JOIN metaproject ON metaproject_pivot.metaproject_id=metaproject.metaproject_id WHERE tray.dataset_id=%s AND tray.tray_index=%s", (dataset_id, tray_index))
			row = cursor.fetchone()
		metaproject, major, minor, patch = row
		prerelease = None
		if '-' in patch:
			patch, prerelease = patch.split('-')
		return (metaproject, int(major), int(minor), int(patch), prerelease)
		
	if category == 'neutrino-generator':
		if 'NUGEN::elogmin' in steering:
			emin, emax = 10**steering['NUGEN::elogmin'], 10**steering['NUGEN::elogmax']
		elif 'NUGEN::from_energy' in steering:
			emin, emax = steering['NUGEN::from_energy'], steering['NUGEN::to_energy']
		else:
			emin, emax = steering['NUGEN::emin'], steering['NUGEN::emax']
		nugen_kwargs = dict()
		if 'NUGEN::injectionradius' in steering:
			nugen_kwargs['InjectionRadius'] = steering['NUGEN::injectionradius']
		elif 'NUGEN::cylinder_length' in steering:
			nugen_kwargs['CylinderHeight'] = steering['NUGEN::cylinder_length']
			nugen_kwargs['CylinderRadius'] = steering['NUGEN::cylinder_radius']
		if get_metaproject(dataset_id, 'nugen', 0)[1:] >= (4,1,6):
			nugen_kwargs['InjectionMode'] = 'Cylinder'
		generator = NeutrinoGenerator(NEvents=steering['nevents'],
		    FromEnergy     =emin,
		    ToEnergy       =emax,
		    GammaIndex     =steering['NUGEN::gamma'],
		    NeutrinoFlavor =steering['NUGEN::flavor'],
		    ZenithMin      =steering['NUGEN::zenithmin']*I3Units.deg,
		    ZenithMax      =steering['NUGEN::zenithmax']*I3Units.deg,
		    **nugen_kwargs)
	elif category == 'CORSIKA-in-ice':
		if steering['composition'].startswith('5-component') or steering['composition'] == 'jcorsika':
			gamma = get_tray_parameter(dataset_id, "pgam")
			if gamma is None:
				gamma = [-2]*5
			else:
				gamma = [-abs(v) for v in gamma]
			norm = get_tray_parameter(dataset_id, "pnorm")
			if norm is None:
				norm = [10., 5., 3., 2., 1.]
			if get_tray_parameter(dataset_id, 'CutoffType') == "EnergyPerNucleon":
				LowerCutoffType = 'EnergyPerNucleon'
			else:
				LowerCutoffType = 'EnergyPerParticle'
			UpperCutoffType = get_tray_parameter(dataset_id, 'UpperCutoffType')
			if UpperCutoffType is None:
				corsika_version = get_tray_parameter(dataset_id, 'CorsikaVersion')
				if isinstance(corsika_version, list):
					corsika_version = corsika_version[-1]
				if corsika_version is None or '5comp' in corsika_version:
					# 5-component dCORSIKA only supports a lower cutoff
					UpperCutoffType = 'EnergyPerParticle'
				elif get_metaproject(dataset_id, 'generate', 0)[1] >= 4:
					#  Upper cutoff type appeared in IceSim 4, and defaults to the lower cutoff type
					UpperCutoffType = LowerCutoffType
				else:
					UpperCutoffType = 'EnergyPerParticle'
			length = get_tray_parameter(dataset_id, 'length', "icecube.simprod.generators.CorsikaGenerator")
			if length is None:
				if 'CORSIKA::length' in steering:
					length = steering['CORSIKA::length']*I3Units.m
				else:
					length = 1600*I3Units.m
					warnings.warn("No target cylinder length for dataset {dataset_id}! Assuming {length:.0f} m".format(**locals()))
			radius = get_tray_parameter(dataset_id, 'radius', "icecube.simprod.generators.CorsikaGenerator")
			if radius is None:
				if 'CORSIKA::radius' in steering:
					radius = steering['CORSIKA::radius']*I3Units.m
				else:
					radius = 800*I3Units.m
					warnings.warn("No target cylinder length for dataset {dataset_id}! Assuming {radius:.0f} m".format(**locals()))
			if use_muongun:
				from icecube import MuonGun
				nevents = steering['CORSIKA::showers']
				if gamma == [-2.0]*5 and norm == [10., 5., 3., 2., 1.]:
					model = 'Standard5Comp'
				elif gamma == [-2.6]*5 and norm == [3., 2., 1., 1., 1.]:
					model = 'CascadeOptimized5Comp'
				else:
					raise ValueError("Unknown CORSIKA configuration!")
				generator = nevents*MuonGun.corsika_genprob(model)
			else:
				generator = FiveComponent(steering['CORSIKA::showers'],
				    emin=steering['CORSIKA::eprimarymin']*I3Units.GeV,
				    emax=steering['CORSIKA::eprimarymax']*I3Units.GeV,
				    normalization=norm, gamma=gamma,
				    LowerCutoffType=LowerCutoffType, UpperCutoffType=UpperCutoffType,
				    height=length, radius=radius)
		elif steering['composition'].startswith('polygonato') or steering['composition'].startswith('Hoerandel'):
			if use_muongun:
				from icecube import MuonGun
				length = steering['CORSIKA::length']*I3Units.m
				radius = steering['CORSIKA::radius']*I3Units.m
				area = numpy.pi**2*radius*(radius+length)
				areanorm = 0.131475115*area
				generator = (steering['CORSIKA::showers']/areanorm)*MuonGun.corsika_genprob('Hoerandel5')
			else:
				generator = Hoerandel(steering['CORSIKA::showers'],
				    emin=steering['CORSIKA::eprimarymin']*I3Units.GeV,
				    emax=steering['CORSIKA::eprimarymax']*I3Units.GeV,
				    dslope=steering['CORSIKA::dslope'],
				    height=steering['CORSIKA::length']*I3Units.m,
				    radius=steering['CORSIKA::radius']*I3Units.m)
	elif category == 'CORSIKA-ice-top':
		
		# get the parent (generator) dataset, as the generator parameters may
		# be buried several generations back
		substeering = steering
		while not ('CORSIKA::ebin' in substeering and 'CORSIKA::radius' in substeering):
			try:
				substeering = get_steering(cursor, substeering['inputdataset'])
			except KeyError:
				# sampling radius is in the topsimulator config
				radius = get_tray_parameter(dataset_id, 'r', "icecube.simprod.modules.IceTopShowerGenerator")
				break
		else:
			# sampling radius is a steering parameter
			if type(substeering['CORSIKA::radius']) == str:
				radius = SimprodFunction(substeering['CORSIKA::radius'], substeering)
			else:
				radius = lambda CORSIKA_ebin: substeering['CORSIKA::radius']
		
		# logarithmic energy bin is a function of the procnum
		ebin = SimprodFunction(substeering['CORSIKA::ebin'], substeering)
		
		# check that the energy steps are spaced like we expect
		dlogE = ebin(procnum=1) - ebin(procnum=0)
		assert dlogE > 0, "Subsequent procnums end up in different energy bins"
		eslope = substeering['CORSIKA::eslope']
		assert eslope == -1, "Weighting scheme only makes sense for E^-1 generation"
		
		try:
			oversampling = substeering['CORSIKA::oversampling']
		except KeyError:
			oversampling = get_tray_parameter(dataset_id, 'samples', "icecube.simprod.modules.IceTopShowerGenerator")
		
		ctmin = numpy.cos(numpy.radians(substeering['CORSIKA::cthmax']))
		ctmax = numpy.cos(numpy.radians(substeering['CORSIKA::cthmin']))
		# projected area x solid angle: pi^2 r^2 (ctmax^2 - ctmin^2)
		
		emin = substeering['CORSIKA::ebin_first']
		emax = substeering['CORSIKA::ebin_last']
		num_ebins = int((emax - emin) / dlogE) + 1
		ebins = numpy.linspace(emin, emax, num_ebins)
		
		# go up further levels if necessary
		while not 'CORSIKA::primary' in substeering:
			substeering = get_steering(cursor, substeering['inputdataset'])
		primary = substeering['PRIMARY::%s' % substeering['CORSIKA::primary']]
		
		# number of showers in bin
		if type(substeering['CORSIKA::showers']) == str:
			nshowers =  SimprodFunction(substeering['CORSIKA::showers'], substeering)
		elif 'CORSIKA::showers' in substeering:
			nshowers = lambda CORSIKA_ebin: substeering['CORSIKA::showers']
		else:
			nshowers = lambda CORSIKA_ebin: 1.
		
		bin_r_n = numpy.array([(eb, radius(CORSIKA_ebin=eb), nshowers(CORSIKA_ebin=eb)) for eb in ebins])
		probs = []
		for (r, n), ebins in itertools.groupby(bin_r_n, lambda pair: (pair[1], pair[2])):
			ebins = [pair[0] for pair in ebins]
			probs.append(PowerLaw(eslope, 10**ebins[0], 10**(ebins[-1]+dlogE), n*len(ebins),
					      area=AngularGenerationDistribution(ctmin, ctmax, Circle(r)),
					      particle_type=ParticleType.values[primary]))
		
		# turn into a collection
		generator = GenerationProbabilityCollection(probs).to_PDG()
		# normalize to relative proportion in each bin
		generator /= sum([prob.nevents for prob in probs])
		# and scale to total number of showers with over-sampling
		generator *= oversampling
	
	else:
		raise ValueError("No weighting scheme implemented for %s simulations" % (category))
	cursor.close()
	db.close()
	return generator

class EnergyWeight(object):
	def __init__(self, target_flux, generation_spectrum):
		self.target_flux = target_flux
		self.generation_spectrum = generation_spectrum
	def __call__(self, E, zenith):
		return self.target_flux(E)/self.generation_spectrum(E, zenith)


class IceTopDatasetInfo(object):
	"""
	Simple class to store the typical information always used in IceTop simulations
	"""
	def __init__(self, data):
		self.primary = data['primary']
		self.ebin_first = data['ebin_first'] # min log(E) bin
		self.ebin_last = data['ebin_last'] # max log(E) bin
		self.zenith_min = data['zenith_min']
		self.zenith_max = data['zenith_max']
		self.nruns = data['nruns'] # number of corsika runs
		self.nshowers = data['nshowers'] # number of showers per corsika file (int), or function of ebin, defaults to 1
		self.oversampling = data['oversampling'] # how many times a corsika shower is sampled, default is?
		self.radius = data['radius'] #float or function of ebin
		# these never change so far...
		self.d_log_e = 0.1
		self.eslope = -1
		num_ebins = int((self.ebin_last - self.ebin_first) / self.d_log_e) + 1
		self.ebins = numpy.linspace(self.ebin_first, self.ebin_last, num_ebins)

	def __str__(self):
		import pprint
		d = self.__dict__.copy()
		del d['ebins']
		return pprint.pformat(self.__dict__)
	def __repr__(self):
		return 'IceTopDatasetInfo(%s)'%str(self.__dict__)


def icetop_mc_weights(dataset_id, dataset_file):
	"""
	This does something similar to from_simprod, but from an ad-hoc json file
	
	:param dataset_id: the number of the SimProd dataset
	:param dataset_file: a json file containing dataset info
	:returns: an instance of :py:class:`GenerationProbability`
	          that, when called, will return the number of particles injected into the simulation
	          per :math:`\textrm{GeV} \, \textrm{m}^2` \, \textrm{sr}`."
	"""
	from collections import OrderedDict
	import json
	import os

	def _mergedicts_(x, y):
		z = x.copy()   # start with x's keys and values
		z.update(y)    # modifies z with y's keys and values & returns None
		return z

	if not os.path.exists(dataset_file):
		raise Exception('dataset sampling info file %s does not exist'%dataset_file)
	dataset = IceTopDatasetInfo(json.load(open(dataset_file), object_pairs_hook=OrderedDict)[str(dataset_id)])

	if not dataset.primary in ParticleType.__dict__:
		raise Exception("The primary particle for dataset {dataset_id} is {primary}, but there is no corresponding value in ParticleType".format(dataset_id=dataset_id, primary=dataset.primary))

	# variables
	ctmax = numpy.cos(numpy.radians(dataset.zenith_min))
	ctmin = numpy.cos(numpy.radians(dataset.zenith_max))
	primary = eval("ParticleType.%s"%dataset.primary, globals())
	ebin = dataset.ebins

	# flat detector: projected area x solid angle: pi^2 r^2 (ctmax^2 - ctmin^2)

	# so far, radius, nshowers and nruns are functions of energy
	radius = dataset.radius
	nshowers = dataset.nshowers
	nruns = dataset.nruns
	if type(nruns)==int: nruns /= len(ebin)
	if not isinstance(radius,  (list,tuple,numpy.ndarray)):
		radius = numpy.array([eval(str(radius), globals(), _mergedicts_(dataset.__dict__, {'ebin':e})) for e in ebin])
	if not isinstance(nshowers,  (list,tuple,numpy.ndarray)):
		nshowers = numpy.array([eval(str(nshowers), globals(), _mergedicts_(dataset.__dict__, {'ebin':e})) for e in ebin])
	if not isinstance(nruns,  (list,tuple,numpy.ndarray)):
		nruns = numpy.array([eval(str(nruns), globals(), _mergedicts_(dataset.__dict__, {'ebin':e})) for e in ebin])
	assert len(radius) == len(ebin), 'len(radius) != len(ebin): %d != %d'%(len(radius), len(ebin))
	assert len(nshowers) == len(ebin), 'len(nshowers) != len(ebin): %d != %d'%(len(nshowers), len(ebin))
	assert len(nruns) == len(ebin), 'len(nruns) != len(ebin): %d != %d'%(len(nruns), len(ebin))

	bin_r_n = zip(ebin, radius, nshowers, nruns)
	probs = []
	for (r, ns, nr), ebins in itertools.groupby(bin_r_n, lambda pair: (pair[1], pair[2], pair[3])):
		ebins = [pair[0] for pair in ebins]
		probs.append(PowerLaw(dataset.eslope, 10**ebins[0], 10**(ebins[-1]+dataset.d_log_e), dataset.oversampling*ns*nr*len(ebins),
				      area=AngularGenerationDistribution(ctmin, ctmax, Circle(r)),
				      particle_type=ParticleType.values[primary]))

	# turn into a collection
	generator = GenerationProbabilityCollection(probs).to_PDG()

	return generator


