### based on http://code.icecube.wisc.edu/svn/sandbox/jvansanten/CascadeL3_IC79/trunk/python/level3/reco.py
from icecube.icetray import I3Units, load, traysegment, module_altconfig
from level3_HoboTimeSplitter import HoboTimeSplitter
#from icecube.CascadeL3_IC79.level3.reco import HoboTimeSplitter

@traysegment
def TimeSplitFits(tray, name, Pulses='OfflinePulses', If = lambda f: True):
	"""
	Split the event in two time haves and run SPEFit+CscdLlh on each half separately
	"""
	# HoboTimeSplittere is Jakob's IC79 L3 class to split an event based on charge weighted mean time. 
	# Uses HLC pulses only for getting the mean time but splits the entire uncleaned pulse series 
	tray.AddModule(HoboTimeSplitter, name, Pulses='OfflinePulses', Output=name+'Pulses')
	# Segment that does fits on each half of split event
	tray.AddSegment(SplitFits, name, BasePulses=name+'Pulses', If = If)
	
@traysegment
def CoreRemovalFits(tray, name, Pulses='OfflinePulses', Vertex='CredoFit', If = lambda f: True):
	"""
	Remove the cascade-like core of pulses around a reconstructed vertex
	from the event, and run SPEFit+CscdLlh on the core and corona separately.
	"""
	load('core-removal', False)
	
	tray.AddModule('I3CascadeFitCoreRemoval', name,
	    InputRecoPulseSeries='OfflinePulses', VertexName=Vertex,
	    CorePulsesName=name+'Pulses_0', # Core pulses
	    OutputRecoPulseSeries=name+'Pulses_1', # Melonballed pulses
	    )
	    
	tray.AddSegment(SplitFits, name, BasePulses=name+'Pulses')
	

@traysegment
def SplitFits(tray, name, BasePulses, If = lambda f: True):
	"""
	Run SPEFit and CascadeLlh on split pulse series named thing_0 and thing_1.
	"""
	#load('SeededRTCleaning', False)
	load('STTools', False)
	from icecube.STTools.seededRT.configuration_services import I3DOMLinkSeededRTConfigurationService
        from icecube import icetray
	
	for i in xrange(2): 
		pulses = "%s_%d" % (BasePulses, i)
		srt_pulses = 'SRT' + pulses
		'''
		tray.AddModule('I3SeededRTHitMaskingModule', '%s_SRTCleaning_%d' % (name, i),
			       InputResponse=pulses, 
			       OutputResponse=srt_pulses,
			       RTRadius=150, RTTime=1000, 
			       MaxIterations=3, Seeds='HLCcore')
		'''
		stConfigService = I3DOMLinkSeededRTConfigurationService(
                        ic_ic_RTRadius =  150*I3Units.m,
                        ic_ic_RTTime   = 1000*I3Units.ns,
                        treat_string_36_as_deepcore = False,
                        useDustlayerCorrection      = False,
                        allowSelfCoincidence        = True
                        )

		tray.AddModule("I3SeededRTCleaning_RecoPulseMask_Module", '%s_SRTCleaning_%d' % (name, i),
                        InputHitSeriesMapName  = pulses,
                        OutputHitSeriesMapName = srt_pulses,
                        STConfigService        = stConfigService,
                        MaxNIterations         = 3,
                        SeedProcedure          = "HLCCoreHits",
                        Streams                = [icetray.I3Frame.Physics]
                        )


		tray.AddSegment(SPEFit, '%s_SPEFit_%d' % (name, i), Pulses=srt_pulses)
		tray.AddSegment(CascadeLlhVertexFit, '%s_CascadeLlhVertexFit_%d' % (name, i), Pulses=srt_pulses, If=If)
		tray.AddSegment(DipoleFit, '%s_DipoleFit_%d' % (name, i), Pulses=srt_pulses, If=If)
	
@traysegment
def ImprovedLineFit(tray, name, Pulses, If = lambda f: True):
	"""
	A de-crufted version of the improvedLinefit segment
	"""
	from icecube import improvedLinefit
	
	tray.AddSegment(improvedLinefit.simple, name,
	    inputResponse=Pulses, fitName=name)
	
	# Delete debugging cruft that ends up in the frame
	tray.AddModule('Delete', name+"_delete_cruft",
	    Keys=[name+"_linefit_final_rusage", name+"_debiasedPulses"])
	
	return [name, name+"Params"]
	

