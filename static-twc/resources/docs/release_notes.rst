.. $Id$
.. $Author$
.. $Date$

Release Notes
=============

June 12, 2018 Kevin Meagher
---------------------------
Release V01-02-07

* Documentation cleanups
* Script Cleanup (remove `Finish()` and `TrashCan`

March 24 2017 Kevin Meagher (kmeagher@ubl.ac.be)
------------------------------------------------
Release V01-02-06

* Add serializaiton and python dependencies

May 2, 2016
-----------
Release V01-02-05

* Sphinxified documentation
  -- Hans Dembinski
* Added tests for the example scripts
  -- Hans Dembinski

April 8, 2014 Meike de With (meike.de.with@desy.de)
--------------------------------------------------------------------
Release V01-02-04

* Make implicit conversions to bool explicit
  -- Chris Weaver
* Add basic pybindings
  -- Laura Gladstone

January 13, 2014 Meike de With (meike.de.with@desy.de)
--------------------------------------------------------------------
Release V01-02-03

* Record time window used for cleaning
  -- Jakob van Santen

February 21, 2013 Emanuel Jacobi  (emanuel.jacobi@desy.de)
--------------------------------------------------------------------
Release V01-02-02

* Removed dependency on log4cplus
  -- Nathan Whitehorn

January 11, 2012 Emanuel Jacobi  (emanuel.jacobi@desy.de)
--------------------------------------------------------------------
Release V01-02-01

* Switch to new frame API
  -- Nathan Whitehorn
* Remove automagic frame type switching
  -- Nathan Whitehorn
* No log_fatal if input is missing
  -- Fabian Kislat

August 9, 2011 Emanuel Jacobi  (emanuel.jacobi@desy.de)
--------------------------------------------------------------------
Release V01-02-00

* Feature: Hit cleaning modules operate on both Q and P frames.
  -- Nathan Whitehorn
* Support of emitting masks
  -- Nathan Whitehorn
* Bugfix: Prevent hit-cleaning modules that emit masks from sometimes emitting
  empty pulse series instead. This restores the assumption that modules
  only emit single types.
  -- Jakob van Santen

November 23, 2010, Fabian Kislat
--------------------------------------------------------------------
V01-01-00

* Feature: STWC is now able to use times from more than one trigger ID;
  the configuration parameter TriggerConfigIDs was changed from an
  integer to a vector/list of integers to accomodate this.
  -- Marius Wallraff
* Feature: STWC can now be configured to use only the time of the first
  trigger instead of using the time of the last trigger for the end
  of the cut window.
  -- Marius Wallraff

November 06, 2010, Fabian Kislat
--------------------------------------------------------------------
Release V01-00-01

* Bugfix: Extend time window in case there are multiple same triggers
  (e.g. two SMT3).
  -- Chang Hyon Ha

October 18, 2010, Fabian Kislat
-------------------------------
Release V01-00-00
Initial release for IceRec
