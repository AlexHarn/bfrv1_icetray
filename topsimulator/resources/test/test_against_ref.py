#!/usr/bin/env python
"""
Tests output of simulation segment against a reference file.

Call with argument "makeref" to generate new reference files.
"""
import os
import sys
import subprocess as subp
from util import have_g4tankresponse

i3_testdata = os.path.expandvars("$I3_TESTDATA")

gcd = i3_testdata + "/sim/GeoCalibDetectorStatus_IC86.55697_corrected_V2.i3.gz"
inFile = i3_testdata + "corsika-reader/DAT010000"

# generate test files for both tank responses
filesToCompareList = []
for resp in ("param", "g4"):
    if resp == "g4" and not have_g4tankresponse:
        continue
    print("Response: " + resp)

    # if reference does not exist, create it instead of comparing
    base = os.path.basename(sys.argv[0])[:-3]
    common = base + "-%s.i3.gz" % resp
    refFile = i3_testdata + "/topsimulator/ref." + common
    outFile = "cur." + common

    if not os.path.exists(outFile):
        cmd = ["topsimulator-simicetop", "--gcd", gcd, "--resp", resp, "--pecompress",  "1",
               "-s", "1", "-r", "0", "--seed", "1", "-o", outFile, inFile]
        print(" ".join(cmd))
        subp.check_call(cmd)

    filesToCompareList.append((outFile, refFile))

if len(sys.argv) == 2 and sys.argv[1] == "makeref":
    # copy newly-created files to the refence location
    import shutil
    for pair in filesToCompareList:
        outFile, refFile = pair
        print("Writing " + refFile)
        shutil.copy(outFile, refFile)
    raise SystemExit
else:
    # prepare comparison
    from icecube.topsimulator.validationsuite import ValidationReport, Cmp, \
      CmpMap, CmpI3Tree, CmpI3File, CmpFields, CmpSeries


    def CmpPESeries(rep, a, b, **kwargs):
        # compare npe counts per time, ignore pe source info
        from collections import defaultdict
        histA = defaultdict(lambda: 0)
        histB = defaultdict(lambda: 0)
        rep.result(sum([x.npe for x in a])==sum([x.npe for x in b]))
        rep.result(min([x.time for x in a])==min([x.time for x in b]))
        rep.result(max([x.time for x in a])==max([x.time for x in b]))
        for x in a:
            histA[x.time] += x.npe
        for x in b:
            histB[x.time] += x.npe
        # this fails when a PE moves to the neighboring bin, which can happen.
        #rep.result(histA == histB)

    def CmpPrimaryInfo(rep, a, b, **kwargs):
        return CmpFields(rep, a, b,
                         'crsRunID',
                         'crsEventID',
                         'crsSampleID',
                         'firstIntHeight',
                         #'firstIntDepth', # this is still not set
                         'obsLevelHeight',
                         'ghMaxNum',
                         'ghStartDepth',
                         'ghMaxDepth',
                         'ghLambdaa',
                         'ghLambdab',
                         'ghLambdac',
                         'ghRedChiSqr',
                         'resampleRadius',
                         'nResample',
                         'nResampleNominal',
                         'weight',
                         'longProfile',
                         **kwargs
                     )

    def CmpMCHit(rep, a, b, **kwargs):
        # ignore ParticleID
        return CmpFields(rep, a, b,
                         "charge", "cherenkov_distance", "hit_id", "hit_source",
                         "npe", "time")

    from icecube import simclasses, dataclasses
    Cmp.register(simclasses.I3MCPESeries, CmpPESeries)
    Cmp.register(dataclasses.I3MCTree, CmpI3Tree)
    Cmp.register(dataclasses.I3MCHit, CmpMCHit)
    Cmp.register(simclasses.I3CorsikaShowerInfo, CmpPrimaryInfo)

    # check that output is identical to reference
    report = ValidationReport("Comparing current simulation with reference")

    for cur, ref in filesToCompareList:
        if ref is None:  # we ran in reference generation mode
            continue

        CmpI3File(report, cur, ref,
                  ignoreKeys=(
                      "MCTopPESeriesMap_PulsesParticleIDMap", # ParticleIDs change all the time
                  ),
                  strictOnLayout=False,
                  tolerance=1e-2)

    if len(report) == 0:
        print("Reference files are missing.")
        sys.exit(1)
    else:
        print(report)
        sys.exit(report == False)
