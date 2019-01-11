.. _STTools_framework:

The ST programming framework
============================

The STTools project provides a programming framework for developing IceCube hit
selection algorithms based on space (S) and time (T) hit information.
The framework is implemented as C++ template classes and functions. It provides
general IceCube ST functionalities like an optimized DOM pair map class and
abstract IceCube data information extraction functionalities like getting the
time and the charge from a varity of hit types, for instance I3RecoPulse
objects.

.. toctree::
   :maxdepth: 3

   st_configuration_scheme
   I3STModule
