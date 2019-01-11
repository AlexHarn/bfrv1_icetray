.. _filter-tools:

filter-tools Documentation
==========================

**Maintainer:** Erik Blaufuss

.. toctree::
   :maxdepth: 3

   release_notes

Overview
~~~~~~~~
A set of tools used in the online filter.  Also used in 
verifying and simulating the online filters.

The Modules
~~~~~~~~~~~
* :cpp:class:`CreateFilterMask` - A c++ module to create a I3FilterResultMap (aka FilterMask) based on I3Bools located in the Frame
* `FilterMaskMaker <../../doxygen/filter-tools/classpython_1_1FilterMaskMaker_1_1FilterMaskMaker.html>`_ - A python module to creat a FilterResultMap (aka FilterMask) based on I3Bools in the frame with configurable prescales.
* :cpp:class:`DistributePnFObjects` - A module used to help rehydrate the QP frame structure in the compact PFFilt files transfered from pole.
* :cpp:class:`FilterMaskFilter` - An icepick module that select frames based on a online fiter result
* :cpp:class:`FilterCheckModule` - A utility module that is used to compare an I3FilterResultMap against a set of I3Bools
* :cpp:class:`ParticleCheck` - A utility module to compare sets of I3Particles. 
* :cpp:class:`FilterMask2Bools` - Creates a set of I3Bools in the frame based on the contrnts of the  I3FilterResultMap.
* `KeepFromSubstream <../../doxygen/filter-tools/classKeepFromSubstream.htm>`_ - A utility module used in the online filter to flatten the QP strucure for the PFFilt files. Keeps selected objects in Q frame and labels them to match the source split and subevent number.
* :cpp:class:`OrPframeFilterMasks` - A utility module used in the online filter to combine the filter results in I3FilterResultMap entries from multiple P frames.

Also see the `doxygen <../../doxygen/filter-tools/index.html>`_ docs.
