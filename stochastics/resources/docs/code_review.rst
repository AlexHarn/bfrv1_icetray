Code Review
===========

.. highlight:: c++


* URL: http://code.icecube.wisc.edu/svn/sandbox/sderidder/scripts/inice_scripts/stochastics
* Maintainer: Sam De Ridder
* Reviewer: Javier Gonzalez
* revision: 154471, http://code.icecube.wisc.edu/svn/sandbox/sderidder/scripts/inice_scripts/stochastics?rev=r154471


This project has basically one module that does one fit.  The purpose
of this project is to make a fit to the energy loss profile of a muon
bundle (like the one coming from millipede). It is a very simple
project. The main issue that comes up as soon as you see it is that
there is absolutely no documentation. Other than that, things seem to
be ok, considering how small this project is. Upon seeing it, I
wondered whether this project should stand alone or be merged into
another project.

**This project requires root, but in cmake the build is not conditional.**

Documentation
-------------

Documentation is absolutely non-existent. None... zero... zip...  No
reference to the algorithm.  There are no examples. There are no unit
tests.  The formula used to fit is in a comment in
StochLikelihood.cxx:53 and there are some other clues interspersed in
the code.

A: Added documentation, examples will follow. No unit tests yet. 

Scripts/Examples
----------------

None.

Unit tests
----------

No unit tests.

Source code
-----------

The directory structure is still not the standard svn
(trunk/branches/releases/candidates). This would be just the trunk.
The source code is structured according to our standards. Most is C++.

A: Done

Python code
"""""""""""

There is one python module called PlotStoch. Some issues:

* What is the relation between the member function
  PlotStoch._bundleEloss and the function in StochLikelihood.cxx? This
  can cause trouble if they should agree.
* It creates a file test.root in the working directory. This could be
  configurable or choose a name that is not so common. Also make sure
  the file is removed in the end. Is it even necessary???
* It only creates plots for the first nEvents passed. The rest are
  ignored. Well... as long as it is documented...

A: All this is fixed. 

C++ Code
""""""""

The C++ code includes one module (I3Stochastics) and one frame object
(I3EnergyLoss), including the corresponding python bindings and tableio
converter.  I3Stochastics is a mostly the Physics method, but not too
large though (a couple hundred lines). The only extra functions are the ones
passed to Minuit.

Some issues:

* **MEMORY LEAK: fit_eloss_minuit_ is allocated with new and never deleted.**
  (unless root does the cleaning, which I doubt).
* I3Stochastics has an empty constructor and Finish methods. Not needed.
* in I3Stochastics.cxx:260 it silently eats the frame. This happens if evEloss < 0.1.
  There is a comment that says "DO SOMETHING". Seems like a TODO.
* some dead code in StochLikelihood.cxx:66?
* finite (from math.h) has been deprecated. Use std::isfinite. (coding standard: compile at high warning level)
* StochLikelihood.cxx unused parameters nPar, grad, iflag. (coding standard: compile at high warning level)

A: Memory leak fixed; constructor, finish removed; 
I do not see that the frame is eaten, it just spits out a comment; Dead code is removed; finite solved; 
Unused parameters in StochLikelihood are needed because of inheritance of TMinuit. (SetFCN)

Readability
"""""""""""

The code itself is simple to understand.

Usability
"""""""""

output of icetray-inspect?

**FreeParams is a bit mask.** A list of bools is maginally better.
Its default value is 1, but the comment says "[000011]", leading one to believe that A and E0 are, but no: a value of 1 means only E0 is free (I3Stochastics.cxx:71).

A: I agree that this is not the nicest coding, and would agree with booleans. Not done yet however (wondering whether this should just be fixed in the next version). 

Potential sources of error
""""""""""""""""""""""""""

As mentioned above: missing root, memory leaks, missuse of bit mask.


Conclusions
-----------

The purpose of this project is to make a fit to the energy loss
profile of a muon bundle (like the one coming from millipede).  If
there is no other project with similar functionality, this project can
stand alone.  **The name of the project is very generic. Do people
object to it?**

This project definitely needs documentation.

All issues mentioned above should be fixed before incorporating this
project to IceRec, including fixing the cmake build to handle the case
when ROOT is not present. Please note that these changes do not affect
the results.

It seems to me that the functionality here is similar to part of
DDDDR. DDDDR even has an option to fit a TomF-type formula, which
sounds suspiciously close to this. So I would recommend that this
functionality is gathered unless there is a reason to keep them
separate.
