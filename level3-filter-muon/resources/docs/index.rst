.. @maintainer Michael Larson for the Nu Sources Group\
.. $LastChangedBy: Yiqian Xu $\
.. $LastChangedDate: 2016-03-08 13:09:44 -0600 (Tue, 08 Mar 2016) $

.. _level3-filter-muon:   

level3-filter-muon
=====================

.. toctree::
   :maxdepth: 1
   
   release_notes

Introduction
------------
The muon L3 scripts contained in this project are aimed at developing a through-going muon sample for use at energies of a few hundred or higher. The scripts take events passing either the muon filter or the EHE filter, applies some general cuts for the northern and southern skies, and updates reconstructions for later event selection stages. Checks for icetop hits are included, but not cut on during this stage of the processing. Documentation of an early version of the IC86 muon L3 is available at https://wiki.icecube.wisc.edu/index.php/IC86_Muon_Level3.

Usage 
--------------
The primary script for users of the muon L3 is python/level3_Master.py. This scrip takes a number of arguments, most of which will not typically need to be changed by users. The options most likely to be modified by users are as follows:

+---------------------------------+---------------------------------+--------------------+--------------------------------+
| Parser Option                   | Description                     | Default            | Good Default?                  |
+---------------------------------+---------------------------------+--------------------+--------------------------------+
| --input, -i                     | I3 file to process. Include     | None               | No                             |
|                                 | multiple files by passing a     |                    |                                |
|                                 | comma-separated list            |                    |                                |
+---------------------------------+---------------------------------+--------------------+--------------------------------+
| --gcd, -g                       | GCD file to use for processing. | None               | No                             |
+---------------------------------+---------------------------------+--------------------+--------------------------------+
| --output, -o                    | Output file to write events to. | None               | No                             |
+---------------------------------+---------------------------------+--------------------+--------------------------------+
| --num, -n                       | Number of events to process. Set| -1 (all events)    | Yes                            |
|                                 | to -1 to process all events.    |                    |                                |
+---------------------------------+---------------------------------+--------------------+--------------------------------+
| --hd5output                     | HDF5 file to write event info.  | None               | Yes                            |
+---------------------------------+---------------------------------+--------------------+--------------------------------+
| --rootoutput                    | ROOT file to write event info.  | None               | Yes                            |
+---------------------------------+---------------------------------+--------------------+--------------------------------+
| --gsiftp                        | URL to use for FPT transfers. If| None               | Yes, if processing             | 
|                                 | given, this will be prepended to|                    | files locally. If              |
|                                 | the gcdfile, infile, outfile,   |                    | using the grid,                |
|                                 | hd5output, and rootoutput paths.|                    | set this to                    |
|                                 |                                 |                    |http://gsiftp.icecube.wisc.edu/ |
+---------------------------------+---------------------------------+--------------------+--------------------------------+


There are also a few options used to custom photonics tables and splines used in reconstructions. These are generally set to the correct options and don'92t need to be modified unless the user does not have access to IceCube'92s cvmfs repository.

+---------------------------------+-------------------------------------------------------------------------------------------------+
| Parser Option                   | Default                                                                                         | 
+---------------------------------+-------------------------------------------------------------------------------------------------+
| --photonicsdir                  | /cvmfs/icecube.opensciencegrid.org/data/photon-tables/SPICEMie/                                 |
+---------------------------------+-------------------------------------------------------------------------------------------------+
| --photonicsdriverdir            | /cvmfs/icecube.opensciencegrid.org/data/photon-tables/SPICEMie/driverfiles                      |
+---------------------------------+-------------------------------------------------------------------------------------------------+
| --photonicsdriverfile           | mu_photorec.list                                                                                | 
+---------------------------------+-------------------------------------------------------------------------------------------------+
| --infmuonampsplinepath          | /cvmfs/icecube.opensciencegrid.org/data/photon-tables/splines/InfBareMu_mie_abs_z20a10_V2.fits  | 
+---------------------------------+-------------------------------------------------------------------------------------------------+
| --infmuonprobsplinepath         | /cvmfs/icecube.opensciencegrid.org/data/photon-tables/splines/InfBareMu_mie_prob_z20a10_V2.fits | 
+---------------------------------+-------------------------------------------------------------------------------------------------+
| --cascadeampsplinepath          | /cvmfs/icecube.opensciencegrid.org/data/photon-tables/splines/ems_mie_z20_a10.abs.fits          | 
+---------------------------------+-------------------------------------------------------------------------------------------------+
| --cascadeprobsplinepath         | /cvmfs/icecube.opensciencegrid.org/data/photon-tables/splines/ems_mie_z20_a10.prob.fits         | 
+---------------------------------+-------------------------------------------------------------------------------------------------+


There is also one legacy option (-r or --restoretwformc), which was added to handle a bug present in some simulation sets where the SplitInIcePulsesTimeRange object was not written in the L2 processing. The bug is fixed in recent simulation sets, but is necessary for 2013+ simulation created before 2018. Enabling this option will add a TimeRange object into the frame if it is missing. Because the object is only added when missing, enabling this should not harm good events.


Processing Stages
-----------------
The primary entry script for the Muon L3 is python/level3_Master.py. This script passes all configuration options to MuonL3TraySegment.MuonL3, which runs the following stages:

level3_Functions.CleanInputStreams
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
https://code.icecube.wisc.edu/projects/icecube/browser/IceCube/meta-projects/combo/trunk/level3-filter-muon/python/level3_Functions.py#L78`

- Reject events which do not pass either the MuonFilter or the EHEFilter L1 processing. Accept only the InIceSplit event stream.

