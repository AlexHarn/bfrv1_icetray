.. 
.. $Id$
.. 
.. @version $Revision$
.. @date $LastChangedDate$
.. @author T. Waldenmaier <tilo.waldenmaier@desy.de> $LastChangedBy$

.. highlight:: python

.. _topsimulator:

topsimulator
============

This project is maintained by Javier Gonzalez <javierg@udel.edu>

.. toctree::
   :maxdepth: 3
   
   release_notes

Overview
~~~~~~~~

TopSimulator is an icetray framework to simulate the IceTop detector
response. It relies on two services: the injector service and the response
service, which define the way particles are injected into the IceTop array and
how the detector response is simulated. The I3TopSimulator module
passes the particles from the injector to the tank response and
writes the resulting I3MCPEs to the frame.

I3TopSimulator does not know the details of which injector or response services
are being used. The rationale for encapsulating both the injector
and the response inside a single module is that it is not uncommon for CORSIKA
files to be very large, so writing the whole particle tree to the frame is not
feasible.

The I3TopSimulator comes with two Injector services
(I3CorsikaInjector and I3ParticleInjector) and one response service
(I3ParamTankRespone). It is up to the user to write additional injector or
response services.

Topsimulator has an option to keep track of the origin of the particles crossing
the tank down to each photoelectron (the option is called CompressPEs). If
CompressPEs==2, this information is discarded altogether. If CompressPEs==0, then
each photoelectron will point to the particle it produced it. If CompressPEs==1,
topsimulator uses the minor ID of the I3MCPE to classify the photoelectrons into
categories (called 'components'). The exact components used are determined by the
particle injector and are stored in a container in the frame.

Two example scripts are provided in `resources/examples`.

* `sim-single-particles.py` demonstrates the simulation of the tank response to user-defined particle hits.

* `sim-icetop.py` demonstrate the simulation of a CORSIKA shower.

Note: The simulation segments used by both are consistent, but may differ from the *official* ones in simprod-scripts, used in mass-production.

The Modules
~~~~~~~~~~~

* :cpp:class:`I3TopSimulator` - The main simulation module.

  Different injectors and tank responses can be configured. An energy cut on muons can be applied. To keep memory consumption reasonable, photo-electrons (PEs) within a configurable time window (usually 1 ns) are grouped together. By default, the grouping is done over all source particles, so that the connection of the PEs to their source particles is lost. There is an option to prevent this, by grouping less aggressively.

* :cpp:class:`I3TopAddComponentWaveforms` - A module to add component waveforms to the frame.

  The module takes the air shower components, the waveform list map of calibrated waveforms (not launches!) and the PEHitSeries from the frame, and adds one waveform list map for each air shower component. These waveforms are nothing more than a histogram of PE times with range and binning matching that of the input waveforms.

Segments
~~~~~~~~

* `Simulation segments <../../doxygen/topsimulator/namespacepython_1_1segments.html>`_

Injectors
~~~~~~~~~

* :cpp:class:`I3CorsikaInjector`

* :cpp:class:`I3CorsikaGridInjector`

* :cpp:class:`I3ParticleInjector`

TankResponses
~~~~~~~~~~~~~

* :cpp:class:`I3ParamTankResponse`

* G4TankResponse, see g4-tankresponse project.

Utility Classes and Namespaces
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

* :cpp:class:`ExtendedI3Particle`

* :cpp:class:`HitHisto`

* :cpp:class:`HitHistoCollection`

* :cpp:namespace:`GeoFunctions`

* `SparseHistogram <../../doxygen/topsimulator/classtopsim_1_1SparseHistogram.html>`_

* :cpp:class:`I3CorsikaReader`

* `vem_yield <../../doxygen/topsimulator/namespacevem__yield.html>`_
