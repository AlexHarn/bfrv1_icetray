
"""
A collection of cosmic ray flux paramerizations.
"""

import numpy
import operator
import copy
import numexpr

from .weighting import I3Units, ParticleType, PDGCode

def build_lookup(mapping, var='ptype', default='ptype'):
	"""
	Build an expression equivalent to a lookup table
	"""
	if len(mapping) > 0:
		return 'where(%s==%s, %s, %s)' % (var, mapping[0][0], mapping[0][1], build_lookup(mapping[1:], var, default))
	else:
		return str(default)

class CompiledFlux(object):
	"""
	An efficient pre-compiled form of a multi-component flux. For single-element evalutions
	this is ~2 times faster than switching on the primary type with an if statement; for 1e5
	samples it is 2000 times faster than operating on masked slices for each primary type.
	"""
	pdg_to_corsika = numexpr.NumExpr(build_lookup([(int(PDGCode.from_corsika(v)), v) for v in ParticleType.values.keys()]))
	def __init__(self, expr):
		self.expr = numexpr.NumExpr(expr)
		# by default, assume PDG codes
		self._translator = CompiledFlux.pdg_to_corsika
	
	def to_PDG(self):
		"""
		Convert to a form that takes PDG codes rather than CORSIKA codes.
		"""
		new = copy.copy(self)
		new._translator = CompiledFlux.pdg_to_corsika
		return new
	
	def __call__(self, E, ptype):
		"""
		:param E: particle energy in GeV
		:param ptype: particle type code
		:type ptype: int
		"""
		if self._translator:
			ptype = self._translator(ptype)
		return self.expr(E, ptype)
	
	@staticmethod
	def build_lookup(mapping, var='ptype', default=0.):
		"""
		Build an expression equivalent to a lookup table
		"""
		# force mapping to be a list if it wasn't already
		mapping=list(mapping)
		if len(mapping) > 0:
			return 'where(%s==%s, %s, %s)' % (var, mapping[0][0], mapping[0][1], build_lookup(mapping[1:], var, default))
		else:
			return str(default)

class Hoerandel(CompiledFlux):
	"""
	All-particle spectrum (up to iron) after Hoerandel_, as implemented
	in dCORSIKA.
	
	.. _Hoerandel: http://dx.doi.org/10.1016/S0927-6505(02)00198-6
	"""
	delta_gamma = 2.1
	eps_cutoff = 1.9
	E_knee = 4.49*I3Units.PeV
	z = "where(ptype > 100, ptype%100, 1)"
	knee = "(1+(E/(%(E_knee)s*%(z)s))**%(eps_cutoff)s)**(-%(delta_gamma)s/%(eps_cutoff)s)" % locals()
	ptypes = [14, 402, 703,  904, 1105, 1206, 1407, 1608, 1909, 2010, 2311, 2412, 2713, 2814, 3115, 3216, 3517, 4018, 3919, 4020, 4521, 4822, 5123, 5224, 5525, 5626]
	def __init__(self):
		gamma = numpy.array([2.71, 2.64, 2.54, 2.75, 2.95, 2.66, 2.72, 2.68, 2.69, 2.64, 2.66, 2.64, 2.66, 2.75, 2.69, 2.55, 2.68, 2.64, 2.65, 2.7, 2.64, 2.61, 2.63, 2.67, 2.46, 2.59])
		flux = numpy.array([0.0873, 0.0571, 0.00208, 0.000474, 0.000895, 0.0106, 0.00235, 0.0157, 0.000328, 0.0046, 0.000754, 0.00801, 0.00115, 0.00796, 0.00027, 0.00229, 0.000294, 0.000836, 0.000536, 0.00147, 0.000304, 0.00113, 0.000631, 0.00136, 0.00135, 0.0204])
		flux *= (I3Units.TeV/I3Units.GeV)**(gamma-1) # unit conversion
		expr = "%(flux)s*E**(-%(gamma)s)*%(knee)s" % dict(gamma=self.build_lookup(zip(self.ptypes, gamma)), flux=self.build_lookup(zip(self.ptypes, flux)), knee=self.knee)
		CompiledFlux.__init__(self, expr)

