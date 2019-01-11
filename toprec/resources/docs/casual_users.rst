For the casual user
************************

Where is the code, and how do I compile it?
============================================

Laputop is part of the "toprec" project, and so you can check out this project from SVN using
something like::

   svn co $SVN/projects/toprec/trunk toprec

Or, it will come automatically with the "icerec" meta-project.  Compile it normally as part of icetray.

How do I run it?
============================================

Laputop requires a few *services*, which work together with one fitter
*module*.  A python script which runs Laputop will include things that look like this::

    tray.AddService("I3SimpleSnowCorrectionServiceFactory","SimpleLambda21")(
        ("Lambda", 2.1)
        )

    tray.AddService("I3GulliverMinuitFactory","Minuit")(
      << minimizer options here >>
      ) 

    tray.AddService("I3LaputopSeedServiceFactory","ToprecSeed")(
       << seed-related options here >>
      )

    tray.AddService("I3LaputopParametrizationServiceFactory","ToprecParam2")(
       << options related to free parameters here >>
      )

    tray.AddService("I3LaputopLikelihoodServiceFactory","ToprecLike2")(
       << likelihood-function options here >>
      )

    ## The fitter module
    tray.AddModule("I3LaputopFitter","Gulliverized")(
       << options specifying which services to call, and how many , etc. >>
      )


The fitter module at the end (I3LaputopFitter) can perform a reconstruction
consisting of multiple *steps* in succession.  
Each "step" is a minimization of a likelihood function to find a best-fit
I3Particle.
The seed service is used to construct a seed I3Particle for
the first step, and the output I3Particle from each step is passed to the next step
within Laputop, to successively refine the fit.
Each step of the fit can be configured separately by the user, using different
ParametrizationService and LikelihoodService parameters (so, for each of the
steps separately you can specify which 
fit parameters you would like to be fixed or floating, which likelihood
function to use, as well as things like stepsizes and boundaries).

A sample python script which runs Laputop can be found in the code's "resources"
folder:  ``resources/examples/try_gulliverized_fourstep_fitter.py``
In this example, *three* steps are performed.  Laputop can perform one,
two, three, or four steps.

What are the "best" settings for running Laputop?
============================================================

This is a very good question!  The code is structured to give you a lot of 
flexibility to explore different settings...
Any answer I give in this document is likely to change over time.

To get started quickly, try using a "traysegment".  A traysegment is a collection of services and modules
which together perform one task, which you can call as a "black box" from your python script.  
There are four Laputop traysegments in  ``resources/examples``:

