What does this code do differently from I3TopLateralFit?
************************************************************

I tried to implement all of I3TopLateralFit's features and functionality into Laputop, but
this was not always possible.  There are a few ways in which Laputop functions differently, 
and a few ways in which Laputop was *chosen* to be run differently in order to improve
the reconstruction.

Handling of the "11-meter core cut"
------------------------------------------

Tanks that are very close to the core can screw up the reconstruction, because a) they are
sometimes saturated, and b) the Double Logarithmic Paraboloid LDF "blows up" at near distances
and is might not represent the expected charge very well.  

In I3TopLateralFit, this was dealt with by cutting out (from the reconstruction) any tanks
closer than 11 meters to the reconstructed core.  This cut was performed in between the first
and second of four steps.  The first step would reconstruct the core using *all* tanks.
Then any tanks within 11 meters of this result would be removed, and the reconstruction would be
repeated in a second step.  After this second step, the code would check whether any new tanks 
(not already cut) were within 11 meters of the new core; if so, they would be removed as well, 
and the second step would be repeated.
The second step would iterate like this until no new tanks were being cut out, or until
too many were being cut out such as drop below a threshold for reconstruction.  (In practice, 
the second step requires iteration in a small handful of events; most of the time, 
it is done only once and the code will move right along to the next step.)
Once removed, a tank could not be reinstated; it was a "static" cut.

In Laputop, the 11-meter "cut" is performed dynamically (and it is not really a "cut" at all).
As the algorithm computes a likelihood for each tank in each iteration of the fit, it checks 
to see whether the tank is within 11 meters of the current hypothesis core.  If it is, the
likelihood is calculated using *11 meters* as the distance to the hypothesis track, rather than the
actual (smaller) distance.  In effect, it holds the likelihood of a tank *constant* for any 
hypothesis cores within 11 meters.  If, as the minimizer moves the core around,
the likelihood of a particular tank does not change, then it becomes *not a factor* for determining
the best fit track, effectively eliminating it from consideration in the fit.  

However, since this is done dynamically (for every hypothesis) rather than statically, if the
minimizer moves the core in such a way as the tank becomes outside 11 meters again, its 
likelihood will return to being computed normally.  There is no abrupt difference in likelihood
between when a tank is "included" and when it is "excluded" in this way, so there are no pile-ups of
events near this point of transition.

Want to change 11 meters to some other number?
I3TopLateralFit users controlled the distance with the user parameter "CoreCut" whose default
is 11.0.  Laputop users can control the "dynamic" core treatment similarly with the user parameter
"DynamicCoreTreatment" which is part of the LikelihoodService and also has a default of 11.0.
I think the dynamic version is better, but if you prefer to make a static core-radius cut for some reason, 
that option is available too: the option "StaticCoreCutRadii" in the Fitter module allows you
to input a *list* of distances at which to permanently remove tanks before each of the steps
(the default is to perform none).

Number of steps
--------------------------------

I3TopLateralFit used four steps.  The first and second steps were basically the same, but before
and after removal of tanks using the core cut.  If tanks are being "excluded" dynamically 
within the likelihood function, there is no reason to perform these steps separately; 
they are effectively a single step.  Thus, Tom Feusels' "best settings" configuration of Laputop
requires only three steps, and not four.  (As the user, of course, you can ask Laputop to run
any number of steps between one and four.)

Also, in Laputop, the minimizer stops when the fit doesn't converge or 
finish successfully at the end of one of the steps.  
(I3TopLateralFit will proceed with all steps no matter what.)

Handling of stepsizes and boundaries
------------------------------------------

In I3TopLateralFit, the stepsizes from each step are passed on to the next step.  In other words, as
the minimizer "zeros in" on the answer in one step, the stepsize is reduced, and subsequent steps
*begin* with smaller initial stepsizes.  In addition, in the third step of I3TopLateralFit, the
core is restricted to "within 3 times the stepsize" of the core from the previous step.

Unfortunately, there is no graceful way to do this same thing in Laputop.  Gulliver does not "save" the small
stepsizes from the end of one minimization and put them anywhere where they can be retrieved later for use 
in another.
Instead, the initial stepsizes for each step can be set individually by the user, in the ParametrizationService.
(Recommended values in this document are intended to "mimic" the behavior of I3TopLateralFit for most
events.)  

In addition, the "restrict the core position" technique must be done differently in Laputop.  
Rather than restricting the core position to within a variable number of meters (depending on the stepsize),
the user can specify the dimensions of a "box" of X and Y within which the core shall be bound,
with the "LimitCoreBoxSize" option in the ParametrizationService.  Setting this to "15.0" for instance,
will constrain the core to within a 30x30 box centered on the previous core.

