PortiaSplitter
==============

The p-frame splitter for I3Portia. 
This is for the L2 EHE process, mainly
to extract events out of SLOP. All DOM launch time
within the time frame provied by trigger-splitter
is bundled and emitted to a single p-frame.


Usage
-----

I3PortiaSplitter (C++ I3Module)
  Parameters:
     DataReadoutName
     Description : input DOM launch name
     Default     : 'InIceRawData'

  EventHeaderName
      Description : input I3EventHeader name
      Default     : 'I3EventHeader'

  IcePickServiceKey
      Description : Key for an IcePick in the context that this module should check before processing physics frames.
      Default     : ''

  If
      Description : A python function... if this returns something that evaluates to True, Module runs, else it doesn't
      Default     : None

  SplitDOMMapName
      Description : out DOM Map name
      Default     : 'SplittedDOMMap'

  SplitLaunchTime
      Description : bool to split launch or not
      Default     : False

  TimeWindowName
      Description : input time window name
      Default     : 'I3TimeWindow'

  pframeName
      Description : input p-frame name
      Default     : 'InIceSplit'
