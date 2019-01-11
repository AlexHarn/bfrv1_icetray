#!/usr/bin/env python
#
# @copyright (C) 2015 The IceCube Collaboration
# 
# @author Kevin Meagher

"""
Test paraboloid fit params's tablio converter
"""

from pprint import pprint
import sys,os.path,csv
import shutil
import unittest
from I3Tray import *
from icecube.icetray import I3Units
from icecube import icetray,dataclasses,dataio,tableio,paraboloid

filelist = [ os.path.join(os.environ["I3_TESTDATA"],"sim",
                          "Level3_nugen_numu_IC86.2012.011069.000000_20events.i3.bz2")]
csv_dir_name = "tableout"

err1 = []
err2 = []

tray = I3Tray()
tray.AddModule("I3Reader","reader",
               FileNameList  = filelist
               )

def f(frame):
    err1.append(frame['MPEFitParaboloidFitParams'].pbfErr1)
    err2.append(frame['MPEFitParaboloidFitParams'].pbfErr2)
tray.Add(f)

tray.Add(tableio.I3TableWriter,
         tableservice = [tableio.I3CSVTableService(csv_dir_name)],
         keys = ['MPEFitParaboloidFitParams'],
         SubEventStreams = ['Final'],
)

tray.Execute()

del tray

csv_filename = os.path.join(csv_dir_name,"MPEFitParaboloidFitParams.csv")
with open(csv_filename, 'r') as csvfile:
    reader = csv.reader(csvfile)

    header = next(reader)
    i1 = header.index('err1 [radian]')
    i2 = header.index('err2 [radian]')

    next(reader)
    table = list(zip(*(row for row in reader)))

    for i in range(len(err1)):
        assert(abs(err1[i]-float(table[i1][i]))<1e-13)
        assert(abs(err2[i]-float(table[i2][i]))<1e-13)

shutil.rmtree(csv_dir_name)
    

