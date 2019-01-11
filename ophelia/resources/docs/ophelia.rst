
========
Ophelia
========

Ophelia is an analysis suite for (but not limited to) EHE analysis.
It does quick reconstruction and provides storage for relevant information used in the analysis.


Data Classes
============

Data classes are in ophelia/particle directory.

I3OpheliaRecoResult
-------------------

I3OpheliaRecoResult is a data class to store summarized reconstruction results 
which is I3OpheliaFirstGuessTrack and vector of I3OpheliaParticle.

I3OpheliaParticle
-------------------

I3OpheliaParticle is a data class to store energetic cascades induced by 
bremsstrahlung and photonulear interaction. 

I3OpheliaFirstGuessTrack
------------------------

I3OpheliaFirstGuessTrack inherits the I3OpheliaParticle and is a data class 
to store result of I3EHEFirstGuess which is a reconstruction module using 
(improved) line fit algorithm. This class contains I3Particle, line fit velocity, 
position of center of brightness (COB), fit quality and a bool variable whether 
line fit is succeeded or not. Some of the information is also saved in I3Particle.

Reconstruction Module
=====================

I3EHEFirstGuess
---------------

I3EHEFirstGuess is a I3Module to reconstruct tracks by line fit algorithm using portia 
based charge information. Before running this module one have to process portia which 
calculate charge detected by each DOM. Result of line fit is stored in I3OpheliaFirstGuessTrack. 


Usage
^^^^^

  I3EHEFirstGuess (C++ I3Module)

    

  Parameters:
    ChargeOption
      Description : [0 bigger(1 or 2) : use 1 : use 2 ]
      Default     : 0

    IcePickServiceKey
      Description : Key for an IcePick in the context that this module should check before processing physics frames.
      Default     : ''

    If
      Description : A python function... if this returns something that evaluates to True, Module runs, else it doesn't
      Default     : None

    InputLaunchName
      Default     : 'InIceRawData'

    InputPortiaEventName
      Default     : 'PortiaEvent'

    InputPulseName1
      Default     : 'ATWDPortiaPulse'

    InputPulseName2
      Default     : 'FADCPortiaPulse'

    LCOption
      Description : if non-zero use only LC pulse
      Default     : 0

    MinimumNumberPulseDom
      Default     : 0

    NPEThreshold
      Description : Channel wise NPE threshold to be used
      Default     : 0.0

    OutputFirstguessName
      Default     : 'OpheliaFirstGuess'

    OutputFirstguessNameBtw
      Default     : 'OpheliaFirstGuessBaseTimeWindow'

    OutputParticleName
      Default     : ''

    OutputParticleNameBtw
      Default     : ''

    inputSplitDOMMapName
      Default     : 'SplittedDOMMap'


