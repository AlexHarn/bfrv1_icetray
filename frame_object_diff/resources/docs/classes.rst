C++ Classes
===========

.. module:: icecube.frame_object_diff

.. class:: I3GeometryDiff(base_filename,base,cur)
   :noindex:
   
   A diff between two I3Geometry objects.
   
   :param base_filename: Filename of base.
   :param base: Base I3Geometry object.
   :param cur: Current I3Geometry object.
   
   .. method:: unpack(base)
      :noindex:

      Unpack a diff using a base I3Geometry.
      
      :param base: Base I3Geometry object.
      :returns: Current I3Geometry object.

.. class:: I3CalibrationDiff(base_filename,base,cur)
   :noindex:
   
   A diff between two I3Calibration objects.
   
   :param base_filename: Filename of base.
   :param base: Base I3Calibration object.
   :param cur: Current I3Calibration object.
   
   .. method:: unpack(base)
      :noindex:
   
      Unpack a diff using a base I3Calibration.
      
      :param base: Base I3Calibration object.
      :returns: Current I3Calibration object.

.. class:: I3DetectorStatusDiff(base_filename,base,cur)
   :noindex:
   
   A diff between two I3DetectorStatus objects.
   
   :param base_filename: Filename of base.
   :param base: Base I3DetectorStatus object.
   :param cur: Current I3DetectorStatus object.
   
   .. method:: unpack(base)
      :noindex:
   
      Unpack a diff using a base I3DetectorStatus.
      
      :param base: Base I3DetectorStatus object.
      :returns: Current I3DetectorStatus object.

.. class:: I3I3VectorOMKeyDiff(base_filename,base,cur)
   :noindex:
   
   A diff between two I3I3VectorOMKey objects.
   
   :param base_filename: Filename of base.
   :param base: Base I3I3VectorOMKey object.
   :param cur: Current I3I3VectorOMKey object.
   
   .. method:: unpack(base)
      :noindex:
   
      Unpack a diff using a base I3I3VectorOMKey.
      
      :param base: Base I3I3VectorOMKey object.
      :returns: Current I3I3VectorOMKey object.

.. class:: I3I3VectorTankKeyDiff(base_filename,base,cur)
   :noindex:
   
   A diff between two I3I3VectorTankKey objects.
   
   :param base_filename: Filename of base.
   :param base: Base I3I3VectorTankKey object.
   :param cur: Current I3I3VectorTankKey object.
   
   .. method:: unpack(base)
      :noindex:
   
      Unpack a diff using a base I3I3VectorTankKey.
      
      :param base: Base I3I3VectorTankKey object.
      :returns: Current I3I3VectorTankKey object.

