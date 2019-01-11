For the advanced user: all those user options
************************************************

This is for people who want firmer control over the myriad of options and settings available
in Laputop.

Snow Correction Services
--------------------------------

There are two kinds of snow correction service available:

* I3SimpleSnowCorrectionServiceFactory, which has one user parameter: "Lambda" (the attenuation length; default = 2.1).
* I3BORSSnowCorrectionServiceFactory, which has one user parameter: "EMOnly" (a boolean specifying whether the
  service is being given a signal which is only electromagnetic; default = false).

Create one of these (with a name), and then identify it to the Likelihood service using the option "SnowServiceName"
(described in the "LikelihoodService options" section, below).


Minimizer options
-------------------------------

I didn't write this service, so I am ill-prepared to really describe all
the options.  For more information about these, have a look at the documentation for 
the project "lilliput".

SeedService options
--------------------------------

All of these options were copied (basically) from I3TopLateralFit.

* **InCore**: Name of the particle from which to seed the core (default = "ShowerCOG") 
* **InPlane**: Name of the particle from which to seed the direction (default = "ShowerPlane") 
* **InParams**: Name of the I3LaputopParams (or old I3TopLateralFitParams) from which to seed S125 and Beta (default = do not fill from any Params) 
* **InputPulses**: Name of a pulseseries (will only be used to estimate seed S125); this has no default, you *must* enter something
* **Beta**: Seed value of beta for DLP. (default=2.6, although this doesn't really matter if one sets 
  "Step 1 boundaries" for beta as between 2.9 and 3.1... it'll start out at 2.9) 
  Note: if you specify something here and also "InParams", this will override what is in the Params. 


ParametrizationService options
--------------------------------

Most importantly, specify which parameters you want free and which you want to float:

* **FixCore**: Fix the seed Core? (default = False) 
* **FixSize**: Fix S125 and Beta? (default = False) 
* **FixTrackDir**: Fix the seed track direction and time? (default = False) 
* **IsBeta**: Is it Beta? (set to false if it's Age) (default = True).  This one is kind of a holdover 
  from I3TopLateralFit,
  where the "beta" parameter was used to represent age if the user chose the NKG function instead of DLP.  This choice
  is not yet implemented in Laputop, so you can pretty much ignore this option and leave it default. 

Set the initial stepsizes:

* **BetaStepsize**: Stepsize for the beta (or age) (default = 0.6) 
* **SStepsize**: Stepsize for the Log(S125) (default = 1.0) 
* **VertexStepsize**: Stepsize for the core position X and Y (default = 20.0) 

Set maximum/minimum limits:

* **CoreXYLimits**: Maximum/minimum of core position X and Y (default = 2000) 
* **MinBeta**: Minimum Beta (default = 1.5) 
* **MaxBeta**: Maximum Beta (default = 5.0) 
* **MaxLogS125**: Maximum Log10(S125) (default = 6.0) (The minimum log10(S125) is hard-wired to be -3.0 
* **LimitCoreBoxSize**: An alternative to CoreXYLimits: Restrict the core location to a square 
  around *previous core*, 
  of plus or minus this number of meters in X and Y. (default = -1.0, which means use CoreXYLimits instead... 
  set this to any positive number
  and CoreXYLimits will be ignored.) 



LikelihoodService options
--------------------------------

Familiar options brought over from I3TopLateralFit:

* **DataReadout**: Name of the pulseseries to use (default = 'TopEvent_0') 
* **BadStations**: Name of the list of stations that have been removed by I3TopEventBuilder (default = "") 
  NOTE: DEPRECATED, USE BadTanks INSTEAD! 
* **BadTanks**: Name of the list of tanks that have been removed by I3TopEventBuilder (default = "") 
* **LDF**: Desired lateral density function, such as "dlp" (Double Logarithmic Parabola), which at the moment 
  is the only one that's available.  This setting will be overwritten by the Fitter Module if the user asks for 
  different LDF's for multiple steps (see "LDFFunctions" below). (default = 'dlp') 
* **Curvature**: Name of the curvature function you'd like to use for timing likelihood (in addition to charge
  likelihood), such as "gausspar", which at the moment is the only one that's available.  Use an empty string " 
  to *disable* calculation of a timing likelihood.  Like the "LDF" option, this one will be overwritten 
  by the Fitter Module if the users asks for changes between steps (see "CurvFunctions" below).  (default = 'gausspar') 
* **DynamicCoreTreatment**: Radius from the core, within which to perform *dynamic* removal of tanks in LLH calculation 
  (default = 11.0) 
* **SoftwareThreshold**: Apply a software threshold (in VEM) to all pulses before starting likelihood calculation. 
  LC is recalculated. (default = none applied) 
* **Trigger**: Minimum number of stations required for fitting (default = 5).  Using this option should not
  be necessary, as Gulliver will make a go/no-go
  decision automatically, based on the number of data points and the number of degrees of freedom 
  (See "Differences between I3ToplateralFit and Laputop", below.)
  But you can use this option (brought over historically from I3TopLateralFit) to invoke an additional hard cut on the
  number of stations.
* **SaturationLikelihood**: Use likelihood term for saturated signals (default = True) 
* **CorrectEnvironment**: Correct expected signal for pressure (and temp) (default = False, implemented, but will only
  correct S125 based on Bakhtiyar Ruzybayev's pressure correction formula (is only done for the output S125 where it should be.) 
* **MaxIntraStationTimeDiff**: Don't use stations for timing likelihood when 
  the difference in time is larger than this value, all stations used when <0.(default= -1, use all stations).
* **SnowServiceName**: Name of the "snow correction service name" which will supply the attenuation function
  for signals in snow.  (For instance, "SimpleLambda" or "BORS")


Fitter options
--------------------------------

Specify the number of steps and the names of the services you'd like to call:

* **NSteps**: Number of steps (from 1 to 4) (default = 1) 
* **Minimizer**: Name of minimizer service to use (no default; you must specify something) 
* **SeedService**: Name of seed service (no default; you must specify something) 
* **Parametrization1**: Parametrization service (1st step) (no default; you must specify something) 
* **Parametrization2**: Parametrization service (2nd step) (default = ", used only if NSteps :math:`\ge` 2) 
* **Parametrization3**: Parametrization service (3rd step) (default = ", used only if NSteps :math:`\ge` 3) 
* **Parametrization4**: Parametrization service (4th step) (default = ", used only if NSteps == 4) 
* **LogLikelihoodService**: LogLikelihood service (no default; you must specify something) 

Specify *lists* of LDF's, timing functions, and cut choices for your N steps. 
If specified, the length of these lists *must*
match the number of steps from "NSteps", or the code will exit with an error.

* **LDFFunctions**: List of LDF functions for steps (default = []) 
  Currently available: "dlp"
* **CurvFunctions**: List of Curvature functions for steps (default = [])
  Currently available: "gausspar", or an empty string for no curvature/timing fitting 
* **StaticCoreCutRadii**: List of radii at which to do a static (permanent) cut at the start 
  of each step (default = none). 
  Note: due to the structure of Gulliver, this cut cannot be applied to the
  first step, and so the *first* entry of this list will be ignored.
* **StaticTimeCutResiduals**: List of time residuals beyond which to do a static (permanent) 
  cut at the start of each step (default = none). 
  Note: due to the structure of Gulliver, this cut cannot be applied to the
  first step, and so the *first* entry of this list will be ignored.

Other options:

* **IcePickServiceKey**: Key for an IcePick in the context that this module should check before 
  processing physics frames. (default = none) 
* **If**: A python function... if this returns something that evaluates to True, Module runs, 
  else it doesn't (default = None) 
* **StoragePolicy**: Which I3Particles would you like to output in the frame?  This section of the
  code was borrowed from I3SimpleFitter, which was written to deal with multiple seeds.  There are many
  options allowed, but the only ones which have been tested in Laputop are:

  - "OnlyBestFit" (default) only a single (last) result
  - "Intermediate": store all the results of the intermediate steps (they will be labeled "_1", "_2", etc.




