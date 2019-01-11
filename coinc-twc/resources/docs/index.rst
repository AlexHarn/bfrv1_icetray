.. _coinc-twc:

Coinc-TWC
*********

:Authors:
  T. Feusels <tom.feusels@ugent.be>, S. De Ridder <sam.deridder@ugent.be>	

.. toctree::
   :maxdepth: 2

   release_notes

Description
===========

This project looks for causally connected InIce SMT triggers (with the IceTop SMT) when there are 
multiple InIce triggers. Then the InIce pulses are cleaned around the chosen trigger. A particle 
travelling at light speed through the ice will arrive about 4350 ns later at the top of the IceCube
than when it passed the surface and travelled vertically. If it travels at an angle of 30 degree, the 
particle will arrive about 6500 ns later. Because the IceTop trigger is mainly caused by the EM component
which might arrive a bit later as the HE muons in the core of the shower and to be safe on the late side
too, the default causalTriggerWindow (InIce SMT Trigger Time minus IceTop SMT Trigger Time) is 
expected to be in [3500,9000] ([windowMin,windowMax]).
Once the causal trigger is found, the pulse/DOMLaunch cleaning window will be defined by [Trigger Time 
- cleanWindowMinus, Trigger Time + TriggerLength + cleanWindowPlus]. For RecoPulses, the  defaults for 
both cleanWindowMinus and cleanWindowPlus are on the safe side. For DOMLaunches, they should be very 
small.

However when the TriggerLength of the causal InIce SMT is so large that it encloses also a second (well
separated) InIce SMT, then it might be good to restrict the length of the pulse cleaning window. The default 
is safe and much larger than what one might choose for a Dynamic hitcleaning (as a possible next step).
When there are multiple InIce SMT which fit in the causal trigger window, one can either choose to keep 
all of them (which is safer because at an early stage one doesn't which is the actual coincident IT-II shower)
and then the cleaning window will be extended, or not and then the most probable, earliest one will be used.
To be really conservative and to keep always at least one InIce SMT, there is the possibility to "keep looking",
which means that the causal trigger window will be extended by 0.5us each time until at least one trigger is
found.

There are very rare cases (< 0.1 % of all IceTop triggered events) where there are multiple IceTop SMTs and
multiple InIce SMTs. If there is an IceTopVEMPulses available in the frame from topeventbuilder, this
could be used to select the "best" IceTop SMT. If not then the code can treat them separately and will extend
the pulse cleaning if multiple causal InIce SMT are found (or with keepLooking must be found). However if
at a later stage, topeventbuilder puts the IceTop SMTs in separate pulse series, then one should run the module
again to select the right InIce pulseseries.

Since IC79 there are two kinds of InIce SMTs : the global InIceSMT8 and the new InIceSMT3 (DeepCore SMT). Optionally 
the user can only look for causal triggers between IceTopSMT and InIceSMT8 (or InIceSMT3).
NEW : Search strategy for looking for causally connected triggers. In data there are cases where multiple physics events
happen in the same timewindow and only generate one InIce SMT8 trigger, with a longer triggerlength. In this case the
trigger won't be windowMin ns later the IceTop trigger. The characteristic for these type of events is that the InIce SMT
starts earlier than windowMin, but the trigger is longer and will extend out in the windowMin,windowMax window as the 
coincident IT-II event still starts in that window (but the trigger happened earlier). Hence "method2" takes the this
into account and adds a triggerlength dependent region before windowMin, ie. triggerlength > - triggertimediff + minWindow
+ mintriggerlength of the IT-II event. For the moment the mintriggerlength is set to 300 ns. As IT has a very good reconstruction
the random coincidences that pass through, will be cleaned out with an inice size cut (icecube_scale). Method2 saves about
1.6% of extra real IT-IC coincidences, to give a grand total of more than 99.6% of real coincidences with the correct 
trigger selection and NO random coincidences remaining (method1 had about 98%).

Question : Could there be a case where an early InIce SMT happens before windowMin (selected with method2) and the second,
real IT-IC coincident InIce SMT in the window between windowMin and windowMax and how is dealt with this in the code?
Answer : Impossible as any HLC within 5 microsec of the previous HLC will extend the triggerlength. As method2 already
requires that the triggerlength extends into [windowMin,windodwMax] there is no more room for a second InIce SMT in that window.
From a cross-check plot which looks at the minimum time difference between multiple InIceSMTs they are at last 6 microsec separated.
So the code does not have to deal with this situation.

More info :I3Wiki:Coincident_Time_Window_Cleaning

Parameters
==========

::

  -inputresponse                [string, DEFAULT=""] Label under which the input readout (DOMLaunches or RecoPulses) is found.
  -outresponse                  [string, DEFAULT=""] Label under which the cleaned output readout (DOMLaunches or RecoPulses) will be put.
  -triggerhierarchyname         [string, DEFAULT="I3TriggerHierarchy"] Label under which the triggers are stored in the frame.
  -windowMin                    [double, DEFAULT=3500.] Defines the minimum of the causal trigger window.
  -windowMax                    [double, DEFAULT=9000.] Defines the maximum of the causal trigger window.
  -cleanWindowMinus             [double, DEFAULT=300.] Defines an extra time before the InIce trigger time for starting the window where the pulses will be kept.
  -cleanWindowPlus              [double, DEFAULT=400.] Defines an extra time after the (InIce trigger time+triggerlength) for ending the window where the pulses will be kept.
  -cleanWindowMaxLength         [double, DEFAULT=9000.] If not 0, defines the maximum length of the cleaning window.
  -keepMultipleCausalTriggers   [bool, DEFAULT=true] If there are multiple InIce SMTs which fit in the CausalTriggerWindow, then those pulses can also be kept. 
  -keepLookingForCausalTrigger  [bool, DEFAULT=false] If there was no InIce SMT found in the specified window, the window Can be extended by 0.5 us on both sides until at least one InIce SMT was found. 
  -IceTopVEMPulsesName          [string, DEFAULT="IceTopVEMPulses_0"] If there are also multiple IceTop SMTs, the IceTop SMT that TopEventBuilder selected for its VEMPulses will be selected.
  -CheckSingleSMTs              [bool, DEFAULT= true] If there is only one InIceSMT, also check whether this one is causally connected and clean the InIce Pulse series.
  -UseSMT8                      [bool, DEFAULT=true] Look for causal triggers between the IceTopSMT and the InIceSMT8.
  -UseSMT3                      [bool, DEFAULT=false] Look for causal triggers between the IceTopSMT and the InIceSMT3.
  -Strategy                     [string, DEFAULT="method1"] Looking for causal triggers in static window between windowMin and windowMax (method1) or add an additional search triggerlength dependent search region (method2 : triggerlength must be higher than windowMin minus triggertime difference (ii-it) + a minimum triggerlength of 300 ns).
  -Stream                       Run it on Q frames or P frames?

Examples
========

PlotTriggersAndPulses.py
 This script can plot the icetop and inice triggers, together with on one plot the icetop and all uncleaned inice pulses, and on the other the icetop and selected (by coinc-twc) inice pulses.                           

