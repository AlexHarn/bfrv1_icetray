.. _simclasses:

simclasses
==========

**Maintainer:** Alex Olivas - Though this is more of a community project.

This project is only meant to contain classes that inherit from I3FrameObject, or object that are stored in containers that inherit from I3FrameObject (e.g. I3Map or I3Vector).  The idea being that you should be able to include this in any other meta-project and be able to read data produced by simulation.  This keeps the dependencies minimal.

* :cpp:class:`I3CorsikaShowerInfo`
* :cpp:class:`I3MCNKGPoint`
* :cpp:class:`I3MCNKGInterpolation` 
* :cpp:class:`CorsikaLongStep`
* :cpp:class:`I3MCPE`
* :cpp:class:`I3WimpParams` 
* :cpp:class:`I3MCPulse`
* :cpp:class:`I3MCPMTResponse` - Retired in IceSim4.  Needs to be kept around to read old data.

Release Notes
^^^^^^^^^^^^^
.. toctree::
   :maxdepth: 3
   
   release_notes
