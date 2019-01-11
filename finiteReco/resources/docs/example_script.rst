Example Script
~~~~~~~~~~~~~~

Below is an example script that shows how to use the `tray segment <http://code.icecube.wisc.edu/projects/icecube/browser/IceCube/projects/finiteReco/trunk/python/segments.py>`_ for length reconstruction that comes with FiniteReco. The script should run "as is" on the cobalt nodes. If you want to run it somewhere else, you probably have to change the paths to the Photonics tables (and make sure you have the correct tables installed). The script runs what is called the `"advanced" method <LengthReco.html>`_, using gulliver. This is highly recommended for all physics purposes. If you want to run the `"simple" method <I3StartStopPoint.html>`_ (i.e. the first guess only), which is significantly faster and can be used for testing, simply replace "advanced" by "simple" in the call of the tray segment (i.e. ``tray.AddSegment(finiteReco.simpleLengthReco, 'simpleLengthReco', [...])``.

Options
^^^^^^^

The segment takes the following options:

* ``inputPulses``: The pulses (an I3RecoPulseSeriesMap or the corresponding mask) to be used. Note that the performance of FiniteReco depends critically on the set of pulses used. In particular, a rather strict noise cleaning (e.g. SeededRT cleaning) might remove many isolated physics hits in the outer layers of IceCube, which might be essential for FiniteReco to determine the start and stop point accurately. In general, it is recommended to use a softer cleaning like classicRT for the pulses fed to FiniteReco.
* ``inputReco``: The reconstructed track (an I3Particle) to be used as input. The more precise the reconstruction, the better the precision of the start and stop points found by FiniteReco. Use the most sophisticated reconstruction you have available! Note that the direction of the reconstruction is not changed by FiniteReco, only the position of the vertex and the length!
* ``geometry``: Determines the strings that are used. Nowadays, most people probably want to change this to ``'IC86'``...
* ``PhotonicsDir``: Directory where the Photonics tables are located. FiniteReco is very picky about using the correct tables. Currently, the only tables that are known to work are the ones at the location given in the example below. If you want to run FiniteReco anywhere else than in Madison, you probably have to copy those tables and modify the paths accordingly.
* ``PhotonicsDriverDir``: Location of the Photonics driver file.
* ``PhotonicsListFile``: File name of the Photonics driver file. Again, this is the only driver file currently known to work with FiniteReco.

Output
^^^^^^

The example script puts four new objects into the frame:

* ``MPEFit_Contained_advancedLengthReco``: An I3Particle. Its direction is identical to that of the seed MPEFit, but its vertex is set to the reconstructed start point and its length to the reconstructed value.
* ``MPEFit_Contained_advancedLengthReco_FiniteCuts``: An I3FiniteCuts object, a container holding a few cut parameters that might be helpful in identifying starting tracks. A description can be found `here <I3FiniteCuts.html>`_.
* ``MPEFit_Contained_advancedLengthReco_FitParams``: An I3LogLikelihoodFitParams object, a container holding parameters returned by gulliver, such as the rLogL and the dof of the likelihood fit.
* ``MPEFit_Contained_advancedLengthReco_StartStopParams``: An I3StartStopParams object, a container holding the likelihood values for the track being starting, stopping, or infinite; returned by `I3StartStopLProb <I3StartStopLProb.html>`_.

The Script
^^^^^^^^^^^^^^

Here is the example script. You can safely copy and paste code from here::

   #!/usr/bin/env python

   from os.path import expandvars

   from I3Tray import *
   from icecube import icetray, dataclasses, dataio, gulliver, finiteReco

   infile  = expandvars('$I3_TESTDATA/event-viewer/Level3aGCD_IC79_EEData_Run00115990.i3')
   outfile = 'advancedLengthReco.i3'

   tray = I3Tray()

   tray.AddModule('I3Reader', 'reader',
                  Filename = infile
                  )

   tray.AddSegment(finiteReco.advancedLengthReco, 'advancedLengthReco',
                   inputPulses        = 'OfflinePulses_FR_DCVeto',
                   inputReco          = 'MPEFit',
                   geometry           = 'IC79',
                   PhotonicsDir       = '/data/sim/scratch/test_pt/photonics-prod/tables/SPICEMie_i3coords/',
                   PhotonicsDriverDir = '/data/sim/scratch/test_pt/photonics-prod/tables/SPICEMie_i3coords/driverfiles/',
                   PhotonicsListFile  = 'SPICEMie_i3coords_level2_muon_resampled.list'
                   )

   tray.AddModule('I3Writer', 'writer',
                  Streams  = [icetray.I3Frame.DAQ, icetray.I3Frame.Physics],
                  Filename = outfile
                  )

   
   tray.Execute()
   
