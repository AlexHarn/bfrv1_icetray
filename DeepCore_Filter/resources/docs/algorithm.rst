
Algorithm
---------

I3DeepCoreVeto
^^^^^^^^^^^^^^
The DeepCoreVeto is the main workhorse of this project and is used as a standard filter in the IceCube collaboration. The algorithm relies on a mixed hypothesis for the particle to look for causal hits occuring outside of the DeepCore fiducial region.

The module requires two sets of hit series (either I3DOMLaunchSeriesMap or I3RecoPulseSeriesMap). The first, the "InputFiducialHitSeries", is the collection of hits occuring in the fiducial volume. This is now generally a SeededRT pulse series (IC79,IC86-1: HLC launch series) occuring on the high quantum efficiency DOMs as well as the strings immediately adjacent to the HQE strings. The second hit series, the "InputVetoHitSeries", is similarly cleaned but contains hits in the non-fiducial ("veto") region.

Once these two hit series are defined, you can add the module in the usual way::

    from icecube import DeepCore_filter
    tray.AddModule("I3DeepCoreVeto<I3RecoPulse>","deepcore_filter",
                   ChargeWeightCoG = False,
                   DecisionName = "DeepCoreFilter_Bool",
                   FirstHitOnly = True,
                   InputFiducialHitSeries = "SRTPulseDCFid",
                   InputVetoHitSeries = "SRTPulseICVeto",
                   VetoHitsName = "",
                   MinToVeto = 1,
                   ParticleName = "",
                   )

The meaning of each of these parameters is explained below.

**I3DeepCoreVeto**

  *ChargeWeightCoG* :
     If using pulses, setting this option to True will calculate the CoG 
     using charge-weighting. If this option is False or if using launches,
     all hits are weighted equally.

  *DecisionName* :
     The name of the output boolean in the frame. The decision is stored as 
     "True" if the frame passes the filter and False otherwise.

  *FitHitOnly* : 
     Use only the earliest hit from each DOM. This was originally introduced
     during the dark days of DOMsimulator, where the number and structure of 
     pulses in simulation did not match data well except for the first hit.
     This is "True" by default to retain compatibility of current filtered data
     with previous years.

  *InputFiducialHitSeries* :
     The name of a hit series (I3RecoPulseSeriesMap[Mask] or I3DOMLaunchSeriesMap)
     to treat as the "fiducial" hits. This is an HLC cleaned launch series for 
     2010-2011 and an SRT cleaned pulse series for 2012+.

  *InputVetoHitSeries* : 
     The name of a hit series (I3RecoPulseSeriesMap[Mask] or I3DOMLaunchSeriesMap)
     to treat as the "veto" hits. This is an HLC cleaned launch series for 2010
     -2011 and an SRT cleaned pulse series for 2012+.

  *VetoHitsName* :
     The name to be used for an output hit series containing identified causally
     connected hits in the veto region. This can be useful for looking at 
     different settings and hit cleaning methods. If this is blank, no hit 
     series is written to the frame.

  *MinToVeto* :
     The DeepCoreFilter uses a threshold cut on the number of identified causal hits
     in the veto region. This parameter controls the minimum threshold at which a
     frame is rejected by the filter.

  *ParticleName* : 
     Internally, the DeepCore veto is comparing a CoG in the fiducial region to all
     given hits in the veto region. Specifying a name with this parameter will result
     in the module writing the CoG information to the frame. This is useful if you 
     want to study reasons for events failing to pass the filter. By default, this
     parameter is blank, which indicates that the CoG should not be written to the 
     frame.


The DeepCoreVeto algorithm is broken into in five separate steps. Each will be outlined separately.

First, the DeepCoreVeto calls `DeepCoreFunctions<I3RecoPulse>::GetAverageTime`. This function will calculate the average time (with charge weighting, if requested) of all hits in the fiducial hit series. This is a normal average: there is no correction for the look-back time from the hit to a vertex (since no vertex is yet calculated). The time is therefore simply

.. math::
    t_{avg} = \frac{\sum_i^{N) t_i}{N}

where N is the number of hits in the hit series. The standard deviation of the times of fiducial hits is also calculated simultaneously. 

.. math::
    \sigma_t = \sqrt{\frac{\sum_i^{N}(t_i - \frac{i-1}{i} (\sum_j^{i-1} t_j)))^2}{N-1}}

The resulting average and standard deviation are then used to perform an internal hit cleaning algorithm in the `DeepCoreFunctions<I3RecoPulse>::ReduceHits` function. In that function, all hits outside of one standard deviation of the mean hit time are removed from the fiducial hit series. 

The hits are then used to calculate a center-of-gravity (CoG) in the `DeepCoreFunctions<I3RecoPulse>::GetCoG` function. This calculation is a simple (charge-weighted, if requested) average position in (x,y,z) for the cleaned fiducial hits accepted by the `DeepCoreFunctions<I3RecoPulse>::ReduceHits` function. 

