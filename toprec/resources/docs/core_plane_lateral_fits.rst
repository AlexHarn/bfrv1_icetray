Core (or COG) Fit
---------------------
 
I3TopRecoCore calculates a Shower core, using the sqrts of the pulse
amplitude as weights and calculating the COG of these. If no Pulses
are available, it can also be run with Hits, but then no sqrt is
used and the result is worse (no equal distribution between the
tanks).
 
- **datareadout** [string, DEFAULT="ITSimData"] Name of the input Pulses from the data to be used
- **showercore** [string, DEFAULT="ShowerCOG"] Name of the output I3Particle
- **ntanks** [DEFAULT=all of them] Number of highest-charge tanks to use
- **weighting_power** [DEFAULT=0.5] power for the weighting - the weight will be PE^power
- **verbose** [bool, DEFAULT=false] 


Plane Wave Fit
-------------------

I3TopRecoPlane calculates what plane wave fits the shower front best. Therefore it calculates the minimum of this:

.. math:: \chi^2 = \sum_i w_i (t_i^{measured} - t_i^{fit})^2 = \sum_i \frac{(t_i - T_0 + \frac {u\,x_i + v\,y_i}{c})^2}{\sigma^2}
   
The fit parameters are the average time :math:`T_0` and the shower direction, represented by u and v. 
Using the COG of the x and y in that weights (:math:`w_i=1/\sigma^2`), :math:`T_0` decouples from the other
parameters and they all can be calculated
easily from the matrix of their linear system. The :math:`\sigma` is set to 5 ns for all pulses equally as a rough
approximation.

The calculation goes through 2 iterations. In between, a correction
for the different tank heights is applied and a cut for too big
time residuals can be done optionally.

- **datareadout** [string, DEFAULT="ITSimData"] Name of the input Pulses from the data to be used
- **showerplane** [string, DEFAULT="ShowerPlane"] Name of the output I3Particle
- **trigger** [int, DEFAULT=3] Set a software trigger on how many hits should be there. A usual
  value would be 10 (= 3 stations).
- **residualcut** [double, DEFAULT=3000] Time residual (in ns) to the shower plane above which  
  a hit will be cutted as noise. A good value is 100. With 15, you get a perfect Chi2 shape, 
  loosing half of your events. The DEFAULT means "anything within the array" = no cut.
- **threshold** [double, DEFAULT=0] Minimum Amplitude (in mV) to accept as a real pulse. More than 40 is too much.
- **EventHeaderName** [DEFAULT="IceTopEventHeader"] Event header name
- **verbose** [bool, DEFAULT=false]
   
