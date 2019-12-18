.. _trigger-splitter:

================
trigger-splitter
================

 **Maintainer** : Erik Blaufuss

.. toctree::
   :titlesonly:
   
   release_notes


-----------
Description
-----------
This splitter module takes the specified trigger hierarchy in the Q-frame and 
splits the Q-frame into P-frame. It uses the specified triggers to split the 
frame, ignoring the rest of the triggers. If the specified trigges overlap, a 
cut condition is applied on the time between the end of the previous trigger 
and the begining of the next trigger. The effect is as if only the specified 
triggers ran on DAQ with the specified readout windows.

It requires the following parameters set properly:

* **InputResponses** - List of input RecoPulseSeriesMaps (must be in the DAQ frame)
* **OutputResponses** - List of corresponding output RecoPulseSeriesMapMasks names in each P-frame, must be the same number of names as InputResponses, and must not contain any of the names in InputResponses
* **TriggerConfigIDs** - List of the TriggerConfigIDs the module keeps (default all in-ice triggers as of 2011)
* **TrigHierName** - The trigger hierarchy name in the Q-frame (default I3TriggerHierarchy)
* **NoSplitDt** - The time between the end of the previous trigger and the begining of the next trigger considered in which a splitting should no longer happen (default 10us)
* **ReadoutWindowMinus** - Readout window buffer before trigger (default 4us)
* **ReadoutWindowPlus** - Readout window buffer after trigger (default +6us)
* **WriteoutFrameTimes** - Write out the start and end of the readout times in each P-frame (default false)
* **WriteTimeWindow** - Write into p-frames the time window of trigger times.
  **SubEventStreamName** - name to assign to the split in the I3EventHeader.  Currently uses the instance name.
* **UpdateTriggerHierarchy** 

 - NONE : Don't update.
 - UPDATE : Write a new triggerhierachy in the P-frame. (Default)
 - REPLACE : UPDATE and rename the q-frame trigger hierarchy.

