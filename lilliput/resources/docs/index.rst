.. _lilliput:

Lilliput
========

**Maintainer:** Kai Krings <kai.krings@icecube.wisc.edu>

The :mod:`lilliput` project is a collection of `Gulliver` services, that
is: services that are implementations of the interfaces (base classes)
defined in the `Gulliver` project, for likelihood functions, minimizers,
parametrization and seed preparation. For a full description of these
interfaces and how they are used in a likelihood-based reconstruction
module, see the :mod:`gulliver` and :mod:`gulliver-modules` projects,
respectively.

.. toctree::
    :titlesonly:

    release_notes
    python_api


Likelihood services
-------------------

:class:`I3GulliverIPDFPandel`

    A wrapper for all likelihood functions implemented in the
    :mod:`ipdf` project. It supports two PDFs: `Pandel` and
    `PhotoSpline`; several strategies to the likelihood in a DOM:
    `SPEAll`, `SPE1st`, `MPE`, and `PSA`; and several different `event
    hypothesis`: infinite muon, point-like cascade, and directional
    cascade.  The `Pandel` PDF can be configured with several hole ice
    models but this is not supported by default, you have to enable this
    with a `cmake` flag.


Minimizer services
------------------

:class:`I3GulliverMinuit`

    A wrapper for the `TMinuit` function minimization tool in the `ROOT`
    libraries. It is the default minimizer for most reconstructions, but
    it is kind of a poorly documented black box and produces lots of
    annoying unquenchable noise.

:class:`I3GSLSimplex`

    A wrapper for the better documented `GSL` implementation of the
    `simplex` algorithm. It has not yet been proven (in :mod:`gulliver`)
    that this minimizer performs as good or better than Minuit, but as
    soon as we achieve that state, we can happily discard the old
    Minuit.


Parametrization services
------------------------

:class:`I3SimpleParametrization`

    Parametrizes :class:`I3Particle` objects in the most trivial way:
    fitted variables are just `x`, `y`, `z`, `t`, `zenith`, `azimuth`,
    `length`, `energy` without any clever transformations, except for
    the energy and the length (can be logarithmic instead of linear).
    Variables chosen to be fitted by configuring a positive step size
    for them. Bounds can be specified, but are not mandatory.

:class:`I3HalfSphereParametrization`

    Parametrizes :class:`I3Particle` objects in a less trivial way:
    fitted variables `x`, `y`, `z`, `t`, `length`, and `energy` are
    treated the same way, but the direction coordinates are forced to be
    Cartesian by projecting them onto a tangential plane which touches
    the direction sphere at the direction of the initial seed for the
    reconstruction. This means that only half of the direction sphere is
    accessible, the reconstruction can not move more than 90 degrees
    away from the initial direction.


Seed services
-------------

:class:`I3BasicSeedService`

    Uses any number :class:`I3Particle` objects, e.g. from a first-guess
    module (:mod:`linefit`, :mod:`dipolefit`, ...) or from some other
    likelihood reconstruction, and prepares it as a seed for the
    likelihood reconstruction. For "infinite tracks" this includes
    shifting the vertex as close as possible to the center-of-gravity of
    the hits/pulses. For any event type the vertex time can be adjusted
    (according to various recipes) such that the distribution of time
    residuals of the hits/pulses is reasonable, e.g. such that the
    lowest time residual is zero. With a bad initial time residual
    distribution the minimizer might get "lost" right at the start of
    the search.


.. note::

    Lilliput is an island on which the most well-known part of the story
    of `Gulliver's Travels`_, the novel written by `Jonathan Swift`_
    (1726, amended 1735), takes place. On his first voyage the
    adventurer/surgeon/captain Lemuel Gulliver is washed ashore on the
    island Lilliput after a shipwreck and finds himself surrounded by
    very many very small people. Originally, the idea was to host only
    the minimizer services in the :mod:`lilliput` project, and to host
    the other services in different projects with other names of islands
    and countries in the same novel, such as Brobdingnag (inhabited by
    giants), Houyhnhnm (governed by horses) and Balnibarbi (mad
    scientists); but this met with fierce resistance from the IceRec
    overlord, so I succumbed, and crammed all services into this one
    project.

    Have a look at `Project Gutenberg`_.

.. _Gulliver's Travels:
    http://en.wikipedia.org/wiki/Gulliver%27s_Travels

.. _Jonathan Swift:
    http://en.wikipedia.org/wiki/Jonathan_Swift

.. _Project Gutenberg:
    http://www.gutenberg.org/files/17157/17157-h/17157-h.htm


.. seealso::

    `Doxygen documentation <../../doxygen/lilliput/index.html>`_,
    :ref:`gulliver`, :ref:`gulliver-modules`
