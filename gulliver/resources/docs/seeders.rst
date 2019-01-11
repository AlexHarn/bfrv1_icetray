======================================
How to implement your own seed service
======================================

Overview
--------

A log-likelihood reconstructed must be seeded with a hypothesis (e.g.
the result of a first-guess reconstruction) relatively close to the
global minimum in the likelihood space. For Gulliver-based
reconstructions, a hypothesis is represented as an
:cpp:class:`I3EventHypothesis` object. Obviously, the seed hypothesis
must be an object of the same type. The :cpp:class:`I3SeedServiceBase`
class collects one or several first-guess reconstruction results
(:cpp:class:`I3Particle` objects) from an event, forms seed event
hypotheses, and hands a seed to :cpp:class:`I3Gulliver` via the
:cpp:func:`I3SeedServiceBase::GetSeed` method. Usually, this happens
inside a Guilliver-based module like :cpp:class:`I3SimpleFitter`
(:mod:`gulliver-modules`).

For most trivial event hypotheses that do not use
:cpp:member:`I3EventHypothesis::nonstd`, you can use the
:cpp:class:`I3BasicSeedSerive` class (:mod:`lilliput`) if you want to
implement your own Gulliver reconstruction. This seed service collects a
list of first-guess reconstruction results and provides different
options to correct the seed's vertex position and time (for *infinite*
tracks). If your event hypothesis needs other event data that is not
provided by the :cpp:class:`I3Particle` class, you definitely need to
implement your own seed service. Projects that define their own seed
service are: `double-muon <../double-muon/index.html>`_ and
:mod:`toprec`.


API
---

All seed services have to be derived from :cpp:class:`I3SeedServiceBase`
and should implement the following pure virtual methods:

.. cpp:class:: I3SeedServiceBase

    .. cpp:function:: virtual unsigned int SetEvent(const I3Frame &f)

        Provide event data: get the first-guess tracks; fill blank data
        members; do tweaks like space and time corrections.

        The generated seeds should all have an *OK* fit status and
        *non-NAN* position, direction, and time data members.

    .. cpp:function:: virtual I3EventHypothesis GetSeed(unsigned int iseed) const

        Return a seed as a deep copy of some internally stored seed
        because :cpp:class:`I3Gulliver` will most likely change its
        values.

    .. cpp:function:: virtual I3EventHypothesis GetDummy() const

        Return a *dummy* seed; useful in case all first guesses fail. In
        particular if the seeds normally have a *non-NULL*
        :cpp:member:`nonstd` datamember, then this should be initialized
        in this dummy seed as well.

    .. cpp:function:: virtual I3EventHypothesis GetCopy(const I3EventHypothesis &eh) const

        Return a (deep) copy of an existing event hypothesis.

    .. cpp:function:: virtual void Tweak(const I3EventHypothesis &eh) const

        When a seed service generates a seed, it may perform some tweaks
        or transformations on it, intended to facilitate the
        minimization. The generic example is the vertex correction for
        an infinite track.

    .. cpp:function:: virtual void FillInTheBlanks(const I3EventHypothesis &eh) const

        Many first guess algorithms do not provide the complete
        description for the kind of event we are trying to reconstruct.
        A simple example is the energy of a cascade or muon. The seed
        service might take care of filling in a sensible initial value
        for such a quantity.


Example
-------

Example for an :cpp:class:`I3SeedServiceBase` implementation in Python
that seeds a reconstruction with different directions based on a
`healpy` grid::

    import healpy
    import numpy

    import icecube
    import icecube.icetray
    import icecube.dataclasses
    import icecube.gulliver


    class HEALPixSeedService(icecube.gulliver.I3SeedService):
        """Seed service implementation based on HEALPix

        Seed reconstruction with directions based on a full-sky HEALPix
        grid.

        """
        def __init__(self, vertex_fit="", nside=32):
            super(HEALPixSeedService, self).__init__()

            self.key = vertex_fit
            self.nside = nside

            self.npix = healpy.nside2npix(self.nside)
            self.pixels = numpy.arange(self.npix)

        def SetEvent(self, frame):
            self.vertex_fit = frame[self.key]
            return self.npix

        def GetSeed(self, i):
            eh = icecube.gulliver.I3EventHypothesis(self.vertex_fit)
            eh.particle.shape = icecube.dataclasses.I3Particle.InfiniteTrack
            eh.particle.speed = icecube.dataclasses.I3Constants.c
            eh.particle.length = numpy.nan
            eh.particle.dir = icecube.dataclasses.I3Direction(
                *healpy.pix2ang(self.nside, self.pixels[i]))
            return eh

        def GetCopy(self, eh):
            return icecube.gulliver.I3EventHypothesis(eh.particle)

        def GetDummy(self):
            p = icecube.dataclasses.I3Particle()
            return icecube.gulliver.I3EventHypothesis(p)

        def Tweak(self, eh):
            return

        def FillInTheBlanks(self, eh):
            return

.. _healpy: https://healpy.readthedocs.org/en/latest/
