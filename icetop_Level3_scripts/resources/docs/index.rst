.. _icetop_Level3_scripts:

icetop_Level3_scripts
*****************************************************

:Author:
 S. De Ridder <sam.deridder@ugent.be>

.. toctree::
   :maxdepth: 1

   release_notes

Description
===========

This project is created by the cosmic ray working group in order to have a unified, more advanced processing for most cosmic ray related studies.
The Level3 processing will be useful for you if your analysis requires a standard IceTop filter to be passed (i.e. IceTopSTA3). If not, this project is still a collection of interesting IceTop-related scripts.

Shortly summarized, it takes the L2 files, and then:

- Cleans up the L2 files
- Includes the correct snow heights (which is not the case in L2!)
- Unifies the naming over several years of data taking
- Reruns the filters and selects the events which pass the IceTopSTA3 (or IceTopSTA5) filter.
- Does some IceTop pulse cleaning, reconstruction and creates frameobjects useful for for example cuts.
- Tries to find InIce pulses related to the air shower. (correlated in time with the IT pulses)
- Performs cleaning and an energy loss reconstruction (millipede) along the muon bundle track, where the track is taken from the IceTop reconstruction.
- Add a container with possibly useful variables to cut on.
- Runs the muon Level3 reconstruction:  it reconstructs a completely independent track (from the one reconstructed in IceTop) in the ice and runs some energy loss reconstruction along this track.

A detailed overview of what is done, which variables exist in the output files and some verification is added on the IceTop L3 wiki: https://wiki.icecube.wisc.edu/index.php/IceTop_Level3_file_structure

Level3 GCD
==========

For IceTop, existing GCD files do not contain all correct information. For example, snow heights are not known yet at the moment of the creation of the GCD file. Therefore, in L3 we create our own L3 GCD file which resolves these issues. These are small files which only save a difference with an existing GCD file. It uses the frame_object_diff project for this.
Creating those GCDs first is needed before it is possible to run the Level3 processing.

Data
^^^^

For data, we will put the correct snow heights in the L3 GCD file, together with the verifed VEMCal constants. The L2 VEMCal constants (which are in the standard GCD file) will be saved as separate frame-objects. Thus, the L2 GCD file is needed, together with a reference GCD file which will serve as a base for the diff. For the already produced IceTop L3, we used the MC GCD file for that certain config as base.

Script: resources/scripts/MakeL3GCD.py
Parameters:

::
 --runnumber             [int] Number of the run to be processed
 --day                   [int] Day for snow height interpolation
 --month                 [int] Month for snow height interpolation
 --year                  [int] Year for snow height interpolation
 --output                [string] Shiny new Level 3 GCD-File
 --MCgcd                 [string] MC GCD File which will be the base for the diff
 --L2-gcdfile            [string] Manually specify the L2 GCD file to be used. When you run in Madison with the standard L3 GCD diff, this is not needed.

Simulation
^^^^^^^^^^

In fact, L3 GCD files for MC should be exactly the same as the used GCD files for production. However, for the IC79 MC, a bug has been discovered which affects the snow heights that were simulated. This is called the "Observation level bug" (https://wiki.icecube.wisc.edu/index.php/Observation_Level_Bug ). This is solved in the L3 GCD files. For the datasets not affected by this bug, nothing is changed to the GCD and thus the diff is very small

Script: resources/scripts/MakeL3GCD_MC.py
Parameters:

::
 --output                [string] Shiny new Level 3 GCD-File
 --MCgcd                 [string] MC GCD File which will be the base for the diff
 --obsLevel              [float, DEFAULT=2834.] Used observation level in the simulation. (Either 2834. or 2837.)

Run IceTop Level3
=================

The main script to run the IceTop L3 processing is level3_IceTop_InIce.py in resources/scripts. The level3_iceprod.py script in the same directory is similar but is used for submission through iceprod. They should be merged in the future though.
At the end of the processing, also verification_histograms are saved (specified by --histos), together with a pickle file containing a distribution showing the time differences between events, in order to get/check the livetime (--livetime).

Parameters:


 inputFiles              [Array of strings] Input L2 Files

::
 --L3-gcdfile            [string] Manually specify the L3 (diff) GCD file to be used. When you run in Madison, this is not needed (Will be found in default directory).
 --L2-gcdfile            [string] Manually specify the L2 GCD file to be used. When you run in Madison with the standard L3 GCD diff, this is not needed.
 --output, -o            [string] Output file name
 -n                      [int] Number of frames to process
 --det, -d               [string] Detector configuration name, eg: IC79, IC86.2011, IC86.2012. Auto-detected if filename has standard formatting
 --do-inice              [bool, DEFAULT=False] Also do the in-ice reconstruction?
 --waveforms             [bool, DEFAULT=False] Extract waveforms (only if more than 5 stations in HLC VEM pulses)
 --select                [bool, DEFAULT=False] Apply selection (containment, max. signal and min. stations). Default is to calculate variables but not filter.
 --isMC, -m              [bool, DEFAULT=False] Is this MC?
 --dataset               [int] Dataset number for MC. Needed when using default GCD, to look for it..
 --add-jitter            [bool, DEFAULT=False] Do we add extra jitter on the IT pulses in MC?
 --run                   [int] Runnumber, needed for data. Needed when using default GCD, to look for it..
 --snow-lambda           [float] Attenuation factor for snow. If not defined, use the one defined for every year.
 --spe-corr              [bool, defualt=False] Should we do the SPE correction during Level3?
 --histos                [string] Histograms file name. Needs to be pickle file.
 --livetime              [string] Livetime file name, only needed for data. Needs to be pickle file

Examples
========

The examples directory contains:

- an example script for booking to root files (bookLevel3.py)
- an example script to only write the histos and livetime files (writeHistosAndLivetime.py)
- the histos directory containing scripts to plot the verification histograms (with a README in the directory).