class Hoerandel5(Hoerandel):
	"""
	Hoerandel_ with only 5 components, after Becherini_ et al. (also the same as Arne_ Schoenwald's version)
	
	.. _Hoerandel: http://dx.doi.org/10.1016/S0927-6505(02)00198-6
	.. _Arne: http://www.ifh.de/~arnes/Forever/Hoerandel_Plots/
	.. _Becherini: http://dx.doi.org/10.1016/j.astropartphys.2005.10.005
	"""
	ptypes = [getattr(ParticleType, p) for p in ('PPlus', 'He4Nucleus', 'N14Nucleus', 'Al27Nucleus', 'Fe56Nucleus')]
	def __init__(self):
		gamma = numpy.array([2.71, 2.64, 2.68, 2.67, 2.58])
		flux = numpy.array([8.73e-2, 5.71e-2, 3.24e-2, 3.16e-2, 2.18e-2])
		flux *= (I3Units.TeV/I3Units.GeV)**(gamma-1) # unit conversion
		expr = "%(flux)s*E**(-%(gamma)s)*%(knee)s" % dict(gamma=self.build_lookup(zip(self.ptypes, gamma)), flux=self.build_lookup(zip(self.ptypes, flux)), knee=self.knee)
		CompiledFlux.__init__(self, expr)

class Hoerandel_IT(Hoerandel):
	"""
	Modified 5-component Hoerandel spectrum with N and Al replaced by O.
	"""
	ptypes = [getattr(ParticleType, p) for p in ('PPlus', 'He4Nucleus', 'O16Nucleus', 'Fe56Nucleus')]
	def __init__(self):
		gamma = numpy.array([2.71, 2.64, 2.68, 2.58])
		flux = numpy.array([8.73e-2, 5.71e-2, 6.40e-2, 2.18e-2])
		flux *= (I3Units.TeV/I3Units.GeV)**(gamma-1) # unit conversion
		expr = "%(flux)s*E**(-%(gamma)s)*%(knee)s" % dict(gamma=self.build_lookup(zip(self.ptypes, gamma)), flux=self.build_lookup(zip(self.ptypes, flux)), knee=self.knee)
		CompiledFlux.__init__(self, expr)

class GaisserHillas(CompiledFlux):
	"""
	Spectral fits from an `internal report`_ (also on the arXiv) by Tom Gaisser_.
	
	.. _`internal report`: http://internal.icecube.wisc.edu/reports/details.php?type=report&id=icecube%2F201102004
	.. _Gaisser: http://arxiv.org/abs/1111.6675v2
	"""
	ptypes = [getattr(ParticleType, p) for p in ('PPlus', 'He4Nucleus', 'N14Nucleus', 'Al27Nucleus', 'Fe56Nucleus')]
	def get_expression(self, flux, gamma, rigidity):
		z = "where(ptype > 100, ptype%100, 1)"
		return "%(flux)s*E**(-%(gamma)s)*exp(-E/(%(rigidity)s*%(z)s))" % locals()
	def get_flux(self):
		return [[7860., 3550., 2200., 1430., 2120.]]
	def get_gamma(self):
		return [[2.66, 2.58, 2.63, 2.67, 2.63]]
	def get_rigidity(self):
		return [4*I3Units.PeV]
	def __init__(self):
		flux = [self.build_lookup(zip(self.ptypes, f)) for f in self.get_flux()]
		gamma = [self.build_lookup(zip(self.ptypes, g)) for g in self.get_gamma()]
		rigidity = self.get_rigidity()
		CompiledFlux.__init__(self, "+".join([self.get_expression(f, g, r) for f, g, r in zip(flux, gamma, rigidity)]))

class GaisserH3a(GaisserHillas):
	"""
	Spectral fits from an `internal report`_ (also on the arXiv) by Tom Gaisser_.
	
	.. _`internal report`: http://internal.icecube.wisc.edu/reports/details.php?type=report&id=icecube%2F201102004
	.. _Gaisser: http://arxiv.org/abs/1111.6675v2
	
	The model H3a with a mixed extra-galactic population (Fig. 2)
	has all iron at the highest energy and would represent a
	scenario in which the cutoff is not an effect of energy loss
	in propagation over cosmic distances through the CMB but is
	instead just the extra-galactic accelerators reaching their
	highest energy.
	"""
	def get_flux(self):
		return super(GaisserH3a, self).get_flux() + [[20]*2 + [13.4]*3, [1.7]*2 + [1.14]*3]
	def get_gamma(self):
		return super(GaisserH3a, self).get_gamma() + [[2.4]*5, [2.4]*5]
	def get_rigidity(self):
		return super(GaisserH3a, self).get_rigidity() + [30*I3Units.PeV, 2e3*I3Units.PeV]