MuonL3TraySegment.RestoreTimeWindow
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
https://code.icecube.wisc.edu/projects/icecube/browser/IceCube/meta-projects/combo/trunk/level3-filter-muon/python/MuonL3TraySegment.py#L16

- Re-runs trigger-splitter in order to recreate the SplitInIcePulsestimeRange object if missing. If the key is already present in the frame, do nothing.

level3_SplitHiveSplitter.SplitAndRecoHiveSplitter 
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
https://code.icecube.wisc.edu/projects/icecube/browser/IceCube/meta-projects/combo/trunk/level3-filter-muon/python/level3_SplitHiveSplitter.py#L34

- Run the HiveSplitter module (Note: should be updated to use IceHive), which uses the geometry of the detector to efficiently split coincident events.

  - Remove afterpulses (which may have been split by HiveSplitter into a separate frame)

  - Attempt to identify and merge erroneously split frames (ie, frame where multiple sub-events were identified, but where only one true particle exists).

  - Remove any split frames produced solely from SLC hits. 

  - Produce a set of time windows for the newly-split frames

  - Run basic L2 track reconstructions (2 LineFits, a single-iteration SPE, a 2-iteration SPE, and an MPE track fit) on the newly-split frames

level3_CalculateCutValues.CalculateCutValues
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
https://code.icecube.wisc.edu/projects/icecube/browser/IceCube/meta-projects/combo/trunk/level3-filter-muon/python/level3_CalculateCutValues.py#L10

- Calculate values using the CommonVariables project to get the number of total hits, direct hits, and some basic information about the `best fit track https://code.icecube.wisc.edu/projects/icecube/browser/IceCube/meta-projects/combo/trunk/level3-filter-muon/python/level3_Functions.py#L7`.

level3_Cuts.DoPrecuts
^^^^^^^^^^^^^^^^^^^^^^
https://code.icecube.wisc.edu/projects/icecube/browser/IceCube/meta-projects/combo/trunk/level3-filter-muon/python/level3_Cuts.py#L88

- Calculate the `average charge-weighted distance to the track https://code.icecube.wisc.edu/projects/icecube/browser/IceCube/meta-projects/combo/trunk/level3-filter-muon/python/level3_Functions.py#L30` with and without DeepCore

- Events with a total charge below 100 PE (ie, low energy events) or with an average charge-weighted distance above 90 PE*m (implying a poor fit) fail the precut

- Events with a zenith angle less than 60 degrees pass if log10(qtot with DeepCore) >= 0.6(cosZenith-0.5)+2.6

- Events with a zenith angle between [60, 78.5] degrees pass if log10(qtot with DeepCore) >= 3.9(cosZenith-0.5)+2.6

- Events with a zenith angle larger than 78.5 degrees (ie, from the Northern hemisphere or horizon) pass the cut


level3_Cuts.DoLevel3Cuts
^^^^^^^^^^^^^^^^^^^^^^^^^^
https://code.icecube.wisc.edu/projects/icecube/browser/IceCube/meta-projects/combo/trunk/level3-filter-muon/python/level3_Cuts.py#L51

- Pass the event if at least 10,000 PE were observed, including hits from DeepCore.

- Find the RlogL (reduced log-likelihood, logL/ndof) from the best fit. If the best-fit track is LineFit, set the RlogL to 1000. If RlogL < 9, pass the event.

- Calculate a PlogL = RlogL*(number of hit DOMs-5)/(number of hit DOMs-3) for events with more than 5 hit DOMs. Set PlogL to 1000 for events with fewer than 5 hit DOMs. If PlogL < 7.5, pass the event.

- Define a "direct hit ellipse" using the track length calculated from direct hits (LDir) and the number of direct DOMs (NDir), ellipse=(LDir/180)**2 + (NDir//10)**2. If the value is greater than 2 and at least 6 direct hits are observed, pass the event.

- If an event passes at least one of the previous three conditions, and has a reconstructed zenith of at least 85 degrees (ie, Northern hemisphere), the event satisfies the L3 processing.

- If an event passes at least one of the previous three conditions, has a reconstructed zenith of less than 85 degrees (Southern hemisphere + horizon), and the event satisfies a 2d cut in zenith and log10(total charge), the event satisfies the L3 processing.

level3_Reconstruct.DoReconstructions
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
https://code.icecube.wisc.edu/projects/icecube/browser/IceCube/meta-projects/combo/trunk/level3-filter-muon/python/level3_Reconstruct.py#L268

- Rerun the L2 reconstructions using the new event splits, including MuEx, Cramer-Rao, and Paraboloid

- Run Cramer-Rao and Paraboloid as angular error estimators using the the first good fit from ["MPEFit",'a0"SPEFit2",'a0"SPEFitSingle",'a0"LineFit"].

- Run SplineMPE over the events

- Run MuEX, TruncatedEnergy and Millipede over the events to obtain energy estimates 

- Test for IceTop hits using the shield module using several separate estimated in-ice reconstructed directions. Note that we are not cutting on these variables at this point!

level3_WriteOutput.WriteOutput
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
https://code.icecube.wisc.edu/projects/icecube/browser/IceCube/meta-projects/combo/trunk/level3-filter-muon/python/level3_WriteOutput.py#L6

- Write basic MC information (like information about the primary neutrino and muon) to the frame, if available

- Also run the final best-tracks through CommonVariables for use in later event selections

- Write out information to i3, hdf5, and/or root files 


To Do:
---------------------------------------
- Add some figures to explain this more visually
- Check whether the C++ modules are actually used anywhere. If they are not, then consider deleting them.
