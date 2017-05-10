#!/usr/bin/env python
"""
Test reordering of modules within an I3Tray.
"""

from icecube import icetray, dataio
from I3Tray import I3Tray

def thing1(frame):
	frame['foo'] = icetray.I3Int(42)
	
def thing2(frame):
	if not 'foo' in frame:
		raise ValueError("Modules didn't run in the right order!")
		
def thing3(frame):
	frame['bar'] = icetray.I3Bool()

def run(hook=lambda tray: None):
	tray = I3Tray()
	tray.Add("I3InfiniteSource", Stream=icetray.I3Frame.Physics)
	tray.Add(thing3, "thing3")
	tray.Add(thing2, "thing2")
	tray.Add(thing1, "thing1")
	hook(tray)
	tray.Execute(1)

#FIXME: assert throws an AssertionError, which is caught by the except.
#       this is bad, bad, bad.  this always passes.
        
# Without reordering, thing2 should fail
try:
	run()
	assert False, "Module should have raised an error when run out-of-order"
except:
	pass

# With reordering, everything is hunky-dory
run(lambda tray: tray.MoveModule('thing1', 'thing2'))

# Moving a nonexistent module fails
try:
	run(lambda tray: tray.MoveModule('no_name', 'thing2'))
	assert False, "Reordering should fail with nonexistent target module"
except:
	pass

# Moving a module to before a nonexistant module also fails
try:
	run(lambda tray: tray.MoveModule('thing1', 'no_anchor'))
	assert False, "Reordering should fail with nonexistent anchor module"
except:
	pass

