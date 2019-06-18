#!/usr/bin/env python

import os
import shutil
import tempfile
import unittest

from I3Tray import I3Tray
from icecube import icetray,dataio,dataclasses
from icecube.frame_object_diff import segments

def cmp(obj1,obj2):
    """Compare two frame objects"""
    def cmp_helper(attr,o1,o2):
        a1 = getattr(o1,attr)
        a2 = getattr(o2,attr)
        if a1 != a2:
            if hasattr(a1,'keys'):
                key_diff = set(a1.keys())^set(a2.keys())
                if key_diff:
                    print('key diff',key_diff)
                else:
                    print('keys the same')
                    for i,k in enumerate(a1.keys()):
                        v1 = a1[k]
                        v2 = a2[k]
                        if v1 != v2:
                            print(i,k,v1,v2)
                            break
                    else:
                        print('values the same')
            raise Exception(attr+' is not equal')
    if obj1 != obj2:
        # try to give more specific reasons why not equal
        if isinstance(obj1,dataclasses.I3Geometry):
            for a in ('omgeo','stationgeo','start_time','end_time'):
                cmp_helper(a,obj1,obj2)
        elif isinstance(obj1,dataclasses.I3Calibration):
            for a in ('dom_cal','vem_cal','start_time','end_time'):
                cmp_helper(a,obj1,obj2)
        elif isinstance(obj1,dataclasses.I3DetectorStatus):
            for a in ('dom_status','trigger_status','start_time','end_time'):
                cmp_helper(a,obj1,obj2)
        else:
            print(fr1[k])
            print(fr2[k])

        raise Exception(k+' is not equal')

class SegmentTests(unittest.TestCase):
    def setUp(self):
        self.tmpdir = tempfile.mkdtemp(dir=os.getcwd())
    def tearDown(self):
        shutil.rmtree(self.tmpdir)

    def test_std(self):
        base = os.path.join(os.path.expandvars('$I3_TESTDATA'),
                            'GCD','GeoCalibDetectorStatus_2013.56429_V0.i3.gz')

        infile = os.path.join(os.path.expandvars('$I3_TESTDATA'),
                            'GCD','GeoCalibDetectorStatus_2013.56429_V1.i3.gz')

        outfile = os.path.join(self.tmpdir,'out.i3.gz')
        outfile2 = os.path.join(self.tmpdir,'out2.i3.gz')

        # compress file
        tray = I3Tray()
        tray.Add('I3Reader',Filename=infile)
        tray.Add(segments.compress,
                 base_filename=base)
        tray.Add('I3Writer',Filename=outfile,
                 Streams=[icetray.I3Frame.Geometry,
                          icetray.I3Frame.Calibration,
                          icetray.I3Frame.DetectorStatus])
        tray.Execute()
        
        del tray

        # uncompress file
        tray = I3Tray()
        tray.Add('I3Reader',Filename=outfile)
        tray.Add(segments.uncompress)
        tray.Add('I3Writer', Filename=outfile2,
                 Streams=[icetray.I3Frame.Geometry,
                          icetray.I3Frame.Calibration,
                          icetray.I3Frame.DetectorStatus])
        tray.Execute()
        

        # compare
        frames1 = [fr for fr in dataio.I3File(infile) if fr.Stop != fr.TrayInfo]
        frames2 = [fr for fr in dataio.I3File(outfile2) if fr.Stop != fr.TrayInfo]
        for i,(fr1,fr2) in enumerate(zip(frames1,frames2)):
            try:
                for k in fr1.keys():
                    cmp(fr1[k],fr2[k])
            except Exception:
                print('exception in frame',i)
                raise

    def test_inline(self):
        infile = os.path.join(os.path.expandvars('$I3_TESTDATA'),
                            'GCD','GeoCalibDetectorStatus_2013.56429_V1.i3.gz')
        #infile = os.path.join(os.path.expandvars('$I3_BUILD'),
        #                      'GCD_3.i3.bz2')

        outfile = os.path.join(self.tmpdir,'out.i3.gz')
        outfile2 = os.path.join(self.tmpdir,'out2.i3.gz')

        # compress file
        tray = I3Tray()
        tray.Add('I3Reader',Filename=infile)
        tray.Add(segments.inline_compress)
        tray.Add('I3Writer',Filename=outfile,
                 Streams=[icetray.I3Frame.Geometry,
                          icetray.I3Frame.Calibration,
                          icetray.I3Frame.DetectorStatus])
        tray.Execute()
        
        del tray

        # uncompress file
        tray = I3Tray()
        tray.Add('I3Reader',Filename=outfile)
        tray.Add(segments.inline_uncompress)
        tray.Add('I3Writer', Filename=outfile2,
                 Streams=[icetray.I3Frame.Geometry,
                          icetray.I3Frame.Calibration,
                          icetray.I3Frame.DetectorStatus])
        tray.Execute()
        

        # compare
        frames1 = [fr for fr in dataio.I3File(infile) if fr.Stop != fr.TrayInfo]
        frames2 = [fr for fr in dataio.I3File(outfile2) if fr.Stop != fr.TrayInfo]
        for i,(fr1,fr2) in enumerate(zip(frames1,frames2)):
            try:
                for k in fr1.keys():
                    cmp(fr1[k],fr2[k])
            except Exception:
                print('exception in frame',i)
                raise

unittest.main()
