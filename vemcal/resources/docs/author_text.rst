Information From Author
=======================

vemcal
        Vertical Equivalent Muon Calibration for IceTop
	
author of project
        Tilo Waldenmaier <tilo.waldenmaier@desy.de>
maintaned by 
        Serap Tilav <tilav@udel.edu>
Description
        The vemcal project is a collection of icetray modules, standalone routines and scripts to perform the muon calibration of the IceTop DOMs. In the following a brief description of the individal modules is given.

Usage
        See example script `< ../../examples/vemcal_PnF_filter.py>`

I3VEMCalExtractor Module 
        The I3VEMCalExtractor is the most important module which is intended to run on the JEB filter clients at South Pole to extract all the needed information from the IceTop minimum bias and the standard IceTop pulses. The extracted information is written it in a condensed format (I3VEMCalData) back to the frame in order to transfer it to the North. The I3VEMCalExtractor has the following configuration parameters:


+-----------------------------+----------------------------------------------+------------------------------------------------------------------------------------------------------------------+
|Parameter Name               |Parameter Input Detaiails                     |Parameter Description                                                                                             |
+=============================+==============================================+==================================================================================================================+
|IceTopMinBiasPulsesName      |[string, DEFAULT="IceTopMinBiasPulses"]:      |Name of IceTop MinBias pulses in frame.                                                                           |
+-----------------------------+----------------------------------------------+------------------------------------------------------------------------------------------------------------------+
|IceTopPulsesName             |[string, DEFAULT="IceTopPulses"]:             |Name of standard IceTop pulses in frame.                                                                          |
+-----------------------------+----------------------------------------------+------------------------------------------------------------------------------------------------------------------+
|InIcePulsesName              |[string, DEFAULT="InIcePulses"]:              |Name of standard InIce pulses in frame.                                                                           |
+-----------------------------+----------------------------------------------+------------------------------------------------------------------------------------------------------------------+
|MaxCharge                    |[double, DEFAULT=30000.0]:                    |Upper charge limit (in [pe]) for the standard IceTop hits.                                                        |
+-----------------------------+----------------------------------------------+------------------------------------------------------------------------------------------------------------------+
|MaxHGLGTimeDifference        |[double, DEFAULT=20.0]:                       |Upper limit in [ns] for the time difference between the high gain (HG) and the low gain (LG) hit in the same tank.| 
+-----------------------------+----------------------------------------------+------------------------------------------------------------------------------------------------------------------+
|VEMCalDataName               |[string, DEFAULT="I3VEMCalData"]:             |Name of the I3VEMCalData container in which the data will be stored in the frame.                                 |
+-----------------------------+----------------------------------------------+------------------------------------------------------------------------------------------------------------------+




