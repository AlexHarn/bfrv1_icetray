.. _gcdserver:

gcdserver
=================

gcdserver is a set of tools for importing data into the MongoDB
GCD database and retrieving that data to build I3Geometry,
I3Calibration, and I3DetectorStatus objects.  Each supported data type
has a specific import script.  Supported data types include DOMCal,
VEMCal, snow depth, and baseline XML files, noise rate JSON files,
and run configuration XML files.  Pymongo, zmq, and pyzmq are required
dependencies for running import scripts and tests.

gcdserver consists entirely of Python code and does not need compilation.
Tests are run assuming the availability of a mongoDB server on 'localhost'

Resources
^^^^^^^^^
Note: Due to security restrictions on the MongoDB server at WIPAC, most of
these tools must be run either inside the WIPAC network or on SPS/SPTS.  To
run on SPS/SPTS, you must include
'--dbhost=dbs --i3livehost=i3live --i3mshost=i3ms' in your command-line
arguments.

BuildGCD.py: Build a GCD file using the gcdserver database for the specified
run.

DBDaemon.py: Daemon that listens for GCD database updates from I3MS and
adds them to the MongoDB database at WIPAC

GCDDiff.py: Compare two GCD files and list the differences

I3OmDbDump.py: Import initial geometry/calibration data from I3OmDb database
tables into gcdserver

I3OmDbRunImport.py: Import calibration data for a specific run into the
gcdserver database

InitializeDB.py: Create the 'omdb' database in MongoDB and add indexes.
Optionally drop existing data.

TransactionManager.py: Manage gcdserver database transactions: Options:
--list=N: List the last N transactions
--replay=i: Resend data in transaction i over I3MS
--rollback=i: Rollback transaction i in the local database.  Note that
rollbacks are not synchronized over I3MS and must be done seperately at
WIPAC and at SPS

.. toctree:: 
   :titlesonly: 

   code_review
   release_notes
