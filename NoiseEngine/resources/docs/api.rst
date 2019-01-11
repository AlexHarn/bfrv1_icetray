
API
---

NoiseEngine can be added to your scripts using either a standard IceTray module::

   from icecube import NoiseEngine
   tray.AddModule("NoiseEngine","NoiseEngine",
                  HitSeriesName = "TWSRTOfflinePulses",
                  ChargeWeight = True,
	          EndVelocity = 1.0,
		  HealpixOrder = 1,
		  NChLimit = 20,
		  OutputName = 'NoiseEngine_bool',
		  StartVelocity = 0.1,
		  Threshold = 3.0,
		  TimeWindowLength = 750,
		  )

or in the preferred way as a tray segment::

   from icecube import NoiseEngine
   tray.AddSegment(NoiseEngine.WithCleaners,"NoiseEngine",
                   HitSeriesName = "OfflinePulses",
                   OutputName = "NoiseEngine_bool",
                   writePulses = False)
    

The meaning of each of these parameters is explained below.

**NoiseEngine**

  *HitSeriesName* :
     The name of the hit series to use. For the module, this should be cleaned. 
     For the segment, pass an uncleaned hit series (which will be cleaned internally)

  *OutputName* :
     The name of the output boolean in the frame. The decision is stored as 
     "True" if the frame passes the filter and False otherwise. "Passes the 
     filter" in this case implies that the frame is likely to contain a particle
     interaction and should be kept for further processing.

  *ChargeWeight* : 
     Should hits with higher charge be favored over hits with lower charge? If this
     is enabled, each pair of hits will be weighted according to the average charge
     of the pair. If disabled, every pair has a weight of 1.

  *HealpixOrder* :
     The module bins pairs of hits into a unit sphere with equal area per bin. The
     binning is performed using the HEALPix code (http://healpix.jpl.nasa.gov/). The
     "order" of the map determines the number of bins: nbins  12*4^(order).

  *NChLimit* :
     Because the NoiseEngine module attempts to form all possible pairs of hits, the 
     module runs with O(n^2) complexity. In order to reduce the processing time, a hard
     limit on the number of hits is introduced here. Any event with nChannel above this
     value will not be processed under the assumption that noise triggers are exceedingly
     unlikely to produce such an event.

  *StartVelocity* :
     A velocity window is used to limit the pairs of hits to only those likely to be 
     causally connected. This is the lower limit on the apparent velocity between hits
     in units of m/ns (ie, standard IceCube units)
     
  *EndVelocity* :
     The end of the velocity window described in the StartVelocity parameter

  *Threshold* :
     The minimum number of pairs of hits in a single bin in order to accept the event as
     likely to be caused by a physics interaction (and thus "pass" the filter).

  *TimeWindowLength* :
     NoiseEngine performs a sliding time window in order to maximize the number of 
     interesting hits available. This is the length of the time window used.

  *writePulses* :
     If you want a recording of the pulses picked by the sliding time window, set this to True.