@traysegment
#def I3SPEFitter(tray, name, Pulses='OfflinePulses', Jitter=4.0 * I3Units.ns,
def SPEFit(tray, name, Pulses='OfflinePulses', Jitter=15.0 * I3Units.ns,
#	   NoiseRate=10 * I3Units.hertz, Seeds=None, Iterations=1, If = lambda f: True):
	   NoiseRate=10 * I3Units.hertz, Iterations=1, If = lambda f: True):
	"""
	Run SPE Fit.
	"""
	load('gulliver', False)
	load('lilliput', False)
	load('gulliver-modules', False)

	# Services to do Gulliver reconstruction
	tray.AddService('I3SimpleParametrizationFactory', name + '_Param',
	    StepX = 20*I3Units.m, StepY = 20*I3Units.m, StepZ = 20*I3Units.m,
	    StepZenith = 0.1*I3Units.radian, StepAzimuth= 0.2*I3Units.radian,
	    BoundsX = [-2000*I3Units.m, 2000*I3Units.m],
	    BoundsY = [-2000*I3Units.m, 2000*I3Units.m],
	    BoundsZ = [-2000*I3Units.m, 2000*I3Units.m])

	# Define the gulliver minimization sevice to use
	tray.AddService('I3GulliverMinuitFactory', name + '_Minuit',
			Algorithm='SIMPLEX', MaxIterations=10000, Tolerance=0.01,
			)

	# Use convoluted pandel as the PDF for the likelihood
	tray.AddService('I3GulliverIPDFPandelFactory', name + '_Pandel',
	    InputReadout=Pulses, EventType='InfiniteMuon', Likelihood='SPE1st',
#	    PEProb='GaussConvolutedFastApproximation', JitterTime=Jitter,
	    PEProb='GaussConvoluted', JitterTime=Jitter,
	    NoiseProbability=NoiseRate)

	# linefit seed service
	tray.AddService('I3BasicSeedServiceFactory', name + '_Seed',
#	    InputReadout=Pulses, FirstGuesses=Seeds, TimeShiftType='TFirst')
			InputReadout=Pulses, 
			FirstGuesses= ["SPEFit2","LineFit"],     
			TimeShiftType='TFirst',
			)
	
	if Iterations == 1:
		tray.AddModule('I3SimpleFitter', name,
			       SeedService=name + '_Seed', 
			       Parametrization=name + '_Param',
			       LogLikelihood=name + '_Pandel', 
			       Minimizer=name + '_Minuit', 
			       If=If
			       )
	else:
		tray.AddModule('I3IterativeFitter', name,
			       SeedService=name + '_Seed', Parametrization=name + '_Param',
			       LogLikelihood=name + '_Pandel', Minimizer=name + '_Minuit',
			       RandomService="SOBOL", 
			       NIterations=Iterations, 
			       If=If,
			       )

@traysegment
def CascadeLlhVertexFit(tray, name, Pulses, If = lambda f: True):
	"""
	Run CscdLlhVertexFit, seeded with CLast.
	"""
	load('clast', False)
	load('cscd-llh', False)

	seed = name + '_CLastSeed'
	# preparing a seed for CascadeLlhVertexFit
	tray.AddModule('I3CLastModule', name+'Clast',
	    Name=seed, InputReadout=Pulses, If=If)

	# actual CascadeLlhVertexFit 
	tray.AddModule( 'I3CscdLlhModule', name + '_CscdLlh',
			InputType = 'RecoPulse', # ! Use reco pulses
			RecoSeries=Pulses, 
			FirstLE = True, # Default
			SeedWithOrigin = False, # Default
			SeedKey=seed,
			ResultName=name, 
			MinHits = 8, # ! Require 8 hits
			AmpWeightPower = 0.0, # Default
			Minimizer = 'Powell', # ! Set the minimizer to use
			PDF = 'UPandel', # ! Set the pdf to use
			ParamT = '1.0, 0.0, 0.0, false',   # ! Setup parameters
			ParamX = '1.0, 0.0, 0.0, false',   # ! Setup parameters
			ParamY = '1.0, 0.0, 0.0, false',   # ! Setup parameters
			ParamZ = '1.0, 0.0, 0.0, false',   # ! Setup parameters
			If=If,
			)
def DipoleFit(tray, name, Pulses='SRTOfflinePulses', If = lambda f: True):
	load('dipolefit', False)
	
	tray.AddModule('I3DipoleFit', name,
	    InputRecoPulses=Pulses, DipoleStep=4, Name=name)
	
	# Delete debugging cruft    
	tray.AddModule('Delete', name+'_delete_cruft',
	    Keys=[name+'_rusage'])

	


