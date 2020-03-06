.. _rpdf-usage:

=====
Usage
=====

IceTray Service
===============

The intended usage of rpdf is to provide the likelihood service for a
:ref:`Gulliver` reconstruction. As shown in the following example:
     
.. code:: python

  tray.AddService("I3RecoLLHFactory", "spe_likelihood",
     InputReadout="InIcePulses",
     JitterTime=15*I3Units.ns,
     NoiseProbability=10.*I3Units.hertz
  )

  tray.Add("I3SimpleFitter",
    OutputName="SPESingleFit",
    SeedService="seed",
    Parametrization="simpletrack",
    LogLikelihood="spe_likelhood",
    Minimizer="minuit",
  )

In the example an instance of :cpp:class:`I3RecoLLH` with the name
``"spe_likelihood"`` is placed in the tray's
context by :js:data:`I3RecoLLHFactory`. This example reads an
:cpp:class:`I3RecoPulsesSeriesMap` from the frame and uses a jitter time of
15 ns and a noise rate of 10 Hz. A Gulliver reconstruction
:js:data:`I3SimpleFitter` is then added to the tray and passed the name of the
rpdf service to the ``LogLikelihood`` parameter.



Python
======

The low level functions are also available via a python interface.
An example calculation is below:

.. code-block:: python

  from icecube import rpdf

  icemodel = rpdf.H2
  peprob = rpdf.FastConvolutedPandel()
  jitter = 15*I3Units.ns
  noise =  10*I3Units.hz
  llhtot = 0

  for omkey, pulseseries in self.pulses:
  
    #find the position of the OM
    ompos = geometry[omkey].position
    
    #calculate the geometrical parameters tgeo and deff 
    geo_params = rdf.muon_geometry(ompos,part,icemodel)

    #residual time is the pulse time minus tgeo 
    t_res= pulseseries[0].time-geo_params.first

    #calculate the convoluted Pandel function of t_res 
    llh = peprob.pdf(t_res,geo_params.second,jitter,icemodel)

    #add noise and sum the log of the likelihoods
    llhtot += math.log(self.noise + llh)

  print llhtot

This is equivalent to calculating the SPE1st likelihood in ``I3RecoLLH``.

A easier method if you want to calculate the likelihood of a single event
hypothesis is to instantiate a ``I3RecoLLH`` object in python land:

.. code-block:: python

  recollh = rpdf.I3RecoLLH("SRTHVInIcePulses","SPE1st","GaussConvoluted",
                           15*I3Units.ns,10.*I3Units.hertz,rpdf.H2)

  particle = phys["SPEFitSingle_TWHV"]
  hypothesis = gulliver.I3EventHypothesis(particle)
  recollh.set_geometry(i3geometry)
  recollh.set_event(physics_frame)


  
