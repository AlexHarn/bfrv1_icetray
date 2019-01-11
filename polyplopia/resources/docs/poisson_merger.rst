PoissonMerger
-------------
PoissonMerger injects background event read from a separate file ontop of primary events in the chain by sampling from a Poisson distribution.  This C++ module replaces an older Python module and can merge events at the I3MCTree-level after generation. This is likely to become the prefered way since the combined MCTree can be processed through photon propagation at once instead of having to do this in separate tasks during production.

How to use
============
To inject background cosmic-rays into a signal stream, you must first configure an instance of a :cpp:class:`I3GeneratorService` 
and pass it to the I3Module PoissonMerger.  For example, to
pregenerated .i3 files (generated for example by corsika-reader)::

    from icecube.icetray import I3Units
    from icecube.polyplopia import CoicidentI3ReaderService

    # Read events from file ""BACKGROUNDFILE.i3"
    background = polyplopia.CoincidentI3ReaderService()
    background.open("BACKGROUNDFILE.i3")

    tray.AddModule("PoissonMerger","merge",
       BGWeights = "CorsikaWeightMap",
       CoincidentEventService = background,
     )

