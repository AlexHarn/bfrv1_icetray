.. _tensor-of-inertia:

Tensor of Inertia
=================

Maintainer : **Alex Olivas**

.. toctree::
   :maxdepth: 1
   
   release_notes

Overview
^^^^^^^^
This is an IceTray module to implement the Inertia tensor fit first guess algorithm.
It works by calculating a Tensor of Inertia from the hit optical modules in the event,
using the amplitudes as virtual masses. From the tensor the eigenvales and eigenvectors
can be calculated, the smallest eigenvalue corresponds to the longest axis, which approximates the direction of the track.
The tensor of inertia algorithm treats the pulse amplitidues of the PMTs as 
virtual masses, with each PMT The amplitudes of the OMs at position :math:`r_i` from 
the center of gravity of the hits. The center of gravity of the virtual mass distribution is given by:

:math:`\vec{COG} = \sum_{i=1}^{N} {a}_{i}^{w} * \vec{{r}_{i}}`

where :math:`a_i` is the ith PMT amplitude. The Inertia Tensor itself is given by the usual formula with PMT amplitudes replacing the masses:

:math:`{I}^{k,l} = \sum_{i=1}^{N} {a}_{i}^{w} * (\delta^{kl} * \vec{{r}_{i}}^2 - {r}_{i}^{k} * {r}_{i}^{j})`
      
The amplitude weight w can be set arbitrarily, with 0 and 1 being the two most common 
values.  (1 itself is the default value) The smallest eigenvalue  of the inertia 
tensor corresponds to the longest axis, which approximates the track if the 
smallest eigenvalue is much smaller than the other two eigenvalues.
	
Files 
^^^^^

* :cpp:class:`I3TensorOfInertia` - contains the tensor of inertia module which reconstructs a particle with the tensor of Inertia algorithm. 
* :cpp:class:`I3TensorOfInertiaCalculator` contains the necessary functions for the Inertia Tensor algorithm.

The module I3TensorOfIntertia puts the following two frame objects (found in the
recclasses project) in the frame : I3TensorOfInertiaFitParams and I3TensorOfInertiaTest.


Input Variables
^^^^^^^^^^^^^^^

* Name - [DEFAULT='ti'] Name given to the fit the module adds to the event
* InputSelection - OMResponse selector to use for input
* InputReadout - [DEFAULT=RecoPulses] Data Readout to use for input
* MinHits - [DEFAULT=3] Minimum number of hits needed for reconstruction
* AmplitudeWeight - [DEFAULT=1.0] Weight applied to virtual masses in the reconstruction 

`Code Details <../../doxygen/tensor-of-inertia/index.html>`_
