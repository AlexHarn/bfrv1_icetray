I3DSTExtractor
==============

:cpp:class:`I3DSTExtractor` is meant to be used in the northern hemisphere in order to
decode or extract the dst information from the .i3 files arriving from the PF
filter. The purpose of this module is to extract inforation from the DST and
encode it in a different format or expand it within the I3Frame depending on
how it is configured. Because I3DSTExtractor can no longer expect to find an
I3DSTHeader on the first frame, it uses frame buffering in order to search
ahead for the first I3EventHeader in the frame (i.e. the first event to pass
the filter). It then uses information contained in the I3EventHeader in order
to generate the DSTHeader. If writting to a .dst (.zdst) file the DST time is
replaced by a delta_t relative to the time of the DSTHeader. The following are
options which need to be configured in :cpp:class:`I3DSTExtractor`::

   EventHeaderName (string)
      Description :  The Event Header Name (to add)
      Default     :  "InIceEventHeader"

   ExtractToFrame (bool)
      Description :  Should build I3 dataclasses in frame
      Default     :  0

   FileName (string)
      Description :  Name of file to save DST to (suffix can be one of .root or .hdf5)
      Default     :  ""

Other options are available but in general should be left to defaults.

:js:data:`I3DSTExtractor13`
^^^^^^^^^^^^^^^^^^^^^^^^^^
Decodes and extracts :cpp:class:`I3DST13` and :cpp:class:`I3DSTReco13` frame objects and exports them to external file formats or optionally rehydrates reconstruction objects to the I3Frame.


:js:data:`I3DSTExtractor16`
^^^^^^^^^^^^^^^^^^^^^^^^^^
Decodes and extracts :cpp:class:`I3DST16` frame objects and exports them to external file formats or optionally rehydrates reconstruction objects to the I3Frame.
