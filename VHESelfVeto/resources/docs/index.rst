.. The Icecube Collaboration
..
.. $Id$
..
.. @version $Revision$
.. @date $LastChangedDate$
.. @author Nathan Whitehorn <nwhitehorn@physics.wisc.edu> $LastChangedBy$

.. _VHESelfVeto:

VHESelfVeto
===========

The VHESelfVeto (Very High Energy Self Veto) is a set of modules for implementing a simple detector edge veto to identify in-detector starting events at high energy (10 TeV and up). The analysis, along with the operation of these modules,
is explained in more detail at on the `IC79/IC86 starting track wiki page <https://wiki.icecube.wisc.edu/index.php/High-Energy_Starting_Track_Event_Search>`_, which will eventually be moved into this documentation.

The main module is VHESelfVeto, which operates on a given pulse series and is self-configuring with respect to geometry. Given an input pulse series, it writes three objects to the frame: the time at which the light in the detector crosses the configured detection threshold, the charge-weighted average position of the pulses contributing to that threshold crossing, and an I3Bool which is true if the event can be vetoed and false if it cannot be (i.e. false if the event appears to be starting). The time and position can be used to seed reconstructions if desired.

The remaining modules (DetectorShrinker, FiducialVolumeEntryPointFinder, HomogenizedQTot, TauGeneratesMuon, VertexInFiducialVolume) are used for working with calibration and passing rate estimation for the passing-event cut implemented by VHESelfVeto. HomogenizedQTot calculates a "homogenized" total charge (i.e. neglecting Deep Core). DetectorShrinker is used to provide a reduced I3Geometry on which to run VHESelfVeto. By nesting veto regions, the false passing rate of the VHESelfVeto cut `can be estimated from data <https://wiki.icecube.wisc.edu/index.php/High-Energy_Starting_Track_Event_Search#Atmospheric_Muon_Background>`_. The remaining modules are utilities for working with simulation, mostly to determine which simulated events are in fact starting.

.. toctree::
   :maxdepth: 1
   
   release_notes
