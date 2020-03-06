====================
Code review of rpdf
====================

Reviewer: J. Braun
SVN Revision: 162804
Date: May 29, 2018

Overview
--------
rpdf was created by Kevin Meagher to address code quality issues with ipdf.
The issues were presented by Kevin:

https://drive.google.com/drive/folders/1Vcr4x9Jmh8EaNeZYuT8Wm2dubaIBo6bG

rpdf consists mostly of I3RecoLLH, a new gulliver service for reconstructions using
Pandel likelihoods, and algorithms copied mostly verbatim from ipdf.

Overall Impression
------------------

The code in rpdf represents a major improvement over ipdf. A few minor issues need
to be addressed before rpdf is used in standard processing:

- Unit tests must be written (see below)
- Speed and accuracy should be tested against ipdf and documented
- Where appropriate, references to ipdf should be placed in the code so that users
  can find the SVN history

Project Metrics
---------------

Files (including directories):    

::
 
  rpdf:    33
  ipdf:    119

Lines of code:

::

                 rpdf    ipdf
                 ----    ----
           .h:    576    6966
         .cxx:   1196    4980
          .py:    206     174
         --------------------
        total:   1978   12120

Lines of .rst documentation: 

::

  rpdf:   566
  ipdf:   653

Unit tests: 

:: 

  rpdf: 0
  ipdf: 79

Comments: rpdf contains only 15% of the lines of code contained in ipdf.  If
rpdf satisfies all current usage of ipdf, this is a significant success.
Documentation and unit tests will be discussed below.

Documentation
-------------

The documentation in rpdf is much more thorough than the previous documentation
in ipdf.  The improvements include background on likelihood reconstruction,
derivation on geometrical calculations, motivation for the Pandel PDF,
description of the convoluted Pandel PDF, and images from the AMANDA
reconstruction paper and convoluted Pandel paper.


Unit Tests
----------

rpdf contains a single python file in resources/test that compares ipdf
reconstruction results against rpdf.  The file is not automatically run by
'make test'.  In contrast, ipdf contains 79 unit tests. While many of these
tests may not be relevant for rpdf, I believe at least some minimum test
coverage is warranted before replacing ipdf.


Examples
--------

rpdf/resources/examples contains two Python files. python_I3RecoLLH.py
demonstrates how to obtain PDF values from rpdf, and
python_likelihood_service.py demonstrates how to use rpdf in an IceTray
module and how to create a new gulliver likelihood service.  These
examples are sufficient in my view.

Code
----

The code is high quality and  much cleaner than that of ipdf.  ipdf uses CRTP, with
child classes containing the PDF implementation and access to the PDF done through the
base class, whereas rpdf stores a pointer to the selected PDF function.  Additionally,
ipdf uses a template parameter for the ice model, whereas rpdf uses a struct.
Favoring composition over templates and an inheritance hierarchy greatly improves
the readability of the code.  Additionally, the removal of old PDFs we no longer
use is welcome.

The structure of the project is straightforward.  Inline comments and method/class
comments are generally very good.  Issues with individual files are generally
stylistic.  I recommend running a spelling check.

Issues with specific files:

private/rpdf/I3RecoLLH.cxx:
- Inconsistent tabs/spaces used in indentation
- Indented braces in 'for' blocks is strongly discouraged

private/rpdf/I3RecoLLHFactory.cxx:
- Inconsistent tabs/spaces used in indentation
- Indented braces are strongly discouraged.  The following:

.. code-block:: c++

  if (type=="IceModel")
    {
      ice_model_=boost::python::extract<rpdf::IceModel>(icemodel);
    }

should be either

.. code-block:: c++

  if (type=="IceModel")
  {
    ice_model_=boost::python::extract<rpdf::IceModel>(icemodel);
  }

or

.. code-block:: c++

  if (type=="IceModel") {
    ice_model_=boost::python::extract<rpdf::IceModel>(icemodel);
  }

private/rpdf/pandel.cxx:

- Inconsistent tabs/spaces used in indentation
- Many stylistic issues in this file that are carried over from ipdf.  Not all
  of these need to be listed or fixed, but at least the missing spaces should
  be fixed, as this impacts readability.
- References to ipdf should be placed in the code so that users can find the SVN
  history
