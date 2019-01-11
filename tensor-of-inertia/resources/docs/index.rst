.. _tensor-of-inertia:

Tensor of Inertia
=================

Maintainer : **Alex Olivas**

.. toctree::
   :maxdepth: 1
   
   release_notes

Overview
^^^^^^^^

The tensor of inertia algorithm treats the pulse amplitidues of the PMTs as 
virtual masses, with each PMT The amplitudes of the OMs at position r_i from 
the center of gravity of the hits. 

The amplitude weight w can be set arbitrarily, with 0 and 1 the two most common 
values.  (1 itself is the default value) The smallest eigenvalue  of the inertia 
tensor corresponds to the longest axis, which approximates the track if the 
smallest eigenvalue is much smaller than the other two eigenvalues.
	
Files 
^^^^^

* :cpp:class:`I3TensorOfInertia` - containts the tensor of inertia module which reconstructs a particle with the tensor of Inertia algorithm. 
* :cpp:class:`I3TensorOfInertiaCalculator` containts the neccessary functions for the Inertia Tensor algorithm.

The module I3TensorOfIntertia puts the following two frame objects (found in the
recclasses project) in the frame : I3TensorOfInertiaFitParams and I3TensorOfInertiaTest.

`Code Details <../../doxygen/tensor-of-inertia/index.html>`_
