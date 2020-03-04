#!/usr/bin/env python

import sys,os.path,csv
import shutil
import unittest
from I3Tray import *
from icecube import icetray,dataclasses,tableio
from icecube import gulliver_bootstrap

#This tests the converters to output the boostrap params into tables

csv_dir_name = "tableout"
frame_name = "TheParams"

def PutInFrame(frame):
    params = gulliver_bootstrap.BootstrapParams()
    params.status = gulliver_bootstrap.BootstrapParams.ResultStatus.Underflow
    params.totalFits = 42
    params.successfulFits = 84
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
    iStatus = header.index('status')
    iTotalFits = header.index('totalFits')
    iSuccessfulFits = header.index('successfulFits')

    next(reader)
    table = list(zip(*(row for row in reader)))

    assert(int(table[iStatus][0]) == int(gulliver_bootstrap.BootstrapParams.ResultStatus.Underflow))
    assert(int(table[iTotalFits][0]) == 42)
    assert(int(table[iSuccessfulFits][0]) == 84)

shutil.rmtree(csv_dir_name)