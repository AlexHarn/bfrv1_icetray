.. _core-removal:

============
Core-Removal
============

.. toctree::
   :maxdepth: 1
   
   release_notes

**Original Author**: Doug Rutledge

This module splits a given pulse-series (``InputRecoPulseSeries``) into
two parts:  The one close around a given vertex (``VertexName``) (it is
called the core pulses and stored in ``CorePulsesName``) and the part
far out (corona pulses stored in ``OutputRecoPulseSeries``).

The pulses of an OM are part of the core pulses if the OM distance
to the vertex position is smaller than a radius :math:`R=k R_{SPE}`, 
where :math:`k` is a scaling factor given by
``SPEFraction`` and the SPE radius :math:`R_{SPE}` (stored as
``SPERadiusName``) is calculated with the formula 
   
.. math::

   R_{SPE}=\left\{\begin{array}{ll}
   \lambda_{Att}\ln E + c_\lambda &\quad E > E_c\\
   R_{min} + c_{spline}\ln^2E &\quad E\le E_c
   \end{array}\right.

where :math:`\lambda_{Att}` (``LambdaAttn``), :math:`C_\lambda`,
:math:`E_c` (``CriticalEnergy``) and :math:`R_{min}`
(``MinimumSPERadius``) are given as parameters. The spline parameter
is calculated as :math:`c_{spline}=\frac{\lambda_{Att}}{2\ln E_c}`
and constant is calculated as
:math:`c_\lambda=R_{min} - \frac{\lambda_{Att}}{2}\ln E_c`.
The energy :math:`E` is taken from the vertex particle or if
``NChCalib`` is set estimated from the numbers of channels with
different algorithms depending on ``DetectorConf`` (0: Amanda, 1:
IceCube, 2: Combined).


See Also
========

.. toctree::
   :maxdepth: 1
	      
   C++ API Reference </doxygen/core-removal//index>
   IceTray Inspect Reference </inspect/core_removal>
