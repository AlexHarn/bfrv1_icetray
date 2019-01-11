#!/usr/bin/env python

from __future__ import print_function

import os
import sys
import string
from I3Tray import *
from os.path import expandvars
from icecube import dataio
from optparse import OptionParser

load("libdataclasses")
load("libdataio")
load("libphys-services")
load("libcscd-llh")
load("libclast")

workspace = expandvars("$I3_BUILD")
i3testdata = expandvars("$I3_TESTDATA")
dataf = i3testdata + "/event-viewer/Level3aGCD_IC79_EEData_Run00115990.i3"

usage = "usage: %prog [options]"
parser = OptionParser(usage)


parser.add_option("--executeallev", action="store_true", default=False, dest="EXEC", help="If True script executes all events")
parser.add_option("--executenev", type="int", default=5+3, dest="NEVENTS", help="If EXEC ==false, script executes this number of events")

parser.add_option("-n", "--cscdllhname", action="store", type="string", default='cscd-llh', dest="CSCDLLHNAME", help="Cscd-llh name")
parser.add_option("-i", "--input", action="store", type="string", default=dataf, dest="INPUT", help="Input i3 file to process")
parser.add_option("--inputtype", type="string", default="RecoPulse", dest="INPUTTYPE", help="Type of input hits. Options are RecoHit and RecoPulse")
parser.add_option("--inputreadout", type="string", default="MaskedOfflinePulses", dest="INPUTREADOUT", help="Input Read Out")



parser.add_option("--seedkey", type="string", default="CFirst", dest="SEEDKEY", help="Seed Key")
parser.add_option("--energyseed", type="float", default=2.0, dest="ENERGYSEED", help="Energy seed")
parser.add_option("--seedwithorigin", action="store_true", default=False, dest="SEEDWITHORIGIN", help="If true, use the detector origin and t=0 as a first guess")

parser.add_option("--minhits", type="int", default=10, dest="MINHITS", help="Min hits")

parser.add_option("--useampweightpower", action="store_true", default=False, dest="USEAMPWEIGHTPOWER", help="If True script uses Amp weight power")
parser.add_option("--ampweightpower", type="float", default=0.0, dest="AMPWEIGHTPOWER", help="Amp Weight Power")

parser.add_option("-m", "--minimizer", type="string", default="Powell", dest="MINIMIZER", help="Minimizer name")
parser.add_option("--maxcalls", type="int", default=500000, dest="MAXCALLS", help="Max calls")
parser.add_option("--tolerance", type="float", default=0.001, dest="TOLERANCE", help="Tolerance")

parser.add_option("--useparamt", action="store_true", default=False, dest="USEPARAMT", help="If True script uses ParamT")
parser.add_option("--paramtstepsize", type="float", default=50.0, dest="PARAMTSTEPSIZE", help="ParamT step size")
parser.add_option("--paramtlowerlimit", type="float", default=0.0, dest="PARAMTLOWERLIMIT", help="ParamT lower limit")
parser.add_option("--paramtupperlimit", type="float", default=0.0, dest="PARAMTUPPERLIMIT", help="ParamT uper limit")
parser.add_option("--fixparamt", action="store_true", default=False, dest="FIXPARAMT", help="If True script fixes ParamT")

parser.add_option("--paramxstepsize", type="float", default=10.0, dest="PARAMXSTEPSIZE", help="ParamX step size")
parser.add_option("--paramxlowerlimit", type="float", default=0.0, dest="PARAMXLOWERLIMIT", help="ParamX lower limit")
parser.add_option("--paramxupperlimit", type="float", default=0.0, dest="PARAMXUPPERLIMIT", help="ParamX uper limit")
parser.add_option("--fixparamx", action="store_true", default=False, dest="FIXPARAMX", help="If True script fixes ParamX")

parser.add_option("--paramystepsize", type="float", default=10.0, dest="PARAMYSTEPSIZE", help="ParamY step size")
parser.add_option("--paramylowerlimit", type="float", default=0.0, dest="PARAMYLOWERLIMIT", help="ParamY lower limit")
parser.add_option("--paramyupperlimit", type="float", default=0.0, dest="PARAMYUPPERLIMIT", help="ParamY uper limit")
parser.add_option("--fixparamy", action="store_true", default=False, dest="FIXPARAMY", help="If True script fixes ParamY")

parser.add_option("--paramzstepsize", type="float", default=10.0, dest="PARAMZSTEPSIZE", help="ParamZ step size")
parser.add_option("--paramzlowerlimit", type="float", default=0.0, dest="PARAMZLOWERLIMIT", help="ParamZ lower limit")
parser.add_option("--paramzupperlimit", type="float", default=0.0, dest="PARAMZUPPERLIMIT", help="ParamZ uper limit")
parser.add_option("--fixparamz", action="store_true", default=False, dest="FIXPARAMZ", help="If True script fixes ParamZ")

