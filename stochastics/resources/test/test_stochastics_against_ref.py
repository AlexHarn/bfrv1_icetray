#!/usr/bin/env python
"""
Tests output of example against a reference file.
"""
import os
import sys
import subprocess

from argparse import ArgumentParser
parser = ArgumentParser(usage='%s [--generate|--force]'%os.path.basename(sys.argv[0]))
group = parser.add_mutually_exclusive_group()
group.add_argument("--generate", action="store_true", help="Generate reference file", default=False)
group.add_argument("--force", action="store_true", help="Generate test file (otherwise it is not generated if it exists)", default=False)
(args) = parser.parse_args()

gcd = "$I3_TESTDATA/GCD/GeoCalibDetectorStatus_IC79.55380_corrected.i3.gz"

common = os.path.splitext(os.path.basename(sys.argv[0]))[0] + ".i3.gz"
#refFile = os.path.expandvars("$I3_TESTDATA/icetop/ref." + common) # eventually reference should be here
refFile = os.path.expandvars(os.path.dirname(os.path.realpath(__file__)) + "/ref." + common)
outFile = "cur." + common

if args.generate:
    outFile = refFile
    refFile = None

if refFile and not os.path.exists(refFile):
    print('ERROR: Reference does not exist')
    parser.print_help()
    parser.exit(1)

# create file
if args.force or args.generate or not os.path.exists(outFile):
    cmd = [os.path.expandvars("$I3_BUILD/stochastics/resources/examples/energy_loss_fit.py"),
           "-o", outFile,
           "-l", "Millipede_dEdX",
           "-n", "5",
           "-m",
           os.path.expandvars(gcd),
           #os.path.expandvars("$I3_TESTDATA/icetop/Level3_IC86.2012_data_Run00120244_Part00_IT73_IT_coinc.i3")
           os.path.expandvars(os.path.dirname(os.path.realpath(__file__)) + "/test_data.i3")
       ]
    if args.generate: cmd.append('--true')
    subprocess.check_call(cmd)

try:
    from icecube.topsimulator.validationsuite import ValidationReport, Cmp, CmpFields, CmpI3File
except:
    print("Validation tools are not available. Can not validate")
    sys.exit(0)
from icecube import stochastics

# this class is the class that does the comparison of I3EnergyLoss objects
def CmpI3EnergyLoss(rep, a, b, **kwargs):
    return CmpFields(rep, a, b,
                     'nHEstoch',
                     'avRelStochEnergy', 'avStochDepth', 'avStochEnergy',
                     'highestRelStochEnergy', 'highestStochEnergy',
                     'chi2', 'chi2_red',
                     'eLoss_1000','eLoss_1500','eLoss_1600','eLoss_1700','eLoss_1800','eLoss_1900',
                     'eLoss_2000','eLoss_2100','eLoss_2200','eLoss_2300','eLoss_2400','eLoss_3000',
                     'primEnergyEstimate','primEnergyEstimate_err',
                     'primMassEstimate','primMassEstimate_err',
                     'status',
                     'totalRelStochEnergy',
                     #'totalStochEnergy'
                     **kwargs
    )


# compare output file with reference file and create report (only if not args.generate)
if refFile:
    Cmp.register(stochastics.I3EnergyLoss, CmpI3EnergyLoss)

    report = ValidationReport("Comparing stochastics test output with reference")

    CmpI3File(report, outFile, refFile,
              #ignoreKeys=(),
              strictOnLayout=True,
              tolerance=1e-6
          )

    if len(report) == 0:
        print("Reference files are missing.")
        sys.exit(1)
    else:
        print(report)
        sys.exit(report == False)