class GaisserH4a(GaisserH3a):
	"""
	Spectral fits from an `internal report`_ (also on the arXiv) by Tom Gaisser_.
	
	.. _`internal report`: http://internal.icecube.wisc.edu/reports/details.php?type=report&id=icecube%2F201102004
	.. _Gaisser: http://arxiv.org/abs/1111.6675v2
	
	In the model H4a, on the other hand, the extra-galactic component
	is assumed to be all protons.
	"""
	def get_flux(self):
		return super(GaisserH4a, self).get_flux()[:-1] + [[200]]
	def get_gamma(self):
		return super(GaisserH4a, self).get_gamma()[:-1] + [[2.6]]
	def get_rigidity(self):
		return super(GaisserH4a, self).get_rigidity()[:-1] + [60e3*I3Units.PeV]


class GaisserH4a_IT(object):
	"""
	Variation of Gaisser's H4a flux using only four components.
	
	*This is not a very physical flux*: The Oxygen group is the sum of H4a's Nitrogen and Aluminum groups.
	This is the flux used as an "a priori" estimate of mass-composition to produce the IceTop-only flux.
	Reference: M. G. Aartsen et al. PHYSICAL REVIEW D 88, 042004 (2013)
	"""
	ptypes = [getattr(ParticleType, p) for p in ('PPlus', 'He4Nucleus', 'O16Nucleus', 'Fe56Nucleus')]
	def __init__(self):
		self.h4a = GaisserH4a()
		self._translator = self.h4a._translator
	def __call__(self, E, ptype):
		"""
		:param E: particle energy in GeV
		:param ptype: particle type code
		:type ptype: int
		"""
		result = self.h4a(E,ptype)
		if numpy.ndim(ptype)==0:
			if ptype == PDGCode.N14Nucleus or ptype == PDGCode.Al27Nucleus:
				result = numpy.zeros_like(E)
			if ptype == PDGCode.O16Nucleus:
				result = self.h4a(E,PDGCode.N14Nucleus) + self.h4a(E,PDGCode.Al27Nucleus)
		else:
			result[(ptype==PDGCode.N14Nucleus) | (ptype==PDGCode.Al27Nucleus)] = 0
			result[ptype==PDGCode.O16Nucleus] = self.h4a(E[ptype==PDGCode.O16Nucleus], PDGCode.N14Nucleus) + \
			                                    self.h4a(E[ptype==PDGCode.O16Nucleus], PDGCode.Al27Nucleus)
		return result


class Honda2004(CompiledFlux):
	"""
	Spectrum used to calculate neutrino fluxes in `Honda et al. (2004)`_ (Table 1, with modification from the text).
	
	NB: the E_k notation means energy per nucleon!
	
	.. _`Honda et al. (2004)`: http://link.aps.org/doi/10.1103/PhysRevD.70.043008
	"""
	ptypes = [getattr(ParticleType, p) for p in ('PPlus', 'He4Nucleus', 'N14Nucleus', 'Al27Nucleus', 'Fe56Nucleus')]
	def flux(self):
		return ["where(E<100, 14900, 14900*(100**(2.71-2.74)))", 600., 33.2, 34.2, 4.45]
	def gamma(self):
		return ["where(E<100, 2.74, 2.71)", 2.64, 2.60, 2.79, 2.68]
	def b(self):
		return [2.15, 1.25, 0.97, 2.14, 3.07]
	def c(self):
		return [0.21, 0.14, 0.01, 0.01, 0.41]
	def __init__(self):
		A = "where(ptype>100, ptype/100, 1)"
		E_k = "E/%(A)s" % locals()
		flux = self.build_lookup(zip(self.ptypes, self.flux()))
		gamma = self.build_lookup(zip(self.ptypes, self.gamma()))
		b = self.build_lookup(zip(self.ptypes, self.b()))
		c = self.build_lookup(zip(self.ptypes, self.c()))
		# dN/dE = 1/A * dN/dE_k
		expr = "(%(flux)s/%(A)s)*(%(E_k)s + %(b)s*exp(-%(c)s * sqrt(%(E_k)s)))**(-%(gamma)s)" % locals()
		CompiledFlux.__init__(self, expr)