parser.add_option("--useparamenergy", action="store_true", default=False, dest="USEPARAMENERGY", help="If True script uses ParamEnrgy")
parser.add_option("--paramenergystepsize", type="float", default=10.0, dest="PARAMENERGYSTEPSIZE", help="ParamEnergy step size")
parser.add_option("--paramenergylowerlimit", type="float", default=0.0, dest="PARAMENERGYLOWERLIMIT", help="ParamEnergy lower limit")
parser.add_option("--paramenergyupperlimit", type="float", default=0.0, dest="PARAMENERGYUPPERLIMIT", help="ParamEnergy uper limit")
parser.add_option("--fixparamenergy", action="store_true", default=False, dest="FIXPARAMENERGY", help="If True script fixes ParamEnergy")

parser.add_option("-p", "--pdf", type="string", default="HitNoHit", dest="PDF", help="Pdf name")
parser.add_option("--pndlhnhweight", type="float", default=1.0, dest="PNDLHNNWEIGHT", help="Pndl Hnh Weight")

parser.add_option("--minuitverbositylevel", type="int", default=-1, dest="MINUITVERBOSITYLEVEL", help="Minuit output message level. Allowed values are from -1 to 3, inclusive. Higher values give more messages")
parser.add_option("--minuitprintwarnings", type="int", default=0, dest="MINUITPRINTWARNINGS", help="Output Minuit warnings")
parser.add_option("--minuitstrategy", type="int", default=2, dest="MINUITSTRATEGY", help="Output Minuit warnings")
parser.add_option("--minuitcalculategradient", action="store_true", default=False, dest="MINUITCALCULATEGRADIENT", help="Calculate the first derivatives of the minimized funtion")
parser.add_option("--minuitminos", action="store_true", default=False, dest="MINUITMINOS", help="Use exact (non-linear) parameter error analysis")

parser.add_option("--calculatehesse", action="store_true", default=False, dest="CALCULATEHESSE", help="Calculate the Hessian error matrix")
parser.add_option("--improve", action="store_true", default=False, dest="IMPROVE", help="Search for a new minimum around the current minimum")

parser.add_option("--loge", action="store_true", default=False, dest="LOGE", help="Minimizing in log(E) can improve the results when E varies slowly across the range")


(options,args) = parser.parse_args()
if len(args) != 0:
        crap = "Got undefined options:"
        for a in args:
                crap += a
                crap += " "
        parser.error(crap)


tray = I3Tray()

tray.AddModule("I3Reader","reader")
tray.SetParameter("reader", "Filename", options.INPUT)

tray.AddModule("I3CLastModule","cfirst")
tray.SetParameter("cfirst", "MinHits", 10)
tray.SetParameter("cfirst", "DirectHitRadius", 100.0)
tray.SetParameter("cfirst", "InputReadout", "MaskedOfflinePulses")
tray.SetParameter("cfirst", "Name", "CFirst")

tray.AddModule("I3CscdLlhModule", options.CSCDLLHNAME)

tray.SetParameter(options.CSCDLLHNAME, "InputType", options.INPUTTYPE)
tray.SetParameter(options.CSCDLLHNAME, "RecoSeries",options.INPUTREADOUT)

tray.SetParameter(options.CSCDLLHNAME, "SeedWithOrigin", options.SEEDWITHORIGIN)
# if True the SeedKey will be ignored.  To be used only for testing purposes

tray.SetParameter(options.CSCDLLHNAME, "SeedKey", options.SEEDKEY)
tray.SetParameter(options.CSCDLLHNAME, "EnergySeed", options.ENERGYSEED)
tray.SetParameter(options.CSCDLLHNAME, "MinHits", options.MINHITS)
if(options.USEAMPWEIGHTPOWER):
    print("Using Amp Weight Power")
    tray.SetParameter(options.CSCDLLHNAME, "AmpWeightPower", options.AMPWEIGHTPOWER)

tray.SetParameter(options.CSCDLLHNAME, "ResultName", "CscdLlh")

# "Brent", "GfxMinimizer", "Minuit", "Powell", and "Simplex"
tray.SetParameter(options.CSCDLLHNAME, "Minimizer", options.MINIMIZER)
tray.SetParameter(options.CSCDLLHNAME, "MaxCalls", options.MAXCALLS)
tray.SetParameter(options.CSCDLLHNAME, "Tolerance", options.TOLERANCE)

