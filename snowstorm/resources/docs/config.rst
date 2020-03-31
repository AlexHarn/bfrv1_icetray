.. _snowstorm_configuration:
SnowStorm Configuration
=======================

When producing SnowStorm simulations as described in the :ref:`Snowstorm Simulation chain <snowstorm_introduction>` (step 3), the perturbers can be easily configured by a configuration file.
Supported filetypes are ``json`` and ``yaml``.
Examples of configuration files (``yaml``) can be found in the `SnowSuite <https://code.icecube.wisc.edu/projects/icecube/browser/IceCube/meta-projects/combo/trunk/simprod-scripts/resources/scripts/SnowSuite>`_ script collection.

An overview and details of ice models can be found on `this wikipage <https://wiki.icecube.wisc.edu/index.php/Ice_models>`_. It also lists recommendations on which ice model to use and parameter(ranges) to cover as systematic variations.


Baseline IceModel
-----------------

Before configuring the perturbes, the baseline ice model (e.g. for Absorption and DOMEfficiency scaling) must be set by:

* ``IceModelLocation``
    Location of the baseline ice model to use.

and

* ``HoleIceParametrization``
    Baseline HoleIce to be used. In case a HoleIce perturber is added, it will overwrite the model specified here. However, a parametrization must be specified for the first initialization.


Adding perturbers
-----------------

An overview of all currently implemented perturbers can be found :ref:`here <snowstorm_perturbers>`.

All perturbers that should be used/applied in the simulation can be simply added as subitems to the ``Perturbations`` section of the configuration file.
The name must therefore match the perturber's identifier listed in :ref:`SnowStorm Perturbers <snowstorm_perturbers>`.

For example, adding the perturber for scaling the DOMEfficiency looks like this:

.. code-block:: yaml

    # DOM efficiency scaling
    DOMEfficiency:
        type: uniform
        uniform:
            limits: [[0.9, 1.1]]

The sampling distribution can be individually specified for each perturber by the ``type`` argument followed by the distribution's name.
In the example above, a uniform sampling distribution between 0.9 and 1.1 is specified.

The parameters of the sampling distributions, e.g. the limits of the uniform distribution, are defined as subitems of the distribution's name.


Sampling Distributions
----------------------

The following list contains all valid ``type`` identifiers for the sampling distributions along with their parameter keywords:

* Delta distribution
    .. code-block:: yaml

        type: delta
        delta:
            x0: [*value*]

* Gaussian/Normal distribution
    .. code-block:: yaml

        type: gauss
        gauss:
            mu: [*mu*]
            sigma: [*sigma*]

* Uniform distribution
    .. code-block:: yaml

        type: uniform
        uniform:
            limits: [[*lower_bound*, *upper_bound*]]

    .. note::
        ``limits`` must be a list of tuples (or lists with two items).

In case of a perturber depending on more than one parameter, e.g. HoleIce, multiple values for ``x0`` (delta), ``mu, sigma`` (gauss) or ``limits`` (uniform) must be given:

.. code-block:: yaml

    HoleIce_Unified
    type: gauss
    gauss:
        mu: [*mu0*, *mu1*]
        sigma: [*sigma0*, *sigma1*]

.. note:
    There is currently no option for sampling parameters for one perturber according to two different distributions. 

The above distributions can be used for all perturbers listed in :ref:`SnowStorm Perturbers <snowstorm_perturbers>` except for the IceWavePlusModes which needs different identifiers:

.. code-block:: yaml

    IceWavePlusModes
        apply: true
        type: default

.. note:
    There is currently only a ``default`` perturbation type for the IceWavePlusModes. Applying them can be controlled with the ``apply`` keyword.
