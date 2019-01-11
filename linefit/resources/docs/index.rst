.. _linefit:

=======
LineFit
=======

Overview
========
Line Fit is a first guess algorithm, that uses an analytical algorithm 
for a fast track reconstruction.

Model
=====
The linefit reconstruction ignores the geometry of the Cherenekov cone and 
the optical properties of the ice, and assumes photons are travelling on a wavefront perpendicular to the track with velocity V along a 1-dimensional path.

Using the OM locations :math:`r_i`  and hit times :math:`t_i`, two free parameters :math:`\vec{v}` (velocity) and :math:`\vec{r}` (vertex point) 
are  defined so:

.. math::

 r_i=\vec{r} + \vec{v}* t_i

is minimized. The minimization is done by differentiation with respect to the free parameters.

The analytical solution is: 

.. math::

 \vec{r}=<r_i>-V<t_i>
 V=\frac{<r_i*t_i>-<r_i>*<t_i>}{<t_i^2>-<t_i>^2}

Where :math:`<x_i>` is the amplitude weighted average on :math:`x`. 

The line fit yields a vertex point :math:`\vec{r}` and a direction,
The direction is obtained using:

.. math::
  \hat{e}=\frac{\vec{v}}{|v|}

where

.. math::

 \theta=\arccos(-\frac{v_z}{|v|})\quad\phi=\arctan(\frac{v_y}{v_x})

Algorithm
==========

- Obtain selected track info (using ``InputSelection`` and ``InputReadout``).
- Count number of hits N. Continue only if N > ``MinHits``.
- For each hit get hit position :math:`r_i=(x_i,y_i,z_i)`, leading edge time :math:`t_i`, and amplitude :math:`A_i`.
- Calculate amplification :math:`Amp_i=(A_i)^{AmpWeightPower}`
- Sum time, :math:`time^2`, coordinates, and time*coordinates over all hits using:

  .. math::

     <x>=\frac{1}{N}\sum{Amp_i*x_i}
     
- Comment: the average on t^2, tx,ty,tz was weighted by amp. Not by amp^2.
- Calculate velocity, vertex and angles using the above formulas
- Save results name **Name**

Input Variables
===============
- **Name** [DEFAULT="LineFit"]: Name to give the fit results the module adds to the event.  
- **MinHits** [DEFAULT=2] : Minimum number of hits. Event with fewer hits will not be reconstructed. If  MinHits is smaller than 2, the default value "2" will be used.
- **AmpWeighPower** [DEFAULT=0]: Hits are weighted with the amplitude raised to this power.<br> Usually =0 so hits_weight=1, or =1 so hits_weight=amplitude. 
- **LeadingEdge** [DEFAULT=ALL]: Determine what hits to use: options: ALL = all hits (FirstLeadaingEdge and the rest). Otherwise will only use FLE.
- **InputRecoHits** [DEFAULT=""]: RecoHits to use for input
- **InputRecoPulses** [DEFAULT=""]: RecoPulses to use for input

Examples and Tests
==================

To run tests execute the scripts in resources/scripts. 
The following tests will be performed:

- general - a General test, reads events and do a line fit reconstructions with the default values to all parameters. The following variables are checked: Zenith and Azimuth values between 0..2*pi, :math:`V^2=v_x^2+v_y^2,v_z^2`.
- MinHit - Same as test "general". but with MinHits set to 99999999 (so no event will be reconstructed).
- recoos - Comparison between I3LineFit and recoos' linefit. EventWeight is set to 0.0 and hitselection is the same.
- AmpWeightPower - Same as test "general". AmpWeightPower is changed. 



ImprovedLinefit
===============

Improved linefit is a robust first-pass reconstruction algorithm intended as an
improvement to linefit.  As a starting point, the algorithm expects a hit series
that has already undergone the standard hit cleaning modules.  From the starting
hit series, the algorithm generates a new hit series by removing all hits that
are consider "late" hits.  A hit is considered late if it is time stamped more
than "t" nanoseconds later than any other hits within a neighborhood of radius
"d".  This process is the "delay cleaning" of the hit series.

After the delay cleaning, the algorithm then computes the Huberfit on the
remaining data points.  To understand the Huberfit, we need to define a residual
between a track and a data point.  A residual for data point i is defined as

.. math::
   
  p_i= ||v(t_i - t_0) + r_0 -r_i||

where :math:`t_i` and :math:`r_i` are the time and position of the data points, and v is the
velocity of the track and :math:`t_0` and :math:`r_0` are the time and position of the track's
vertex.

The Huberfit finds the values of :math:`v` and :math:`r_0` that minimizes the equation::

  sum( phi(p_i) )
  where phi(p) = p^2 if p<= mu
                 mu*(2*p -mu ) if p > mu

and mu is some constant calibrated to the data.  

After the Huberfit is calculated, we then "debias" the data.  This final step of
the algorithm takes the Huberfit reconstruction as a seed, and discards all data
points with residuals greater than rho, where rho is a constant calibrated to
the data.  The idea is that points far from the track are going to be noise, so
there discarding them will enable better reconstructions.  After the points are
discarded, the algorithm then runs linefit on the new data set to get the final
track reconstruction.  

Usage
^^^^^

To run this algorithm, you will want to use 4 modules (or the single tray segment).  

1.	Run the delay cleaning on the input hitseries.  This hitseries should have already been cleaned by the usual noise reduction algorithms.

2.	Run the Huberfit on the output of the delay cleaning.

3.	Run the debiasing on the output of the Huberfit.  

4.	Run the standard linefit on the output of the debiasing.  

This algorithm is broken up into 3 modules (plus linefit) 

Arguments (DelayCleaning)
^^^^^^^^^^^^^^^^^^^^^^^^^

1.  InputResponse (default: "NFEMergedPulses")
	The name of the starting hit series.  It's important to note that this module is expecting the starting hit series to already have gone though the time-window and RT cleaning.   
	
2.	Distance (default: 156*I3Units::m)
	The constant "d" that defines the neighborhood.
				 
3.	TimeWindow (default: 778*I3Units::ns)
	The constant "t" that defines the allowed "lateness" of a hit.  

4.	OutputResponse (default: "delay_cleaned_pulses)
	The name of the new delay cleaned hit series.  
	
Arguments (HuberFit)
^^^^^^^^^^^^^^^^^^^^

1.  InputResponse (default: "Pulses_delay_cleaned")
	The name of the starting hit series.  
	
2.	Distance (default: 153*I3Units::m)
	The constant "mu" used in the minimization.  
	
3.	Name (default: "HuberFit"+ distance)
	The name of the new Huberfit reconstruction.  
	

Arguments (Debiasing)
^^^^^^^^^^^^^^^^^^^^^

1.  InputResponse (default: "Pulses_delay_cleaned")
	The name of the starting hit series.  
	
2.	Distance (default: 116*I3Units::m)
	The maximum allowed residual before a point is discarded.  
				 
3.	Name (default: "linefit")
	The name of the seed used to debias the data.  

4.	OutputResponse (default: "debiasedPulses")
	The name of the new debiased series.  
	
See Also
========

.. toctree::
   :maxdepth: 1

   release_notes
   Python API Reference </python/icecube.linefit>
   C++ API Reference </doxygen/linefit/index>
   IceTray Inspect Reference </inspect/linefit>

