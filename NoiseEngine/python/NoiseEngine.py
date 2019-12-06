
 ###################################################################################
 # TraySegment: NoiseEngine
 # @Version $Id: $
 # @date: $Date: $
 # @author Michael Larson <mjlarson@crimson.ua.edu>
 # (c) 2011,2012 IceCube Collaboration
 #
 # The NoiseEngine filter module is designed to identify triggers caused by 
 # random detector noise. It does this by using the TrackEngine algorithm,
 # which maps all possible pairs of hits with apparent velocities inside of
 # a given window and maximum time separation onto a binned Healpix unit sphere.
 # If more than some threshold number of pairs land in a single bin, then 
 # the event passes.
 # Thus, the filter looks for any hint of directionality. All parameters are
 # designed to be intentionally easy for any physics events to pass, which
 # results in a dramatic increase in data/MC agreement at very low NCh.
 # For more information and results from IC79, see the Berkeley talk here:
 #  https://events.icecube.wisc.edu/contributionDisplay.py?sessionId=32&contribId=114&confId=43
 ###################################################################################
from icecube import icetray
from icecube.icetray import I3Units

@icetray.traysegment
def WithCleaners(tray, name,
		 HitSeriesName ="Pulses_cleaned", 
		 OutputName = "NoiseEngine_bool",
		 If = lambda f: True, 
                 writePulses = lambda f: True):
	"""
	Clustering algorithm to identify pure noise events, with some noise cleaning before it runs
	For documentation on how these cleaning settings were optimized, see the talk from
	Michael Larson in Uppsala: 
	https://events.icecube.wisc.edu/indico/contributionDisplay.py?contribId=58&sessionId=20&confId=36
	
	:param HitSeriesName: Name of the I3RecoPulseSeries to get pulses from.
	:param OutputName: Name of the output I3Bool to be written to the frame
	:param writePulses: Boolean if you want the hit series that NoiseEngine sees (after the
            pre-processing cleaning) to stay in the frame. Default is true, set to false if you
            would rather just throw these out.
	:param If: the usual python function, makes the segment run conditionally frame by frame. 
	"""
        
	icetray.load("libstatic-twc")
	tray.AddModule( "I3StaticTWC<I3RecoPulseSeries>", name+"StaticTWC",
			InputResponse = HitSeriesName,
			OutputResponse = HitSeriesName+"_STW_"+name,
			TriggerConfigIDs = [1010, 1011], #it's already the default
			WindowMinus = 3000.0,
			TriggerName = "I3TriggerHierarchy",
			WindowPlus = 2000.0,
			If=If
	)
	
	from icecube import STTools
	from icecube.STTools.seededRT.configuration_services import I3DOMLinkSeededRTConfigurationService
	seededRTConfigService_nodust = I3DOMLinkSeededRTConfigurationService(
		useDustlayerCorrection  = False,
		ic_ic_RTTime           = 750*I3Units.ns,
		ic_ic_RTRadius         = 150*I3Units.m
		)
	
	tray.AddModule("I3SeededRTCleaning_RecoPulse_Module", name+"RTCleaning_STTools",
		       AllowNoSeedHits = False,
		       InputHitSeriesMapName = HitSeriesName+"_STW_"+name,
		       OutputHitSeriesMapName = HitSeriesName+"_STW_ClassicRT_"+name,
		       STConfigService = seededRTConfigService_nodust,
		       MaxNIterations = 0,
		       SeedProcedure = "AllCoreHits",
		       If=If,
		       )
	
	from icecube import NoiseEngine
	tray.AddModule("NoiseEngine",name+"NoiseEngine",
		HitSeriesName = HitSeriesName+"_STW_ClassicRT_"+name,
		OutputName = OutputName,
		If=If
	)
	
	tray.AddModule("Delete", name+"NoiseEngineCleanup", 
		keys = [HitSeriesName+"_STW_"+name,HitSeriesName+"_STW_ClassicRT_"+name],
		If= lambda frame: If(frame) and not writePulses
	)

