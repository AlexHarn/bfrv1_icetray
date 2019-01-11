from I3Tray import *
from icecube.icetray.test_module import I3TestModuleFactory
from icecube import tensor_of_inertia, dataclasses
import unittest

def ENSURE_DISTANCE(measured,expected,distance,message):
	if abs(measured - expected) > distance :
		print(message)
		print("measured = %s expected = %s distance = %s" % \
		      (str(measured),str(expected),str(distance)))
		return False
	return True

def ENSURE(cond,message):
	if not cond :
		print(message)
		return False
	return True

class TOITestAMA(unittest.TestCase):
    def testRecos(self):
        ti = self.frame.Get("ti")
        params = self.frame.Get("tiParams")
        self.assertTrue(ENSURE_DISTANCE(ti.pos.x, -48.472, 20, "X failed"))
        self.assertTrue(ENSURE_DISTANCE(ti.pos.y, 6.142, 20, "Y failed"))
        self.assertTrue(ENSURE_DISTANCE(ti.pos.z, 5.567, 20, "Z failed"))
        self.assertTrue(ENSURE_DISTANCE(params.evalratio, 0.03, 0.01, "evalratio failed"))

class TOITestIC23(unittest.TestCase):
    def testRecos(self):
        ti = self.frame.Get("ti")
        params = self.frame.Get("tiParams")
        self.assertTrue(ENSURE_DISTANCE(ti.pos.x, -142.1224, 1, "X failed"))
        self.assertTrue(ENSURE_DISTANCE(ti.pos.y, 15.746, 1, "Y failed"))
        self.assertTrue(ENSURE_DISTANCE(ti.pos.z, 349.839, 1, "Z failed"))
        self.assertTrue(ENSURE_DISTANCE(params.evalratio, 0.0216, 0.001, "evalratio failed"))
        self.assertTrue(ENSURE_DISTANCE(params.mineval, 323.744, 0.01, "mineval failed"))
        

