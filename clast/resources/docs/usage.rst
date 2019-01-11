.. _usage:

Usage
=====

The script ``.../resources/examples/basic.py`` covers the absolute basic usage of ``I3CLastModule`` and is copied here::

    #a slightly modified version of .../resources/test/ClastTest.py

    from icecube import clast
    from I3Tray import I3Tray
    import sys

    #here take a data file including gcd information
    dataf = sys.argv[1]

    #here generic outfile name
    outf = "clast.i3"

    #specify pulses to be used
    pulses = "InIcePulses"

    tray = I3Tray()

    #read in file
    tray.Add("I3Reader", Filename=dataf)

    #run clast module. Defaults can be found in sphinx docs or using "icetray-inspect clast"
    tray.Add("I3CLastModule", InputReadout=pulses, Name="clast") #[defaults: InputReadout=RecoPulses, Name=clast]

    #notes: clast (type I3Particle) and clastParams (type I3CLastFitParams) will be written to the frame.

    #write file
    tray.Add("I3Writer", filename=outf)

    tray.Execute(100)

``I3CLastModule`` takes the following arguments:

* ``AmandaMode [Default: False]``
    In the energy calculation, convert the charge corresponding to whether the detector is IceCube [False] or Amanda [True].

* ``AmplitudeOption [Default: 1]``
    In the ToI and ToI covariance calculation. If the option is 1: leave the pulse amplitude (charge) as is. If the option is 0: treat all charges less than 2.0 as a height of 1.0

* ``AmplitudeWeight [Default: 1.0]``
    In the ToI and ToI covariance calculation. the scaling of the pulse amplitude (charge) that can be applied

* ``DirectHitRadius [Default: 300.0*I3Units::m]``
    In the time calculation, only DOMs within this radius to the CoG are considered as direct hits

* ``DirectHitThreshold [Default: 3]``
    Minimum number of direct hits for the earliest vertex time, fewer than this will force the verted time to be the vertex time with the most direct hits regardless of time

* ``DirectHitWindow [Default: 200.0*I3Units::ns]``
    Time window for which direct hits are still counted after the trial vertex time

* ``InputReadout [Default: "RecoPulses"]``
    The input reconstructed pulses that will be used for calculations

* ``MinHits [Default: 3]``
    Minimum total hits, if there are fewer no reconstruction is performed

* ``Name [Default: "clast"]``
    Prefix for the output

* ``AmEnergyParam[0-5]``
    Amanda energy coefficients to convert charge to deposited energy

* ``Energy Param[0-5]``
    IceCube energy coefficients to convert charge to deposited energy
