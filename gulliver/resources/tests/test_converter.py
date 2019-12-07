#!/usr/bin/env python

import sys,os.path,csv
import shutil
import unittest
from I3Tray import *
from icecube import icetray,dataclasses,tableio
from icecube import gulliver

#This tests the converters to output the boostrap params into tables

csv_dir_name = "tableout"
frame_name = "TheParams"

def PutInFrame(frame):
    params = gulliver.I3LogLikelihoodFitParams()
    params.logl = 33.
    params.rlogl = 42.
    params.ndof = 55
    params.nmini = 36
    frame[frame_name] = params

    header = dataclasses.I3EventHeader()
    header.sub_event_stream = "TestStream"
    frame["I3EventHeader"] = header

tray = I3Tray()

tray.AddModule("BottomlessSource", "BS", Stream=icetray.I3Frame.Physics)

tray.Add(PutInFrame, "PutInFrame")

tray.Add(tableio.I3TableWriter, "TableWriter",
         tableservice = [tableio.I3CSVTableService(csv_dir_name)],
         keys = [frame_name],
         SubEventStreams = ["TestStream"],
)

tray.Execute(1)

del tray

csv_filename = os.path.join(csv_dir_name,frame_name+".csv")
with open(csv_filename, 'r') as csvfile:
    reader = csv.reader(csvfile)

    header = next(reader)
    ilogl = header.index('logl')
    irlogl = header.index('rlogl')
    indof = header.index('ndof')
    inmini = header.index('nmini')

    next(reader)
    table = list(zip(*(row for row in reader)))

    assert(float(table[ilogl][0]) == 33)
    assert(float(table[irlogl][0]) == 42)
    assert(int(table[indof][0]) == 55)
    assert(int(table[inmini][0]) == 36)

shutil.rmtree(csv_dir_name)