- A three-step "standard" configuration (Tom Feusels' favorite parameters as of Berkeley, March 2012)
- A one-step configuration for small showers
- A configuration for using a toprec fit that you've already performed as a seed, and only performing the last step 
  (i.e. with a new snow factor)
- A four-step configuration designed to "mimic" the old I3TopLateralFit as closely as possible

There is also a short python script in there demonstrating how to call one of these traysegments.

Here are Tom Feusels' favorite parameters, as of Feb.7, 2012, and modified by Kath on Apr. 24, 2014 to
reflect the addition of the snow correction service.  This is NOT an official script, just a helpful
example!::

  ## The snow correction service
  tray.AddService("I3SimpleSnowCorrectionServiceFactory","SimpleLambda21")(
        ("Lambda", 2.1)
        )

  ## The minimization service
  tray.AddService("I3GulliverMinuitFactory","Minuit")(
      ("MinuitPrintLevel",-2),  
      ("FlatnessCheck",True),  
      ("Algorithm","SIMPLEX"),  
      ("MaxIterations",2500),    # TF : This results in almost NO unconverged fits.
      ("MinuitStrategy",2),
      ("Tolerance",0.01),    
      )

  ## The seed service
  tray.AddService("I3LaputopSeedServiceFactory","ToprecSeed")(
      ("InCore", "ShowerCOG"),
      ("InPlane", "ShowerPlane"),
      ("Beta",2.6),                    # first guess for Beta
      ("InputPulses",datareadoutName)  # this'll let it first-guess at S125 automatically
  )

  ## Three parametrization services for three steps
  tray.AddService("I3LaputopParametrizationServiceFactory","ToprecParam2")(
      ("FixCore", False),        
      ("FixTrackDir", True),
      ("MinBeta", 2.9),   ## From toprec... 2nd iteration (DLP, using beta)
      ("MaxBeta", 3.1),
      ("maxLogS125",8.0),                   # Default is 6., be a bit safer, although should never happen to be this large     
      ("VertexStepsize",10.0*I3Units::m),   # The COG is very good for contained events, don't step too far
      ("LimitCoreBoxSize", 200.*I3Units::m) # Don't give the fit too much freedom. Change for reconstructing uncontained events
      )

  tray.AddService("I3LaputopParametrizationServiceFactory","ToprecParam3")(
      ("FixCore", False),        
      ("FixTrackDir", False),      # FREE THE DIRECTION!
      ("MinBeta", 2.0),   ## From toprec... 3rd iteration (DLP, using beta)
      ("MaxBeta", 4.0),
      ("LimitCoreBoxSize", 15.0),
      ("maxLogS125",8.0),                   
      ("VertexStepsize",5.0*I3Units::m),
      ## Use these smaller stepsizes instead of the defaults:
      ("SStepsize", 0.045),        # default is 1
      ("BetaStepsize",0.15)        # default is 0.6    
      )

  tray.AddService("I3LaputopParametrizationServiceFactory","ToprecParam4")(
      ("FixCore", False),        
      ("FixTrackDir", True),
      ("MinBeta", 1.5),   ## From toprec... 4th iteration (DLP, using beta)
      ("MaxBeta", 5.0),
      ("maxLogS125",8.0),                   
      ("LimitCoreBoxSize", 45.0),
      ## Use these smaller stepsizes instead of the defaults:
      ("VertexStepsize", 4.0),     # default is 20
      ("SStepsize", 0.045),        # default is 1
      ("BetaStepsize",0.15)        # default is 0.6 
      )

  ## The likelihood service (for all three steps)
  tray.AddService("I3LaputopLikelihoodServiceFactory","ToprecLike2")(
      ("datareadout", datareadoutName),  # <--- specify your pulses here
      ("badstations", "IceTopExcludedStations"),
      ("CorrectEnvironment", False),        
      ("SaturationLikelihood", True),
      ("MaxIntraStationTimeDiff",80.0),    # Don't use time fluctuating tanks for timing fits, could really mess up the hard work
      ("dynamiccoretreatment",11.0),     # do the 11-meter core cut
      ("curvature",""),      # NO timing likelihood (actually DEPRECATED, specify in the Fitter)
      ("SnowServiceName", "SimpleLambda21")   # Specify the snow attenuation function
      )

  #####---------- The fitter module ------------------
  tray.AddModule("I3LaputopFitter","Laputop")(
      ("NonStdName","LaputopParams"),  # irrelevant if you choose CombineParams (below)
      ("SeedService","ToprecSeed"),
      ("NSteps",3),                    # <--- tells it how many services to look for and perform
      ("Parametrization1","ToprecParam2"),   # the three parametrizations
      ("Parametrization2","ToprecParam3"),
      ("Parametrization3","ToprecParam4"),
      ("StoragePolicy","OnlyBestFit"),
      ("Minimizer","Minuit"),
      ("LogLikelihood1","ToprecLike2"),     # the three likelihoods
      ("LDFFunctions",["dlp","dlp","dlp"]),
      ("CurvFunctions",["","gausspar","gausspar"])   # VERY IMPORTANT : use time Llh for step 3, but fix direction!
      )


These settings were tested on "normal" cosmic ray events 
(not particularly small or large, mostly at zenith angles of 45 degrees or less).
So if you are reconstructing a special class of events, you might want to get to know
the details of these settings a little better (see the next section) and experiment for yourself. 



