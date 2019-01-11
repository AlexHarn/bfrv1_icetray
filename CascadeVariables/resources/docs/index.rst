.. _CascadeVariables:

Cascade Variables
=================

**Original Author**: Lukas Schulte 

The I3VetoModule is a rather simple tray module for a quick estimation of how "contained" in the detector volume a given cascade-like event is.
It simply loops over a specified pulsemap and identifies the DOMs that have seen the first, last, and brightest (i.e. most charge) hit in the event.
This information is then stored in an I3Veto frame object. A option for reduced output into an I3VetoShort object is available.
    
Input Variables
---------------
- **HitmapName** [DEFAULT="InIceRawData"]: Name of the pulsemap on which I3Veto runs
- **UseAMANDA** [DEFAULT=False]: Whether to include Amanda DOMs (obsolete for more recent data)
- **DetectorGeometry** [DEFAULT=40]: Detector Geometry: 40, 59, 79, or 86 (for ICXX)
- **OutputName** [DEFAULT="veto"]: Name of resulting I3Veto object
- **FullOutput** [DEFAULT=True]: Whether to write full I3Veto object or only I3VetoShort
       	
Examples
--------
The veto_test.py script in resources/test will give an idea of how to run and setup the module.  However, it will only run out of the box if you 
are on a DESY machine, as the default input files are specified for their file system.  Other input files can be specified, run veto_test.py --help
for details.


See Also
--------
.. toctree::
   :maxdepth: 1
   
   release_notes
   C++ API Reference </doxygen/CascadeVariables/index>
   IceTray Inspect Reference </inspect/CascadeVariables>