.. math::
     \vec{r}_{CoG} = \frac{\sum_{i}^{N}{\vec{r_i}}}{N}

A time for the CoG is then calculated by assuming a simple cascade hypothesis with light propagating from the CoG without scattering. This calculation, in the `DeepCoreFunctions<I3RecoPulse>::GetCoGTime` function, implements a "CoG time" for accepted fiducial hits using the definition

.. math::
     t_{CoG} = \frac{\sum_{i}^{N}{t_i - \frac{\left|\vec{r}_i - \vec{r}_{cog}\right|}{c_{ice}} }}{N}

Now that we have both a position and time associated with the CoG, the DeepCoreVeto checks all veto hits for causality. In the software, a veto hit is causally connected with the fiducial hits if it satisfies

.. math::
     0.25\; m/ns \leq \frac{\left|\vec{r}_{CoG} - \vec{r}_{i}\right|}{t_{CoG}-t_i} \leq 0.40\; m/ns

for each hit in the veto hit series. This therefore is checking for a muon entering the fiducial volume and leaving hits in the veto region in the form of a track hypothesis. If at least MinToVeto (default:1) hits are identified as causally connected to the CoG, the event is identified as "failing" the veto and should be rejected. If less than MinToVeto hits are identified as causally connected to the CoG, the even "passes" the veto, recieving a value of True in the output boolean and should be accepted for further processing in the low energy stream.


I3DeepCoreTimeVeto
^^^^^^^^^^^^^^^^^^
The DeepCoreTimeVeto was an early idea at a secondary filter to apply with the DeepCoreVeto described above. The function calls are laregely identical, but the "mixed hypothesis" is removed. Instead, a CoG is calculated using both the fiducial hits and the veto hits (with the associated cleaning occuring in both hit series). The time difference between the two hits should give an indication of whether the hits are consistent with incoming or outgoing light in the fiducial volume. In practice, this second filter overlaps greatly with the DeepCoreVeto and gives roughly similar rejection power. The combination of the two, with decisions given by an AND of the output boolean values, reduces the rate of cosmic ray muons by roughly a factor of two compared to the DeepCoreVeto alone.

The module is maintained for posterity and for potential use by others in the future. To add the module to your own code, use a similar syntax as above::

    from icecube import DeepCore_filter
    tray.AddModule("I3DeepCoreTimeVeto<I3RecoPulse>","deepcore_timeveto",
                   ChargeWeightCoG = False,
                   FirstHitOnly = True,
                   InputFiducialHitSeries = "SRTPulseDCFid",
                   InputVetoHitSeries = "SRTPulseICVeto",
                   TimeThreshold = 500*I3Units.ns,
                   ParticleName = "",
                   )

**I3DeepCoreTimeVeto**

  *ChargeWeightCoG* :
     If using pulses, setting this option to True will calculate the CoG 
     using charge-weighting. If this option is False or if using launches,
     all hits are weighted equally.

  *DecisionName* :
     The name of the output boolean in the frame. The decision is stored as 
     "True" if the frame passes the filter and False otherwise.

  *FitHitOnly* : 
     Use only the earliest hit from each DOM. This was originally introduced
     during the dark days of DOMsimulator, where the number and structure of 
     pulses in simulation did not match data well except for the first hit.
     This is "True" by default to retain compatibility of current filtered data
     with previous years.

  *InputFiducialHitSeries* :
     The name of a hit series (I3RecoPulseSeriesMap[Mask] or I3DOMLaunchSeriesMap)
     to treat as the "fiducial" hits. This is an HLC cleaned launch series for 
     2010-2011 and an SRT cleaned pulse series for 2012+.

  *InputVetoHitSeries* : 
     The name of a hit series (I3RecoPulseSeriesMap[Mask] or I3DOMLaunchSeriesMap)
     to treat as the "veto" hits. This is an HLC cleaned launch series for 2010
     -2011 and an SRT cleaned pulse series for 2012+.

  *TimeThreshold* :
     The DeepCoreTimeVeto uses a threshold cut on the difference in time between
     the veto CoG and the fiducial CoG. A positive number indicates that the veto
     CoG is allowed to be TimeThreshold earlier than the fiducial CoG while a negative
     value would only allow events where the fiducial CoG comes first. The default 
     value of 500 ns was set using IceSim3 simulation and has not been re-optimized.

  *ParticleName* : 
     Internally, the DeepCore veto is comparing a CoG in the fiducial region to a
     CoG in the veto region. Specifying a name with this parameter will result in 
     the module writing both CoGs to the frame. This is useful if you want to do
     your own optimization of the TimeThreshold parameter. If this parameter is 
     blank, which indicates that the CoG should not be written to the frame.

