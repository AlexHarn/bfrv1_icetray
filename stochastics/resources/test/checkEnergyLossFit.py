#!/usr/bin/env python

from icecube import dataio, icetray, stochastics, dataclasses
import math
from scipy import interpolate

# We should run this on a file in I3TEST_DATA
# For now, we keep it like it is, but thus need to define some fixed (cosmic ray) file.

from I3Tray import *

from argparse import ArgumentParser
parser = ArgumentParser(usage='%s [options] -o <filename>.i3[.bz2|.gz] {i3 file list}'%os.path.basename(sys.argv[0]))
parser.add_argument("-l", "--losses", action="store", type=str, dest="losses", help="vector<I3Particle> containing the energy losses", metavar="BASENAME")
parser.add_argument('inputFiles',help="Input file(s)",nargs="*")
parser.add_argument("-n", action="store", type=int, dest="n", help="number of frames to process")
(args) = parser.parse_args()

if not len(args.inputFiles)>0:
    parser.print_help()
    icetray.logging.log_fatal("Specify input files.")

if not args.losses:
    parser.print_help()
    icetray.logging.log_fatal("Specify vector of I3Particle containing energy losses.")

tray=I3Tray()

tray.AddModule("I3Reader","my_reader",FilenameList=args.inputFiles)

# Parameters are the standard ones used in the cosmic ray IC79 composition analysis.
tray.AddModule('I3Stochastics','stoch_standard',
               A_Param = 0,
               B_Param = 5,
               C_Param = 0.8,
               FreeParams = 1,
               InputParticleVector=args.losses,
               Minimizer = 'MIGRAD',
               OutputName = 'Stoch_Reco_TEST',
               OutputName_red = 'Stoch_Reco_red_TEST',
               Verbose = False,
               SelectionType = 'Type2')

def ElossFunction(x,param_A,param_E0,zenith):
    func=math.exp(-0.00032852*x)*(14.5*param_A/math.cos(zenith))*1.757*pow(param_E0/param_A,1.757-1.)*( -pow(param_E0/param_A,-1.757)*(0.23881/1.757 - 0.00032852/(1-1.757)*param_E0/param_A) + pow((0.23881/0.00032852 * (math.exp(0.00032852*x) -1.)),-1.757)* (0.23881/1.757 - 0.00032852/(1-1.757)*(0.23881/0.00032852 * (math.exp(0.0003285*x) -1.))))
    return func

def testStoch(frame,StochName, inputElosses):
    # Stochastics should be there if the eLosses are in the frame, and if the reconstructed muon bundle is going downwards.
    if inputElosses in frame and frame[inputElosses][0].dir.zenith<math.pi/2.:
        if StochName not in frame:
            icetray.logging.log_fatal("%s should be in frame."%StochName,"testStoch")
         
        # did the fit work?
        stoch=frame[StochName]
        if stoch.status!=0:
            icetray.logging.log_info("Fit did not succeed. Run: %i; Event: %i.%i"%(frame["I3EventHeader"].run_id,frame["I3EventHeader"].event_id,frame["I3EventHeader"].sub_event_id),"testStoch")
        else:
            param_A=stoch.primMassEstimate # should be 16
            param_E0=stoch.primEnergyEstimate

            # Get the slant depths and the reconstructed eloss:
            slants=[]
            toteLoss=0
            for eLoss in frame[inputElosses]:
                slants.append((dataclasses.I3Constants.zIceTop - eLoss.pos.z)/math.cos(eLoss.dir.zenith))
                toteLoss+=eLoss.energy

            # Evaluate the fit at the slant depths, and compare this with the input
            f=[ElossFunction(slant,param_A,param_E0,frame[inputElosses][0].dir.zenith) for slant in slants]
            frac_diff=(sum(f)-toteLoss)/toteLoss
            # Let's say that we do not want a factor of 2 difference (just a guess, I see maximally 0.1 differences in the file I checked.)
            if frac_diff>2. or frac_diff<-0.5:
                icetray.logging.log_fatal("Seems like a bad fit: Fitted total energy loss: %.2f; Input energy losses: %.2f ; Run: %i; Event: %i.%i"%(sum(f),toteLoss,frame["I3EventHeader"].run_id,frame["I3EventHeader"].event_id,frame["I3EventHeader"].sub_event_id),"testStoch")

icetray.logging.set_level_for_unit("testStoch",icetray.I3LogLevel.LOG_INFO)
tray.AddModule(testStoch,"testStochastics",
               StochName="Stoch_Reco_TEST",
               inputElosses=args.losses)

if args.n:
    tray.Execute(args.n)
else:
    tray.Execute()