Deciding whether there are enough hits for a fit or not
-------------------------------------------------------------

In I3TopLateralFit, the code proceeds with a fit whenever the number of stations is at least 5, and this
number can be controlled with the user parameter "Trigger".  (For instance, if fixing the track
direction, one could reduce this to 3, because there are fewer degrees of freedom and thus fewer data 
points necessary to make a fit.)

The Gulliver Framework is set up to automatically make this "go/no-go" decision for you, based on the
number of data points available and the number of degrees of freedom.  It determines the number of 
degrees of freedom from the ParametrizationService, and it determines the number of data points from
the LikelihoodService.  

Determining the "number of data points available" is not a straightforward question.  
In Laputop, this number (which in Gulliver is a variable called ``multiplicity_``),
is determined using the following treatment:
A hit tank supplies both charge data and timing data (unless it is saturated, in which case it supplies only timing data).
So multiplicity = (number of tanks hit but not saturated) + (number of tanks hit, IF the likelihood includes timing).
If this exceeds the number of degrees of freedom, Gulliver will proceed with the fit.

In addition, the user can still invoke the "Trigger" user parameter in Laputop
to set a hard cutoff on the number of stations,
if he or she so chooses.  The default is still 5 stations.

Addition of the saturation likelihood
------------------------------------------

Part of the justification of the "11-meter" cut is to avoid problems introduced by saturated tanks.
If a tank (actually the PMT in the tank) is saturated, it will see less light than expected and will not follow the standard Gaussian likelihood
around the expected value anymore. As the saturated tanks still carry useful information and could still
be used for a better core and angular reconstruction, a saturation likelihood is implemented and can
be used optionally (setting ```SaturationLikelihood`` in the Likelihood Service, whose default is "True").

A saturated tank is one in which Low Gain signals are saturated, meaning their charge is higher 90000 PE 
(converted to VEM to be tank specific). In the future this value should be determined for each LG DOM.
The current value is chosen based on : http://wiki.icecube.wisc.edu/index.php/File:PeakAmplitude_charge_5948_LG.png)
Sometimes during the tank pulse building phase (choosing the HG or LG signal in a tank, done in topeventcleaning)
a saturated HG signal is found, without an LG partner. Topeventcleaning assigns a NaN charge to these DOMs,
with their proper LE time. Laputop recognizes them as saturated and assigns the HGLG crossover value
as estimation for their saturated value.

For saturated signals, the timing likelihood is calculated identically as for unsaturated signals. The
charge likelihood is calculated in a similar way as the PnoHit likelihood. The probability for seeing
the measured signal is the integral from the saturated signal to infinity of the Gaussian likelihood
around the expected signal. This gives a simple Erf function which is added to the likelihood.

ToDo : Add the formula... in latex, in .rst.


Different minimizers
--------------------------------

TMinuit is used within I3TopLateralFit code to perform the minimization, with default settings or hard-coded
settings.  With Laputop, everything about the minimizer is partitioned off into the MinimzerService,
and there are many options available.  Lilliput's "I3GulliverMinuitFactor", when set to use the 
SIMPLEX algorithm, appears to perform better (fewer "wild" minimizations) than I3TopLateralFit, 
which (I think) uses the default MIGRAD.  

The output "energy"
--------------------------------

After fitting S125, I3TopLateralFit also made a number of energy estimations (E_50, E_100, etc.), which are
stored in the I3TopLateralFitParams.  However, these energy estimators are untrustworthy, and the code
which calculates them was not ported into Laputop.  "E_proton" and "E_iron", which were filled according
to Bakhtiyar's zenith-dependent fits to protons and iron, have also been removed from Param storage, 
because there is no "standard" way to translate from S125 to energy.  I3TopLateralFitParams has been
replaced by I3LaputopParams.


The z position of the core
-------------------------------
In I3TopLateralFit the z position from the core is taken from the seed but then recalculated as the weighted
mean z position over all hit tanks. As the best COG only uses 7 tanks, the z position changes slightly
(by a meter or two at the most).
Laputop takes the z position from the seed and does *not* change it during fitting.
Since Laputop calculates the I3Particle's time to be associated with this (slightly different) z position, 
this is simply a different choice of "reference elevation" (which is arbitrary), and should not 
make a substantive difference in analysis.  

The position of a not-hit station
-------------------------------------
In I3TopLateralFit, the position (x,y,z) of a "not-hit" station was taken from DOM 61 of that station.
In Laputop, it is taken from the average of the positions of the two tanks comprising the station.