if(options.USEPARAMT):
    if(options.FIXPARAMT):
        stringparamt = str(options.PARAMTSTEPSIZE) + ', ' + str(options.PARAMTLOWERLIMIT) + ', ' + str(options.PARAMTUPPERLIMIT) + ', true'
    else:
        stringparamt = str(options.PARAMTSTEPSIZE) + ', ' + str(options.PARAMTLOWERLIMIT) + ', ' + str(options.PARAMTUPPERLIMIT) + ', false'
    print("Using ParamT")
    print(stringparamt)
    tray.SetParameter(options.CSCDLLHNAME, "ParamT", stringparamt)
    
if(options.FIXPARAMX):
    stringparamx = str(options.PARAMXSTEPSIZE) + ', ' + str(options.PARAMXLOWERLIMIT) + ', ' + str(options.PARAMXUPPERLIMIT) + ', true'
else:
    stringparamx = str(options.PARAMXSTEPSIZE) + ', ' + str(options.PARAMXLOWERLIMIT) + ', ' + str(options.PARAMXUPPERLIMIT) + ', false'
tray.SetParameter(options.CSCDLLHNAME, "ParamX", stringparamx)


if(options.FIXPARAMY):
    stringparamy = str(options.PARAMYSTEPSIZE) + ', ' + str(options.PARAMYLOWERLIMIT) + ', ' + str(options.PARAMYUPPERLIMIT) + ', true'
else:
    stringparamy = str(options.PARAMYSTEPSIZE) + ', ' + str(options.PARAMYLOWERLIMIT) + ', ' + str(options.PARAMYUPPERLIMIT) + ', false'
tray.SetParameter(options.CSCDLLHNAME, "ParamY", stringparamy)

if(options.FIXPARAMZ):
    stringparamz = str(options.PARAMZSTEPSIZE) + ', ' + str(options.PARAMZLOWERLIMIT) + ', ' + str(options.PARAMZUPPERLIMIT) + ', true'
else:
    stringparamz = str(options.PARAMZSTEPSIZE) + ', ' + str(options.PARAMZLOWERLIMIT) + ', ' + str(options.PARAMZUPPERLIMIT) + ', false'
tray.SetParameter(options.CSCDLLHNAME, "ParamZ", stringparamz)

if(options.USEPARAMENERGY):
    print("Using ParamE")    
    if(options.FIXPARAMZ):
        stringparame = str(options.PARAMENERGYSTEPSIZE) + ', ' + str(options.PARAMENERGYLOWERLIMIT) + ', ' + str(options.PARAMENERGYUPPERLIMIT) + ', true'
    else:
        stringparame = str(options.PARAMENERGYSTEPSIZE) + ', ' + str(options.PARAMENERGYLOWERLIMIT) + ', ' + str(options.PARAMENERGYUPPERLIMIT) + ', false'
    print(stringparame)
    tray.SetParameter(options.CSCDLLHNAME, "ParamEnergy", stringparame)

#Description : The probability density function.
#Options are "UPandel", "UPandelMpe", "HitNoHit", "HitNoHitMpe", and "PndlHnh","HnhDir "
tray.SetParameter(options.CSCDLLHNAME, "PDF", options.PDF)

# PndlHnh
tray.SetParameter(options.CSCDLLHNAME, "PndlHnhWeight", options.PNDLHNNWEIGHT)

# UPandel
tray.SetParameter(options.CSCDLLHNAME, "PandelSmallProb", 1.0e-6)
tray.SetParameter(options.CSCDLLHNAME, "PandelTau", 450.0)
tray.SetParameter(options.CSCDLLHNAME, "PandelLambda", 47.0)
tray.SetParameter(options.CSCDLLHNAME, "PandelLambdaA", 96.0)
tray.SetParameter(options.CSCDLLHNAME, "PandelSigma", 15.0)
#tray.SetParameter("cscd-llh", "PandelLightSpeed",)

tray.SetParameter(options.CSCDLLHNAME, "PandelMaxDist", 0.0)

# Hit/No-hit
tray.SetParameter(options.CSCDLLHNAME, "HitNoHitNorm", 1.4)
tray.SetParameter(options.CSCDLLHNAME, "HitNoHitLambdaAttn", 29.0)
tray.SetParameter(options.CSCDLLHNAME, "HitNoHitNoise", 5.0e-3)
tray.SetParameter(options.CSCDLLHNAME, "HitNoHitDistCutoff", 0.5)
tray.SetParameter(options.CSCDLLHNAME, "HitNoHitDead", 0.05)
tray.SetParameter(options.CSCDLLHNAME, "HitNoHitSmallProb", 1.0e-20)




if(options.EXEC):
    tray.Execute()
else:
    print(options.NEVENTS)
    tray.Execute(options.NEVENTS)

