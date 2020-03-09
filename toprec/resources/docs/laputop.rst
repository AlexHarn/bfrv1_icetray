Laputop (the full reconstruction)
**********************************************

:authors: Katherine Rawlins (krawlins@alaska.edu), Tom Feusels (tom.feusels@ugent.be)

What is Laputop?
====================

Laputop is a maximum-likelihood reconstruction algorithm for IceTop cosmic ray air shower events.
It takes the Core & Plane Wave first guesses as seeds for the track position and direction, respecively,
and makes a four-dimensional (x, y, :math:`S_{125}`, beta), 
fit to the lateral distribution of charges in an event.
The algorithm makes use of the Gulliver framework.

The LDF used is the "Double Logarithmic Parabola" or "DLP":

.. math:: {\rm VEM}(R) = S_{125} \cdot \left(\frac{R}{125\rm m}\right)^{-\beta-\kappa\cdot\log_{10}(R/125{\rm m})}

The two free parameters is this LDF shape are: :math:`S_{125}`, which is the signal per tank in 125m
distance from the core, converted to VEM, and :math:`\beta`, which is a measure of the slope of the LDF,
related to the "age" parameter of the NKG function by:
:math:`{\rm age} = -0.838(6)\beta_{100} + 3.399(15)`,
where :math:`\kappa = 0.30264` seems to be a constant for hadronic showers. 
Internally the logarithms of the charges are used, because the 
probability distribution of these is Gaussian. 
The fluctuations are parametrised by two powerlaws with a knee at about 1.5 VEM.

At the option of the user, Laputop can also perform a combined fit of the charge *and time* 
distributions, if a curvature function is selected in the Fitter.  This is typically
done together with specifying the track direction and vertex time (:math:`\theta`, :math:`\phi`, t) as
additional free parameters in the ParametrizationService. 
The arrival time curvature is described by a function consisting of a parabola with a Gaussian "nose"
with constant parameters for all showers:

.. math:: t(R)/{\rm ns} = C_1 (\exp(-r^2/\sigma^2) - 1) - C_2r^2.



Laputop replaced an older reconstruction code "I3TopLateralFit", which produced
a result commonly named "ShowerCombined" in the frame.
Most of the functionality of I3TopLateralFit is also present in Laputop.
But because of the modular structure of Gulliver, Laputop is designed to
do it in a way which is more flexible to the user, and easier to modify
by developers.  Some improvements to the algorithm were
discovered as part of the process of development.
Details about the original lateral fit can found in the report 
"Klepser, Van Overloop, Kislat: A Lateral Distribution Function and Fluctuation Parametrisation for IceTop" 
(http://internal.icecube.wisc.edu/reports/details.php?type=report&id=icecube%2F200702001).


The name "Laputop" is a combination of a literary reference and a description
of what the code does.  "Laputa" is the name of a floating city in Gulliver's
Travels (appropriate for a code which deals with a detector which floats above 
the rest of IceCube?), and "Top" reminds us that this is an IceTop reconstruction.



User's Guide
================

.. toctree::
   :maxdepth: 2

   casual_users
   advanced_users
   differences
   developers




Links and resources
=========================

These documents trace the history of the development of the project:

* Kath's very first talk about the project on cr-wg call:
  `Docushare 6/29/2011 <https://docushare.icecube.wisc.edu/dsweb/Get/Document-58361/gulliverizing_crwg.pdf>`_
* Kath's second talk about the project on cr-wg call: 
  `Docushare 7/13/2011 <https://docushare.icecube.wisc.edu/dsweb/Get/Document-58453/gulliverizing2_crwg.pdf>`_
* Kath's overview talk at Uppsala pre-meeting:
  `Indico 9/17/2011 
  <https://events.icecube.wisc.edu/indico/materialDisplay.py?contribId=100&amp;sessionId=41&amp;materialId=slides&amp;confId=36>`_
* Tom's talk about the project on cr-wg: 
  `Docushare 1/11/2012 <https://docushare.icecube.wisc.edu/dsweb/Get/Document-59713/Notes-on-Laputop3.pdf>`_
* Tom's second talk about the project on cr-wg:
  `Docushare 1/25/2012 <https://docushare.icecube.wisc.edu/dsweb/Get/Document-59777/LaputopStatus25-01-2012.pdf>`_


More about the Gulliver Framework:

* Link to `wiki page about Gulliver <http://wiki.icecube.wisc.edu/index.php/Gulliver>`_


