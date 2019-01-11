Code Review
===========

.. highlight:: c++

	       

By Javier Gonzalez on r153209.
http://code.icecube.wisc.edu/projects/icecube/browser/IceCube/projects/ddddr/trunk?rev=r153209

This project includes two modules to estimate a muon bundle's energy
loss profile. One for data, which requires input track and a pulse
map, and one for MC, which requires and I3MCTree and an
MMCTrackList. It also provides frame objects to store its results. It
is one of the alternative bundle reconstructions used in coincidence
with IceTop.

Documentation
-------------

The RST documentation covers the main modules well. Further detail
about the algorithm is linked from within. The project also contains a
few example scripts and provides one segment. **Some description of
them would be good**. Most of the C++ classes are documented using
doxygen.

The output of icetray-inspect seems ok.

The list parameters in the RST documentation of I3TrueMuonEnergy is
not complete.

Scripts/Examples
----------------

Four scripts are included:
 * me_data_reco.py: Example usage of the I3MuonEnergy module
 * me_mc_true_reco.py: Example usage of the I3TrueMuonEnergy module
 * true_and_reco_segment.py: Example usage of the TrueAndRecoMuonEnergy segment (combination of the previous two)
 * plot_events.py: Produces plots of the energy loss profile

All scripts use optparse.OptionParser, so one expects the -h options
is reasonable and that they run out of the box.

Issues:
 * The examples depend on I3_PORTS being defined to find test data. In
   general, the test data directory is not guaranteed to exist, and if
   it exists is not guaranteed to be in I3_PORTS. Better to try
   I3_TESTDATA and fail gracefully.
 * The examples do not fail gracefully if the input (GCD and other files) do not exist.
 * plot_events.py might have some issues. It only does actual plotting
   if one of two options is passed (that was unexpected, given the
   name of the script). It saves figures in the scripts source directory as
   opposed to CWD. When I ran on the default test data, I got 13 empty
   plots.

Unit tests
----------

* The only class that is unit-tested is I3MuonEnergyProfile (This test
  passes). Apart from the modules, most classes are plain structs, so probably this is not a big issue.
* Some validation test scripts or unit tests of the modules would be good.


Source code
-----------

Directory structure
"""""""""""""""""""

The directory structure conforms to the standard. There is a directory
resources/tables, and it seems that our standards do not specify any
particular name for directories containing data (data/tables/etc).

**Should we specify this location in the coding standard?**

Code structure
""""""""""""""

The structure of this project is fairly simple. The use of each function and class is clear enough.

Modules
 * I3MuonEnergy
 * I3TrueMuonEnergy
Frame objects:
 * I3MuonEnergyCascadeParams
 * I3MuonEnergyParams
 * I3MuonEnergyProfile
Minuit2 classes
 * MuonEnergyFCNBase
 * MuonEnergyMinuit2
 * ExpoFcn
 * ExpoFunction
 * TomFFcn
 * TomFFunction
 * FitParameterSpecs
 * MinimizerResult
Utilities:
 * MuonGunTrack
 * MuonEnergyFunctions

Issues:

 * Some code has been copied from other projects. These are classes in the public interface
   of the projects, so I see no reason why they should be copied here:
 * FitParameterSpecs.h is a copy of gulliver's I3FitParameterInitSpecs.h
 * MinimizerResult.h is a copy of I3MinimizerResult.h
 * MuonGunTrack.cxx/h are copies of MuonGun's Track.cxx/h (MuonGun is not in IceRec)

 * This project defines two frame object classes:
    * I3MuonEnergyParams and
    * I3MuonEnergyCascadeParams.

 Both classes have their corresponding tableio converters and python
 bindings. **We could consider moving them into recclasses**, but their
 names could be too general and might create confusion when/if this is
 done.

Small issues:

 * There is an empty LinkDef.h. I suppose that is dead code and should be removed.
 * There is I3TrueMuonEnergy::getMedianOfVector and
   I3MuonEnergy:medianOfVector. Their name would imply they do the
   same thing, but the first one behaves differently if entries are
   zero. I suppose this is because in "data" there is always a
   non-zero entry. Maybe document this.
 * trivial non-default destructors (maybe remove it?):

    * FitParameterSpecs


Use of ROOT
'''''''''''

This project actually requires ROOT and this is not documented
anywhere. If ROOT is not present (#ifndef I3_USE_ROOT), the results
differ and the user is never notified. This is true for I3MuonEnergy
and I3TrueMuonEnergy. Shouldn't it not be compiled if root is not
present??? Or am I missing something?


Use of exceptions
'''''''''''''''''

In I3MuonEnergy.cxx:440, exceptions are used to control the flow. This
is a misuse of exceptions (it is normal in Python, but frowned upon in
C++). For example::

	const I3RecoPulseSeries pulses = inIcePulsesMap->find(omkey)->second;
	try
	{
		data_of_this_dom.time = pulses.at(0).GetTime();
	}
	catch(const std::out_of_range& oor)
	{
		data_of_this_dom.charge = 0;
		data_of_this_dom.time = 0;
		data_of_this_dom.dEdX = 0;
	}

Exceptions are time-consuming when compared to an `if` statement to
check whether there is an I3RecoPulse. In most cases, exceptions
should be used only for circumstances that occur infrequently and are
not expected.

I admit, this is not a particularly bad case. A bad case is when
an exception causes a break out a recursion loop, as would
happen if there are complex statements within the `try`
block. Still, I think this should be fixed.

