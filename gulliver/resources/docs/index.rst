.. _gulliver:

Gulliver
========

**Maintainer:** Alan Coleman <alanc@udel.edu>

.. toctree::
   :maxdepth: 1

   release_notes


Gulliver is a Generic Utility for Log-Likelihood-based Reconstructions.
This project provides base classes for the components that define a
generic log-likelihood based reconstruction in IceCube/AMANDA, and as of
2009 also in KM3NET.

A fitter class :class:`I3Gulliver` combines these components into a unit that
can be used to obtain a single maximum likelihood fit as obtained by running
the minimizer starting from a given seed. The "maximum log-likelihood"
reconstruction approach in AMANDA/IceCube is explained in more detail
in [Ahrens]_.

This utility was originally created to facilitate the implementation of
standard AMANDA reconstruction methods and to stimulate the
experimentation with new reconstruction ideas in an easily configurable
and transparent way. It does not intend to accommodate really **all**
possible likelihood-based reconstruction algorithms.

.. rubric:: Table of contents

.. toctree::
   :maxdepth: 2

   components
   anatomy
   minimizers
   loglikelihoods
   parametrizations
   seeders

.. seealso::

  `Doxygen documentation <../../doxygen/gulliver/index.html>`_,
  :ref:`gulliver-modules`, :ref:`lilliput`, :ref:`double-muon`,
  :ref:`finiteReco`, :ref:`millipede-main`, :ref:`paraboloid`.

.. rubric:: About the name

Gulliver is a **G**\ eneric **U**\ tility for
**L**\ og-**L**\ ikelihood-based **R**\ econstructions. The **I**, **V**
and **E** could stand for **I**\ ce (or **I**\ nspiration), neutrino
(:math:`\nu` == **V**) and **E**\ verything **E**\ lse, respectively.

.. note::

  Lemuel Gulliver is the main character in `Gulliver's Travels`_, a
  novel written by `Jonathan Swift`_ (1726, amended 1735). Originally,
  the idea was to host implementations of each of the base classes in
  their own little project, each named after islands and countries in
  the same novel, such as Lilliput (inhabited by very small people),
  Brobdingnag (inhabited by giants), Houyhnhnm (governed by horses) and
  Balnibarbi (mad scientists); but this met with fierce resistance from
  the IceRec overlord, so I succumbed, and crammed all implemented
  services into the ``lilliput`` project (which was originally only
  intended for the minimizer services, for obvious reasons).

  Have a look at `Project Gutenberg`_.

.. _Gulliver's Travels:
  http://en.wikipedia.org/wiki/Gulliver%27s_Travels

.. _Jonathan Swift:
  http://en.wikipedia.org/wiki/Jonathan_Swift

.. _Project Gutenberg:
  http://www.gutenberg.org/files/17157/17157-h/17157-h.htm

.. rubric:: References

.. [Ahrens]
  AMANDA Collaboration (Ahrens et al.).
  "Muon Track Reconstruction and Data Selection Techniques in AMANDA"
  In: *Nucl.Instrum.Meth.* A524 (2004), pp. 169-194.
  arXiv: http://arxiv.org/abs/astro-ph/0407044



