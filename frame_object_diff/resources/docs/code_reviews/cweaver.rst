Code Review 1
=============

.. highlight:: c++

Involved Parties
----------------

* Author: David Schultz
* Sponsor: David Schultz/Claudio Kopper?
* Consumer: Claudio Kopper
* Reviewer: Chris Weaver

Version Reviewed
----------------
* URL: http://code.icecube.wisc.edu/svn/projects/frame_object_diff/trunk
* Revision: 130491

Documentation
-------------

Some RST documentation is provided. 

modules.rst and segments.rst seem to be missing their actual content. 

It might be good to write down some of the rationale for desgin decisions in this code, for instance, why this project is separate from dataclasses. 

C++ classes should have proper doxygen documentation, instead of or in addition to RST. 

Tests
-----

A limited set of tests are included, currently only a unit test for `I3Station`, and and integration test for the overall process of generating and applying a diff. The breadth of testing should be expanded. 

test_segments.py appears to be left in a development state. It would be more useful as a correctness test if it took the difference between two files which actually differ, or if it did that as well. 

Code Comments
-------------

pack_helper.hpp: Could use some comments on what the various type traits and functions are for. 

I3MapDiff.h, I3VectorDiff.h: Should have comments about the general methods and the meanings of the variables. These aren't terribly hard to figure out from reading the code, but that shouldn't be necessary. 

I3Station.h: These classes don't seem to belong here; they look like they should be in dataclasses itself if they offer real benefits over what is currently there. The `I3Station` class seems strange to me, however. Why is `tankradius` a property of a station, not of the tanks? I can imagine that in practice all tanks have the same radii, but if this is something we need to track at all, it seems like it should be tracked on a per-tank basis as it is in the current `I3TankGeo`. The same concern applies all of the tank properties in `I3Station`. 

I3DOmCalibrationDiff.h/.cxx: This class should use the proper accessor functions of I3DOMCalibration. The `friend` declaration in `I3DOMCalibration` should be removed. 

external/lcs.hpp: The second referenced link seems to be dead. 

General Code Comments
---------------------

Some types have split serialization which don't appear to need it: I3CalibrationDiff, I3DetectorStatusDiff, I3GeometryDiff

Several of the difference types (`I3DOmCalibrationDiff` and `I3TankGeoDiff`) appear to simply have all or most of the same member variables as the types of which they are intended to represent differences. Is there any reason that these types difference types shouldn't just contain and instance of the original type to use as a data container? This would only be used in-memory, with the serialization for the difference type continuing to work as it does now. 

In a lot of places hard-coded indices are used for bitsets. The places where this happens tend to have very simple and obvious structures, such that I can't see much of a way for this to lead to correctness issues, but every time I see it it sets me on edge. It doesn't seem unduly difficult to define private enums within the classes which do this for the particular bit positions, i.e.::

    enum{ PositionBit=0, OrientationBit, RadiusBit, HeightBit, 
          FillBit, KeyListBit, SnowHeightBit, TankTypeBit };

Perhaps this concern is overly paranoid, however; I could accept the code being left as it is. 

A lot of this code is also quite verbose. The following suggestions are both stylistic and tentative, so feel free to ignore them. 

Personally, I would choose to write::

    if (bits[0])
      unpacked_->position = position_;
    else
      unpacked_->position = base.position;

as::

    unpacked_->position = (bits[0] ? position_ : base.position);

Likewise, for things like::

    if (CompareFloatingPoint::Compare_NanEqual(base.orientation,
              cur.orientation))
      bits[1] = 0;
    else
      orientation_ = cur.orientation;

I would try things like::

    using CompareFloatingPoint::Compare_NanEqual;

    // . . .

    bits[1] = !Compare_NanEqual(base.orientation, cur.orientation);
    if (bits[1])
      orientation_ = cur.orientation;

    // more compactly
    if ((bits[1] = !Compare_NanEqual(base.orientation, cur.orientation)))
      orientation_ = cur.orientation;

    //or, if copies are cheap
    bits[1] = !Compare_NanEqual(base.orientation, cur.orientation);
    orientation_ = cur.orientation;
