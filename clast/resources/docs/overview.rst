.. _overview:

Overview
========

CLast contains `I3CLastModule`. It is a module that calculates the Tensor of Inertia (of light/charge rather than mass) about the Centre of Gravity to:

* Determine a first guess at the casacade direction and provides a fast and analytic seed for further cascade reconstructions.

* Determine the ratio of the smallest ToI eigenvalue to the total of all 3 eigenvalues; a value of 0.333... is maximal and implies the distribution of light is spherical, values less than this imply less and less spherical distributions of light.

See :ref:`calculations` for information on the specific calculations performed. See :ref:`usage` for scripts to use the `I3CLastModule`.
