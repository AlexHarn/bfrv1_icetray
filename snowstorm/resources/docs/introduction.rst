.. _snowstorm_introduction:
Introduction
============

.. note::
    This is only a (very) short introduction into the SnowStorm simulation approach focusing on "how to run it".
    Please refer to the `SnowStorm paper <https://arxiv.org/abs/1909.01530>`_ for a detailed description of the method.


SnowStorm Simulation Chain
--------------------------

The overall simulation chain is very similar to the default, "non-SnowStorm" one:

1.  Event (signal) generation (``1-process-Gen.py``)

    1.1 CR background simulation

2.  Merging CR background using polyplopia (``2-Polyplopia.py``)
    To ensure that the CR background is treated in the exact same way as the signal simulation (i.e. with the same, perturbed ice model parameters), it is just merged here.
    All particle propagation, including running PROPOSAL, is done in the next step.

2.  Photon propagation with application of the SnowStorm perturbers (``3-Snowstorm.py``) (includes running PROPOSAL)

3.  Detector simulation

4.  Level1 processing

5.  Level2 processing

Steps 3 to 5 are not different to the "non-SnowStorm" simulation chain.
The scripts for steps 1 to 3 can be found in the `SnowSuite <https://code.icecube.wisc.edu/projects/icecube/browser/IceCube/meta-projects/combo/trunk/simprod-scripts/resources/scripts/SnowSuite>`_ script collection.

Details on how to run ``3-Snowstorm.py`` and how to configure the perturbers (see :ref:`SnowStorm Perturbers <snowstorm_perturbers>` for a list) can be found in :ref:`SnowsStorm Configuration <snowstorm_configuration>`.
