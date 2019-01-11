.. _STTools_framework_I3STModule:

I3STModule
==========

The I3STModule provides a base icetray module for developing ST algorithm
specific icetray modules. It inherits from I3ConditionalModule.

Parameters
----------

I3STModule provides the following pre-defined icetray module parameters:

- **GeometryName**: Name of the I3Geometry frame object that should be used for
  constructing the spatial context.
- **InputHitSeriesMapName**: Name of the input hit series map.
- **OutputHitSeriesMapName**: Name of the output hit series map.
- **STConfigService**: The ST configuration service object, that provides the ST
  configuration for the particular ST icetray module.
- **Streams**: The set of I3Frame types (a.k.a. streams) for which the
  ``RunSTAlgorithm`` method will be executed.

.. note:: Icetray modules derived from this base module will have the parameters
          described above automatically. They will also be configured
          automatically through the icetray framework. So the ``Configure``
          method of the derived module does not need to call its base method.