.. _STTools_algorithms_seededRT:

seededRT
========

The seededRT algorithm provides a radius and time (RT) based hit cleaning based
on initial seed hits. The algorithm is based on the classic *RT-cleaning*
already used in AMANDA
and implemented in the *DomTools* project as *I3IsolatedHitsCutModule*.
The seededRT algorithm implemented by STTools is a code abstraction (with some
more but not less features) of the *SeededRTCleaning* project and its provided
icetray modules (see also :ref:`STTools_algorithms_seededRT_description`).

.. toctree::
   :maxdepth: 3

   description
   configuration_services
   icetray_modules
   migration
   scripting
