.. _snowstorm_perturbers:
SnowStorm Perturbers
====================

All SnowStorm perturbers are defined as classes with a single ``transform`` method used when being applied. For details on how to use/apply perturbers in simulation refer to :ref:`SnowsStorm Configuration <snowstorm_configuration>`.

The following list is an overview of all perturbers currently implemented:


Absorption
----------

* Identifier: ``Absorption``

This perturber scales the overall (global) ice absorption coefficient with reference to the baseline ice model.


AnisotropyScale
---------------

* Identifier: ``AnisotropyScale``

This perturber scales the ice anisotropy strength.

.. note:
    It does (and can) **not** change the direction of the anisotropy axis but change the strength of the ice anisotropy only!


DOMEfficiency
-------------

* Identifier: ``AnisotropyScale``

This perturber scales the DOMEfficiency of all DOMs with a factor. The reference DOMEfficiency is taken from the baseline ice model specified in the SnowStorm configuration file.


HoleIce
-------

The `MSU Forward HoleIce wikipage <https://wiki.icecube.wisc.edu/index.php/MSU_Forward_Hole_Ice>`_ for more details on the holeIce model.

For SnowStorm, there are two perturbers for two different HoleIce models/parametrizations:

.. note::
   Both HoleIce models depend on two parameters! Make sure to specify a sampling distribution for both, see :ref:`SnowStorm  Configuration <snowstorm_configuration>` for mote details.


MSU HoleIce
^^^^^^^^^^^

* Identifier: ``HoleIceForward_MSU``

Please refer to the `MSU Forward HoleIce wikipage <https://wiki.icecube.wisc.edu/index.php/MSU_Forward_Hole_Ice>`_ for more details on the MSU HoleIce model.


Unified HoleIce
^^^^^^^^^^^^^^^

* Identifier: ``HoleIceForward_Unified``

More information of the unified angular acceptance model (developed by Philipp Eller) can be found on its `GitHub <https://github.com/philippeller/angular_acceptance>`_ page.


IceWavePlusModes
----------------

The IceWavePlusModes scales the ice absorption and scattering coefficients not globally but depth depend using the ``icewave`` ice model.
To reduce the number of parameters (the ice is split in 10m thick layers, each with independent absorption/scattering), it uses a Fourier analysis.
More details on this ice model and the Fourier decomposition can be found in the `SnowStorm paper <https://arxiv.org/abs/1909.01530>`_.

Scattering
----------

* Identifier: ``Scattering``

This perturber scales the overall (global) ice scattering coefficient with reference to the baseline ice model.
