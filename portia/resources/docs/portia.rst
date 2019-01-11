Portia
======

I3Portia filles classes of I3PortiaPulse and I3PortiaEvent
by looping over all the DOM launches and integrating ATWD and FADC
waveforms. Then divide them by a single electron signal charge.
Output pulse information is stored in I3PortiaPulse class. 
I3PortiaPulse has a member of I3RecoPulse and basic information 
is filled in the class. Output NPEs from ATWD and FADC are compared 
dom-by-dom basis and whichever gives a larger value is summed up for 
the event-sum NPE value. These event-wise information is stored in 
I3PortiaEvent class. Main members are NPE from ATWD and FADC whichever 
gives larger value, ATWD based NPE sum, and FADC based NPE sum.

Making the new pulse
--------------------

There is an option to make best pulse. 
The best pulse means that in addition to the ATWD based NPE, FADC
based NPE obtaind after the ATWD time-window size is added. This is
effective because ATWD time window is narrow but saturation point
is high, while FADC time window is wide but saturation point is low.

If this option is chosen, the event-wise best NPE value is also
calculated based on the sum of ATWD plus after-window FADC based NPEs. 

Usage
-----

Available parameters are listed below (from icetray-inspect portia)::

  I3Portia
   ATWDBaseLineOption
      Description : ATWD baseline option
      Default     : 'eheoptimized'

    ATWDLEThresholdAmplitude
      Description : ATWD LE threshold amp
      Default     : 5.000000000000001e-13

    ATWDPortiaPulseName
      Description : output ATWD portia pulse name
      Default     : 'ATWDPortiaPulse'

    ATWDPulseSeriesName
      Description : output ATWD reco pulse series name
      Default     : 'ATWDPulseSeries'

    ATWDThresholdCharge
      Description : ATWD threshold charge
      Default     : 624150.9744511525

    ATWDWaveformName
      Description : input ATWD waveform name
      Default     : 'CalibratedATWD'

    BestPortiaPulseName
      Description : output Best portia pulse 
      Default     : 'BestPortiaPulse'

    DataReadoutName
      Description : input DOM launch name
      Default     : 'CleanInIceRawData'

    FADCBaseLineOption
      Description : FADC baseline option
      Default     : 'eheoptimized'

    FADCLEThresholdAmplitude
      Description : FADC lE threshold amp
    FADCPortiaPulseName
      Description : output FADC portia pulse name
      Default     : 'FADCPortiaPulse'

    FADCPulseSeriesName
      Description : output FADC reco pulse series name
      Default     : 'FADCPulseSeris'

    FADCThresholdCharge
      Description : FADC threshold charge
      Default     : 624150.9744511525

    FADCWaveformName
      Description : input FADC waveform name
      Default     : 'CalibratedFADC'

    IcePickServiceKey
      Description : Key for an IcePick in the context that this module should check before processing physics frames.
      Default     : ''

    If
      Description : A python function... if this returns something that evaluates to True, Module runs, else it doesn't
      Default     : None

    MakeBestPulseSeries
      Description : Bool to choose to make best pulse
      Default     : False

    MakeIceTopPulse
      Description : Make IceTop pulse or not
      Default     : False

    OutPortiaEventName
      Description : output Portia event name
      Default     : 'PortiaEvent'

    PMTGain
      Description : default value of PMT Gain
      Default     : 10000000.0

    ReadExternalDOMMap
      Description : Bool to read splitted DOM map
      Default     : False

    SplitDOMMapName
      Description : in DOM Map name
      Default     : 'SplittedDOMMap'

    TopDataReadoutName
      Description : IceTop Raw DOMLaunch Name
      Default     : 'CleanIceTopRawData'

    UseFADC
      Description : Bool to choose use fadc waveforms
      Default     : True

    inTopATWDWaveformName
      Description : IceTop calibrated waveform name
      Default     : 'CalibratedIceTopATWD'

  -----------------------------------------------------------------------------
