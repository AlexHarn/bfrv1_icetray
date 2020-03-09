Snow Correction Services
**********************************************

:authors: Katherine Rawlins (krawlins@alaska.edu)

What does a "SnowCorrectionService" do, and why is it needed?
====================
Snow attenuates air shower particles; the deeper an IceTop tank is buried, the greater the overburden 
and the smaller the expected signal charge in the tank.
Snow accumulates over time, blown in by the wind, making each year (or month, or week) of IceTop
data a little different from the year (or month, or week) before.
It covers the array irregularly, generally deeper over tanks that were deployed earliest, 
are near buildings, or located where the wind over the terrain deposits more.

The job of a SnowCorrectionService is to take an unattenuated *expected* signal at a tank from a hypothesis track, and
to compute the *attenuated expected* signal due to the snow.  
The attenuated expectation is then compared with the real detector data, as part of the likelihood calculation within Laputop.  
NOTE: Laputop does *not* use this service to modify the detector *data* (the Pulse charges), only the 
expected charges they are compared to, as it explores different tracks and LDF's.  
  
Now, if you poke around in a typical L3 IceTop data file, you will find things titled "SuchAndSuchTankPulses_SnowCorrected",
which appear to be Pulses that have been "un-attenuated" and so representing the signal before going through the snow.
However, these pulses are *not* intended to be used in reconstruction; they are there to "guide the eye" in special applications, 
such as with steamshovel.  They seem tempting, but do *not* use them as the input pulses for Laputop!  Laputop performs
snow-correction internally, on the hypothesis charges, not on the data.

Currently in Laputop, there are three SnowCorrectionServices available for use.


The "Simple Lambda" snow correction:
====================

The simplest possible treatment for the effect of snow is that it attenuates signals exponentially, 
according to some attenuation length that is characteristic of the medium:

.. math:: S_{corrected} = S \cdot \exp\left((d/\cos(\theta))/\lambda\right)

...where :math:`d` is the vertical snow depth covering a particular tank, 
:math:`\theta` is the zenith angle of the shower, and :math:`\lambda` is a attenuation length
which is a fixed constant.

The SimpleSnowCorrectionService has one configurable parameter, which is the value of the
constant "Lambda" to apply to all signals in all tanks. 

This is the standard snow correction currently in use in L3 IceTop processing.
Because the IceTop signals get more "muonic" as the snow piles deeper, the best value of "Lambda"
changes from year to year.  It is typically between 2.1 and 2.3 meters for the years 2011-2015;
see the details of L3 processing (in the icetop_Level3_scripts project) to see what we think the
best values are for various years.   


The "BORS" snow correction:
====================

"BORS" ("Based On Ring Simulation") was developed by Katherine Rawlins to be the "ultimate" universal function
for computing the appropriate snow correction regardless of radius, shower energy, or composition.  It was
based on a custom Monte Carlo simulation.

* Link to `wiki page about BORS <http://wiki.icecube.wisc.edu/index.php/BORS>`_

The BORSSnowCorrectionService has one configurable parameter, which is a Boolean "EMonly",
specifying whether the service will be given signals which are electromagnetic-only.  
You'd set this option to "True" from within a reconstruction where the EM and muon components
are being reconstructed separately, and set it to "False" for a more standard Laputop where
there is just one LDF for the combined EM/muon signal.

BORS was put through some initial tests, and did not perform very well.  It is not recommended for use.
 

The "RADE" snow correction:
====================

This "RAdius-DEpendent" snow correction function is currently under development by Katherine Rawlins.
It is based on 5 years worth of burnsample data.  It is still in the testing phase.






