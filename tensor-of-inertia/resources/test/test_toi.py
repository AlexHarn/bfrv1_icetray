#!/usr/bin/env python

import os.path
from I3Tray import I3Tray
from icecube import dataclasses,tensor_of_inertia
import unittest

zeniths =  [ 1.46785093641 ,0.185953490066,0.607734289848,1.03230390476,1.27041961654,
             0.799268598939,0.615935625933,2.47937442911,0.465194777188,2.41337416324]
azimuths = [ 0.348498841031,2.42761779284,1.67188673136,3.29613576917,0.571338735367,
             5.66559199078,3.80024573484,5.67930313967,3.49664240905,3.6278461487]
xs       = [ -236.136906345,-248.450191416,60.5295832788,134.746163123,71.4415412104,
             81.2465228493,-227.488753839,-204.719821537,23.4010495136,-125.037827498]
ys       = [-42.8730447423,-77.306736164,-158.286969707,81.1920010829,94.3211669448,
            -175.250158379,-301.255921053,94.5130677371,165.754486927,65.3849332545]
zs       = [-37.011732213,33.7893356253,-85.9255388242,257.133929508,36.8389403737,
            55.4441930335,309.264282733,223.822047104,80.4037877728,-86.8289612874]
times    = [11575.3743974,10618.8615334,11349.0533906,11451.1849709,11392.5789399,
            10858.8605568,10977.6570998,10455.8126577,11591.7131498,12269.389504]

def d(frame,s):
    s.frame_no
    s.assertAlmostEqual(frame['ti'].dir.zenith,zeniths[s.frame_no])
    s.assertAlmostEqual(frame['ti'].dir.azimuth,azimuths[s.frame_no])
    s.assertAlmostEqual(frame['ti'].pos.x,xs[s.frame_no])
    s.assertAlmostEqual(frame['ti'].pos.y,ys[s.frame_no])
    s.assertAlmostEqual(frame['ti'].pos.z,zs[s.frame_no])
    s.assertEqual(frame['ti'].shape,dataclasses.I3Particle.Cascade)
    s.assertEqual(frame['ti'].fit_status,dataclasses.I3Particle.OK)
    s.frame_no+=1

class TestStringMethods(unittest.TestCase):

    frame_no = 0
    
    def test_split(self):
        tray = I3Tray()

        I3_TESTDATA=os.environ['I3_TESTDATA']
        gcd_file = os.path.join(I3_TESTDATA,"GCD","GeoCalibDetectorStatus_2012.56063_V0.i3.gz")
        input_file = os.path.join(I3_TESTDATA,"sim","Level2_IC86.2011_corsika.010281.001664.00.i3.bz2")
        
        tray.AddModule("I3Reader", FileNameList = [gcd_file,input_file] )
        
        tray.AddModule("I3TensorOfInertia",
                       InputReadout = "OfflinePulses")
        
        tray.AddModule(d,s=self)

        tray.Execute(24)

if __name__ == '__main__':
    unittest.main()



