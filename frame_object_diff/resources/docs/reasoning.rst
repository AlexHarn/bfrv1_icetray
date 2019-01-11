Reasoning
=========

General Compression Formula
---------------------------

Where possible, compression was attempted by just taking the
difference of Vector and Map objects and leaving other objects
uncompressed.  When more compression is needed, bitsets are
used to mask members that stayed the same.

I3FrameObjects
--------------

Anything that is derived from an I3FrameObject can store the filename
that the base object came from.  However, if that object is not actually
in the frame and only present inside another object, then the filename
field will be set to an empty string.

Map Compression
---------------

Maps are compressed by storing the keys of any deletions and the full
key-value pairs of any additions/overwrites.

Vector Compression
------------------

Vectors are compressed by only writing the difference between
the two vectors.  The difference is calculated by using the
"longest common subsequence" algorithm. Specifically, this one::

    "A linear space algorithm for computing maximal common subsequences"
    D. S. Hirschberg
    http://portal.acm.org/citation.cfm?id=360861

This is roughly the same algorithm that ``diff`` uses to compare two files.

One consequence of this is that if there is any change in the
element in the container the algorithm will call the entire element
an addition and save the entire thing.  So for a minor change to all
elements in the vector, no space has been saved becase the vector
looks completely different.

One solution for this is to use a map instead of a vector for elements
that have fixed positions.

There is also the option of a second vector compression scheme based
on fixed positions, ``FixedPositionVector``.  This is implemented,
but not used.

I3Station
---------

The ``I3Station`` class was created to help compact ``I3StationGeo``
objects.

Because of their vector nature, ``I3StationGeo`` is difficult
to compress.  Therefore, we get rid of the vector and replace it with
a fixed object.  Each station only has 2 tanks, and many of the values
are the same between tanks.  A further optimization brings these
identical values up one level to the station to prevent duplicate values
from being serialized.

Compared to an ``I3FixedPositionVectorDiff<I3StationGeoDiff,I3StationGeo>``,
this method saves on average 1000 bytes uncompressed, 650 bytes compressed
using gzip.  This doesn't sound like a lot, but it would increase total 
GCDDiff size by 2x.  Much of the savings comes from bringing the values up 
a level to avoid serializing a value twice, and removing the vector
container serialization, especially the size of the vector.

Removing the vector from the dataclass code should be considered
in the future.

