Reviewed by M. Frere 3/29/17
****************************

*JB: All changes have been accepted and implemented except where noted*

General notes:
**************

Documentation is not included in the codebase. If there’s a wiki page, it
should be linked in some README file, but still, it wouldn’t work without
satellite at SPS.

Quite a bit of hardcode. It’s understandable given the nature of the project.
The scripts are not likely to change much, and some won’t be reused.
Furthermore, adjusting the current behavior would be tedious for very little
value.

Unit tests are present. They take about ⅓ of the total code base for a decent
coverage

Not familiar with the different xml/other file structures so it’s hard to tell
if anything is wrong with the import procedures. Code-wise they look clean and
a few people tested the code already.  It’s probably safe to assume this part
is fine.

Maintenance should be fairly easy. Overall the code is clean and readable and
the project isn’t huge.  We could use a few more docstrings/comments even
though function and variable names are often self explanatory.

All in all, the code looks good, and none of the items/fixes mentioned below
are absolutely necessary for a successful project.


python/MongoDB.py
*****************

Pyflakes output:
MongoDB.py:20: 'icecube.gcdserver.Calibration as C' imported but unused

Line 110 -> 135
Consider using ensure_index instead of create_index if the script can be run
more than once

Line 118
The compound index on line 119 makes the index declaration at line 118
redundant (since the first field is the same)

Line 368
Consider adding a docstring to getUniqueDocumentMap

Line 387, 406, 443, ... (seen multiple times including in other files)
Could use super() when overriding, instead of re-referencing the parent class
Example with python 2.x notation: super(ChildClass, self).some_method()

Other notes: 
Additional compound indexes may provide better performance. Right now, index
intersection should prevent major slowness already when using multi-field
queries. It's still worth keeping in mind for future improvements (it's best
to optimize after building an exhaustive list of potential find() query types,
and only worth if perf impact is significant)

Aggregation: It would be interesting to know whether sorting after the match
stage brings any perf improvements (particularly on large datasets). Pretty
cheap to try out.

*JB: This should bring no improvement.  The sort should be a no-op since it
is supported by an index*


python/I3Live.py
****************

Pyflakes output:
I3Live.py:7: 'datetime.timedelta' imported but unused
I3Live.py:7: 'datetime.datetime' imported but unused

Line 49 -> 53
Fractional start/stop isn't 0 when precise time isn't provided by daq
(it just doesn't have 10th of ns precision).
Also, precise_start_available only applies for the run start time.
If using one at all, the other one (precise_stop_available) should
probably be used as well.

Line 58
Not sure if this can cause a problem downstream but stopTime will be
NULL if run is in progress.

*JB: Correct.  Downstream code will need to understand that None is
the indicator that no time was available.  I've added a comment in the code*


python/Geometry.py
******************

Line 109
Other modules extend DataObject, so it may would be beneficial to move it to
another more generic file and avoid confusion (eg: why import Geometry module
in ConfigImport.py, MongoDB.py, etc... ?)

Line 92
Same as above: DictionaryBacked is not strictly related to geometry. It could
live in some util module on the side

*JB: This touches many lines of code but gives little benefit, so I'll leave
it for now*


python/ConfigImport.py
**********************

Pyflakes output:
ConfigImport.py:5: 'datetime.datetime' imported but unused
ConfigImport.py:9: 'icecube.gcdserver.Calibration as C' imported but unused


python/I3MS.py
**************

Line 218
Function and class have the same name, except for the uppercase first letter.
This can be confusing. Same for 'status' and 'geo'.

*JB: This touches many lines of code but gives little benefit, so I'll leave
it for now*

python/I3OmDb.py
****************

Pyflakes output:
I3OmDb.py:50: undefined name 'MYSQL_TIME_FORMAT'
^ I suspect that line never got reached and is not covered by a unit test

python/NoiseRateImport.py
*************************

Pyflakes output:
NoiseRateImport.py:6: 'datetime.datetime' imported but unused

python/I3CalibrationBuilder.py
******************************

Pyflakes output:
I3CalibrationBuilder.py:2: 'datetime.datetime' imported but unused
I3CalibrationBuilder.py:5: 'icecube.icetray.OMKey' imported but unused

Line 166
Is the use of NaN a requirement for GCD instance generation? The note above
shows some uncertainty as well.

*JB: Yes, we're forced to use NaN here*

python/I3DetectorStatusBuilder.py
*********************************

Pyflakes output:
I3DetectorStatusBuilder.py:2: 'icecube.daq_decode' imported but unused
I3DetectorStatusBuilder.py:9: 'icecube.gcdserver.RunData.RunData' imported but unused

python/I3GeometryBuilder.py
***************************

Pyflakes output:
I3GeometryBuilder.py:2: 'datetime.datetime' imported but unused
I3GeometryBuilder.py:5: 'icecube.icetray.OMKey' imported but unused

python/DOMCalImport.py
**********************

Line 241
There is no input control for the runValid parameter (neither before or
further down in doInsert, CalDBInserter, etc...). If an incorrect value
gets passed (typo or such), the rollback mechanism should be enough to
clean the erroneous entries, but this may take some time. Thus, I could
see this becoming a bigger problem if this step is part of a prolonged
operation. The above comment likely applies to the other xxxImport.py
scripts.

*JB: It is valid for the user to specify documents as valid for *any*
run, so we can't have input control here*

Other notes: this file could use more comments/docstrings

resources/I3OmDbDump.py
***********************

Pyflakes output:
I3OmDbDump.py:6: 'pymongo' imported but unused
I3OmDbDump.py:122: local variable 'stringpos' is assigned to but never used

Line 19 -> 25   
Don't use octal notation (remove leading zero). The code should work as
expected in its current state, but I’ll illustrate what could go wrong below::

  >>> 07
  7
  >>> 08
    File "<stdin>", line 1
          08
 	  ^
  SyntaxError: invalid token
  >>> 011
  9
  >>> 055
  45

resources/I3OmDbRunImport.py
****************************

Pyflakes output:
I3OmDbRunImport.py:6: 'pymongo' imported but unused
I3OmDbRunImport.py:14: 'icecube.gcdserver.I3MS.geoDBInserter' imported but
unused

Line 3
Wrong file description at the top - classic copy/paste problem

Line 28  (and any other call to cursor.execute)
Given the scripts will be run by operators, the risk of sql injection is
extremely low. I'll still suggest a trivial change that eliminates the risk.
   
Current code for reference:
  

.. code-block:: python

 sql = ("SELECT * FROM CalibrationDetail where TypeID=%d AND "
        "ValidityStartDate <= '%s' order by ValidityStartDate "
        "desc limit 1" % (typeID, str(runStartTime)))
 curs.execute(sql)

Suggested: apply the following everywhere cursor.execute(..) is called:
  

Change to: curs.execute(sql, (typeID, str(runStartTime))) (and also remove the quotes around %s)

*JB: The MySQL I3OmDb will be retired shortly.  These scripts exist only
for import/testing, and they'll only be used by experts.  I am not worried
about SQL injection in this case.*

resources/BuildGCD.py

Pyflakes output:
BuildGCD.py:26: local variable 'g' is assigned to but never used
BuildGCD.py:27: local variable 'c' is assigned to but never used
BuildGCD.py:28: local variable 'd' is assigned to but never used


resources/InitializeDB.py

Pyflakes output:
InitializeDB.py:3: 'pymongo' imported but unused
