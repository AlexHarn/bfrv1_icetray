.. _STTools_algorithms_seededRT_description:

Description of the seededRT algorithm
=====================================

The old *RT-Cut* cleaned out all hits that had no other hit within a
time-window T and a distance radius R.

Instead of applying the RT criterium to all hits and keeping those fulfilling
it, the seededRT algorithm takes another approach by not necessarily starting
out with all hits, but with a subset of *seeds* which are considered to be
mostly signal related hits. These seeds can be provided for instance using HLC
information. By adding all further hits found within the seeds RT-range to the
list of seed hits and iterating until a convergence, only those (SLC) hits are
kept which cluster around the initial seed hits. Outlying noise hits are
supposedly not added and thus removed in the cleaned output.

A more detailed description of the algorithm can be found on the IceCube
`wiki`_.

.. _wiki: https://wiki.icecube.wisc.edu/index.php/SLC_hit_cleaning