class TIG1996(CompiledFlux):
	"""
	Spectrum used to calculate prompt neutrino fluxes in `Enberg et al. (2008)`_ (Eq. 30).
	The parameterization was taken directly from an earlier paper by Thunman_ et al.
	Only the nucleon flux was given, so for simplicity we treat it as a proton-only flux.
	
	.. _`Enberg et al. (2008)`: http://dx.doi.org/10.1103/PhysRevD.78.043005
	.. _Thunman: http://arxiv.org/abs/hep-ph/9505417
	"""
	def __init__(self):
		ptype = int(ParticleType.PPlus)
		flux = "where(E<5e6, 1.7, 174)"
		gamma = "where(E<5e6, 2.7, 3)"
		expr = "1e4*%(flux)s*E**(-%(gamma)s)" % locals()
		CompiledFlux.__init__(self, "where(ptype==%(ptype)s, %(expr)s, 0)" % locals())


class GlobalFitGST(CompiledFlux):
	"""
	Spectral fits_ by Gaisser, Stanev and Tilav.
	
	.. _fits: http://arxiv.org/pdf/1303.3565v1.pdf
	"""
	ptypes = [getattr(ParticleType, p) for p in ('PPlus', 'He4Nucleus', 'N14Nucleus', 'Al27Nucleus', 'Fe56Nucleus')]
	def get_expression(self, flux, gamma, rigidity):
		z = "where(ptype > 100, ptype%100, 1)"
		return "%(flux)s*E**(-%(gamma)s)*exp(-E/(%(rigidity)s*%(z)s))" % locals()
	def get_flux(self):
		return 	[	[7000.	,3200.	,100.	,130.	,60.],
					[150.	,65.	,6.		,7.		,2.3],
					[14.	,0		,0		,0		,0.025]]
	def get_gamma(self):
		return [	[2.66	,2.58	,2.4	,2.4	,2.3],
					[2.4	,2.3	,2.3	,2.3	,2.2],
					[2.4	,0		,0		,0		,2.2]]
	def get_rigidity(self):
		return [120*I3Units.TeV,
				4*I3Units.PeV,
				1.3e3*I3Units.PeV]
	def __init__(self):
		flux = [self.build_lookup(zip(self.ptypes, f)) for f in self.get_flux()]
		gamma = [self.build_lookup(zip(self.ptypes, g)) for g in self.get_gamma()]
		rigidity = self.get_rigidity()
		CompiledFlux.__init__(self, "+".join([self.get_expression(f, g, r) for f, g, r in zip(flux, gamma, rigidity)]))


class FixedFractionFlux(object):
	"""
	Total energy per particle flux flux split among mass groups with a constant fraction.
	
	By default, this takes as basis the flux from Gaisser H4a summed over all its mass groups,
	then multiplies it by the given fraction. This is a quick way to consider different weightings for systematic checks.
	"""
	corsika_to_pdg = numexpr.NumExpr(build_lookup([(int(ParticleType.from_pdg(v)), v) for v in PDGCode.values.keys()]))
	def __init__(self, fractions={}, basis=GaisserH4a_IT(), normalized=True):
		"""
		:param fractions: A dictionary of fractions. They must add up to one and they should correspond to the ptypes in basis
		:type fractions: a dictionary with dataclasses.ParticleType as keys
		"""
		self.flux = basis
		# the following is weird, and it happens because Hoerandel's ptypes are ints, not enums
		self.ptypes = [(p if type(p)==int else getattr(PDGCode, p.name)) for p in basis.ptypes] # this is weird...
		self.fractions = {k:0 for k in self.ptypes}
		self.fractions.update(fractions)
		if normalized:
			assert(sum(self.fractions.values())==1.)
		assert(len(self.fractions)==len(self.ptypes))

	def particle_types(self):
		"""
		Transform from PDGCode to internal particle codes
		"""
		# Flux objects have an internal list of representative particle types stored in self.ptypes.
		# These are usually corsika codes (ParticleType),
		# but the values passed as arguments to __call__ are PDGCodes, which are then translated.
		# Here we have to do the inverse transformation. Unfortunately, there is no self._inverse_translator in CompiledFlux.
		if self.flux._translator:
			return [self.corsika_to_pdg(p) for p in self.flux.ptypes]

	def __call__(self, E, ptype):
		"""
		:param E: particle energy in GeV
		:param ptype: particle type code
		:type ptype: int
		"""
		E, ptype = numpy.broadcast_arrays(E, ptype)
		if numpy.ndim(ptype) == 0:
			return sum([self.flux(E, p) for p in self.particle_types()])*self.fractions[int(ptype)]
		fractions = numpy.zeros_like(ptype)
		for p in self.ptypes:
			fractions[ptype==p] = self.fractions[p]
		return sum([self.flux(E, p) for p in self.particle_types()])*fractions

