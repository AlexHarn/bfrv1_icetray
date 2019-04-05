#!/usr/bin/env python
#
#  Normalize Weighting
#
#  $Id$
#  @author aobertacke
#


'''
Use this script to normalize the weighting which is calculated by monopole-generator:
    monopole-generator puts a "Weight" into the "MPInfoDict" frame.
After that you have to use this script on all log files at once!
This scripts sums all TimeScale values (sum of weights per file) up to "NormWeight" ,
    which is only correct if you used this script on all log files.
    You cannot only sum up the weights in the produced i3 files, because you loose a lot of
    events with triggering!
Later you should calculate the final weight by dividing "Weight" by "NormWeight"
    (times the wanted flux like 10^-16 cm^-2 s^-1 sr^-1)

Usage: give a pattern for all log files as input
'''

from glob import glob
import sys


pattern=sys.argv[1]

files=glob(pattern)


timescales=[]
for file in files:
    f=open(file)
    for line in f:
        if "Timescale" in line:
            ts=float(line.split(" ")[1])
            timescales.append(ts)
    f.close(file)


normalization=sum(timescales)

print "Devide all weights by %f and multiply them by the wished flux" % normalization


