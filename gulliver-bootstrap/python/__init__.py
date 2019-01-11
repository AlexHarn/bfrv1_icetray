from icecube.icetray import load
load('gulliver-bootstrap', False)
del load
from icecube import icetray
from icecube.load_pybindings import load_pybindings
load_pybindings(__name__, __path__)

@icetray.traysegment
def I3BootstrappedFit(tray, name, seed_service, likelihood_service, pulses='OfflinePulses', If=lambda f: True, n_iterations=4, keep_intermediates=False, do_final_fit=False):
	"""
	Run a series of  bootstrapped gulliver fits (fits to resamplings of the existing pulses). 
	The variation in direction of the fit results is used to compute an estimate of the angular 
	error of the reconstruction of the original pulses (with the same likelihood). 
	Optionally, run a reconstruction of the original pulses, using each of the resampled fit results 
	(and the average of the resampled fit results) as a seed. 
	
	:param seed_service: The name of the service to use for the initial seed. 
		If this is the empty string each resampled fit will be seeded with a linefit to the resampled pulses. 
	:param likelihood_service: The name of the likelihood service to use for all fitting
	:param pulses: The I3RecoPulses to use for the resampling and fitting
	:param If: A Python function, whose result determines whether to process each frame
	:param n_iterations: The number of resampled fits to perform
	:param keep_intermediates: Whether the resamped fit results and fit parameters should be left in the frame
	:param do_final_fit: Whether to do a final fit of the original pulses, using all of the resampled fit results as seeds
	"""
	from icecube import lilliput
	from icecube.icetray import I3Units
	
	likelihoodName=name+"LikelihoodWrapper"
	seedName=name+"SeedWrapper"
	tray.AddService("BootstrappingLikelihoodServiceFactory",likelihoodName)(
		("Pulses",pulses),
		("Bootstrapping",BootstrapOption.Multinomial),
		("Iterations",n_iterations),
		("WrappedLikelihood",likelihood_service)
		)
	tray.AddService("BootStrappingSeedServiceFactory",seedName)(
		("WrappedSeed",seed_service),
		("BootstrappingLikelihood",likelihoodName)
		)
	tray.AddService('I3SimpleParametrizationFactory', name+"Parameterization",
            StepX = 20*I3Units.m, StepY = 20*I3Units.m, StepZ = 20*I3Units.m,
            StepZenith = 0.1*I3Units.radian, StepAzimuth= 0.2*I3Units.radian,
            BoundsX = [-2000*I3Units.m, 2000*I3Units.m],
            BoundsY = [-2000*I3Units.m, 2000*I3Units.m],
            BoundsZ = [-2000*I3Units.m, 2000*I3Units.m])
	tray.AddService('I3GulliverMinuitFactory', name+"Minimizer",
            Algorithm='SIMPLEX', MaxIterations=1000, Tolerance=0.01)
	
	tray.AddModule('I3SimpleFitter', name+"Bootstrap")(
		("If",If),
		("SeedService",seedName),
		("Parametrization",name+"Parameterization"),
		("LogLikelihood",likelihoodName),
		("Minimizer",name+"Minimizer"),
		("StoragePolicy","AllFitsAndFitParams")
		)
	
	tray.AddModule("BootstrapSeedTweak",name+"TweakSeeds")(
		("If",If),
		("BootstrappedRecos",name+"BootstrapVect"),
		("ContainmentLevel",.5),
		("AngularError",name+"ErrorEstimate")
		)
	
	if(do_final_fit):
		finalSeedName=name+"FinalSeed"
		tray.AddService("I3BasicSeedServiceFactory",finalSeedName)(
			("InputReadout",pulses),
			("FirstGuesses",[name+"BootstrapVect"]),
			("TimeShiftType",'TFirst')
			)
		tray.AddModule('I3SimpleFitter', name)(
			("If",If),
			("SeedService",finalSeedName),
			("Parametrization",name+"Parameterization"),
			("LogLikelihood",likelihood_service),
			("Minimizer",name+"Minimizer"),
			("StoragePolicy","OnlyBestFit")
			)
	
	if(not keep_intermediates):
		tray.AddModule("Delete",name+"Cleanup")(
			("If",If),
			("Keys",[name+"Bootstrap",name+"BootstrapFitParams",name+"BootstrapVect",name+"BootstrapFitParamsVect"])
			)