Use of namespaces
'''''''''''''''''

Avoid polluting the namespace. Functions with common names, such as the following should go in a namespace:
 * EarlierThan
 * smallerThan
 * geometricDistance
 * medianOfVector

It is enough to enclose them in an unnamed namespace if they are only
used in a single implementation file (like the ones just mentioned)::

  namespace {
    ...
  }

Otherwise, you might encounter name clashes when linking.

The only namespace in the project is MuonEnergyFunctions (in
MuonEnergyFunctions.h/cxx). The rest is on the top level.

Use of pointers
'''''''''''''''

I looked at the uses of pointers and at the uses of the 'new'
keyword. All pointers are boost::shared_ptr. Most uses correspond to
frame objects that get read or written on the frame. The other use of
pointers is for Minuit2 fit functions.

* Code like this (from I3MuonEnergy.cxx:855)::

    I3ParticlePtr fittrack(new I3Particle(*(frame->Get<I3ParticleConstPtr>(trackName_))));
    return fittrack;

  is easier to read and there is no intermediate copy if replaced by::

    return *frame->Get<I3ParticleConstPtr>(trackName_);

* Also the following might not be necessary. Copying a pointer that was
  just read from the frame defeats the purpose of having pointers in the
  first place, which is that a copy of the object is not made::

    domCal_ = I3DOMCalibrationMapPtr(new I3DOMCalibrationMap(calibration->domCal));
    omGeo_ = I3OMGeoMapPtr(new I3OMGeoMap(geometry_->omgeo));

  One option is to get rid of ``omGeo_`` and ``domCal_``, and just type
  ``geometry_->omGeo_`` or ``calibration_->domCal_`` whenever they are
  needed. Another option is to use something like ``boost::optional<const
  I3OMGeoMap&>``.

* The code in I3MuonEnergy.cxx:323 is never executed, since ``badDomList_`` is always set::

    if(!badDomList_)
    {
      ...
    }

Coding standards
""""""""""""""""

Prefer writing nonmember nonfriend functions
''''''''''''''''''''''''''''''''''''''''''''

* I3MuonEnergyParams and I3MuonEnergyCascadeParams are basically
  structs with only public data members. No need to declare any friend
  functions/classes (but they are declared).


Avoid magic numbers
'''''''''''''''''''

* SURFACE_HEIGHT in I3TrueMuonEnergy.cxx and I3MuonEnergy.cxx could be
  replaced by I3Constants::SurfaceElev and I3Constants::OriginElev
* CASCADE_CONTAINMENT_DISTANCE in I3MuonEnergy. Why can't it be a
  configurable parameter of the module?
* There are a few starting values for the fit parameters in
  I3MuonEnergy.cxx. At least they are at the top and clearly visible.
  I think it is ok to leave them.


I3 prefix
'''''''''

Classes that do not derive from an Icetray base class should not have the
I3 prefix:

 * I3MuonEnergyProfile (was this intended as a frame object?)


Make header files self-sufficient
'''''''''''''''''''''''''''''''''

* Headers can be improved. One should include the minimum necessary in header files (from Sutter&Alexandrescu: "don't
  include headers that you don't need; they just create stray
  dependencies"). For example, the 19 include lines in I3TrueMuonEnergy
  can be replaced by::

    #include "icetray/I3ConditionalModule.h"
    #include <icetray/I3Logging.h>
    #include <string>
    #include <vector>

    I3_FORWARD_DECLARATION(I3Particle);
    I3_FORWARD_DECLARATION(I3Geometry);
    I3_FORWARD_DECLARATION(I3MuonEnergyParams);
    I3_FORWARD_DECLARATION(I3MuonEnergyCascadeParams);


* The following header file does not compile on its own (undefined NAN):
   * MinimizerResult.h


Use const proactively
'''''''''''''''''''''

This is not in our standards currently, but it's in chapter 15 in Sutter & Alexandrescu.
All these should be const:

 * I3MuonEnergyParams::Dump
 * I3MuonEnergyCascadeParams::Dump
 * I3MuonEnergyProfile::medianOfVector, I3MuonEnergyProfile::FindBin and all I3MuonEnergyProfile::Get*
 * MuonEnergyFCNBase::Up, MuonEnergyFCNBase::measurements, MuonEnergyFCNBase::positions (and in all derived classes)
 * ExpoFunction::N, ExpoFunction::b
 * I3TrueMuonEnergy::get*, etc...


Readability
"""""""""""

The code seems readable to me. I would capitalize function names in C++ but we are not supposed to sweat the small stuff.

* For the python parts, `pep8`_ specifies lowercase with underscores for modules,
  functions and variables, and camelcase for classes. All functions are camelcase.

.. _pep8: https://www.python.org/dev/peps/pep-0008/#prescriptive-naming-conventions

Usability
"""""""""

The parameter list seems fine to me.


Compiler warnings
"""""""""""""""""

.. code-block:: bash

    /Users/javier/Work/IceCubeSoftware/combo/trunk/src/ddddr/private/ddddr/MuonGunTrack.cxx:115:1:
    warning: unused function 'operator!='

    /Users/javier/Work/IceCubeSoftware/combo/trunk/src/ddddr/private/ddddr/MuonEnergyMinuit2.h:59:7:
    warning: private field 'minuitPrintLevel_' is not used


Conclusions
-----------

The project seems fine to me, and the issues I found are minor. This
project actually requires ROOT and it is not clear whether ROOT is
required for production. Documentation needs some fixing (example's
documentation, module's parameters in RST
documentation). plot_events.py example did not work for me. There is
some code replication from gulliver and MuonGun. The rest are
coding/documentation details that can be improved.

Now would be the time to decide whether the names of classes that
could end up in recclasses should be changed (I3MuonEnergyParams and
I3MuonEnergyCascadeParams).

This project should be included in IceRec. I think most of the issues
here are not required for a release. What I would consider before a
release would be whether to change classe names, fix the broken
example, and the RST documentation fixes.
