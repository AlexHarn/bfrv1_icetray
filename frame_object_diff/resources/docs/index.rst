.. _frame_object_diff:

Frame Object Diff
=================

Tools for making a "diff" of frame objects.

The design of this project is to compress frame objects that rarely change
between different files.  The GCD files are the primary examples,
with nearly unchanging Geometry, Calibration, and Detector Status frames.

.. toctree::
   :maxdepth: 1
   
   release_notes

Rationale
---------

.. toctree::
   :maxdepth: 2
   
   motivations
   reasoning
   use_cases

Python Objects
--------------

.. toctree::
   :maxdepth: 2

   segments
   modules
   classes

Code Reviews
------------

.. toctree::
   :maxdepth: 1
   
   code_reviews/cweaver

Also see the `doxygen <../../doxygen/frame_object_diff/index.html>`_ docs.
