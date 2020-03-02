from __future__ import division, absolute_import, with_statement, print_function
#Example:
#./UniversalMonopoleSimulationGeneration.py -g /data/sim/sim-new/downloads/GCD/GeoCalibDetectorStatus_2013.56429_V1.i3.gz -d ./ -r 1 -n 1 -x 0.1  -y 0.8 -z 0 --taudnde 1000,0.2
import argparse
import os
import tempfile

import I3Tray
from I3Tray import load
from icecube.monopole_generator.icetray import DefaultMonopoleSimulationTray
from icecube import icetray, dataclasses, dataio, sim_services, phys_services
from icecube import monopole_generator
from icecube.icetray import I3Units
from icecube.ppc import tau_dnde_builder



I3Tray.load("xppc")
I3Tray.load("ppc")
I3Tray.load("libDOMLauncher")


#filename template from the MonSim Production wiki
def gen_filename(runid, NEvents, TotalFiles, fileNumber, betaMin, betaMax, power_law, trigger_level):
    betaMin = str(betaMin).replace(".", "")
    betaMax = str(betaMax).replace(".", "")
    filename_template = "MonoSim_%8d %8d_%8d_%8d_%8d_%s_%s_%s_lv_%s.i3.zstd"
    seed = runid
    return filename_template % (runid, seed, NEvents, TotalFiles, fileNumber, betaMin, betaMax, power_law, trigger_level)


#custom types for ArgParser
def tau_dnde(s):
    try:
        tau, dnde = map(float, s.split(','))
        return tau, dnde
    except Exception:
        raise argparse.ArgumentTypeError("tau, dnde is needed")


def is_valid_read_file(path):
    if not os.path.isfile(path):
        raise argparse.ArgumentTypeError("No valid path to file")
    else:
        return path


def is_valid_read_dir(path):
    if not os.path.isdir(path):
        raise argparse.ArgumentTypeError("No valid path to directory")
    else:
        return path


def is_valid_write_dir(path):
    is_valid_read_dir(path)
    try:
        tmp = tempfile.TemporaryFile(dir=path)
        return path
    except Exception:
        raise argparse.ArgumentTypeError("Directory was not writable")


def u_int(s):
    tmp = int(s)
    if tmp < 0:
        raise argparse.ArgumentTypeError("Unsigned integer was negativ")
    return tmp

def beta_range(s):
    tmp = float(s)
    if not 0 < tmp < 1:
        raise argparse.ArgumentTypeError("Beta value was out of range, needs to be 0 < beta < 1")
    return tmp



if __name__ == "__main__":
    parser = argparse.ArgumentParser(prog='Universal Monopole Simulation Script (UMSS)')
    parser.add_argument("-g", '--GCDFile',           required=True,  action='store', type=is_valid_read_file,                                        help='The path to the GCD file.')
    parser.add_argument("-d", "--OutDir",            required=True,  action="store", type=is_valid_write_dir,                                        help="The directory where the file should be created. Filename will be automatically be choosen from the given settings.")
    parser.add_argument("-r", '--RunID',             required=True,  action='store', type=u_int,                                                     help='RunID for the dataset. Also used as seed as all datasets should have different IDs')
    parser.add_argument('-n', "--NumEventsPerFile",  required=False, action='store', type=u_int, default=10000,                                      help='Number of monopoles to be generated')
    parser.add_argument('-x', '--BetaMin',           required=True,  action='store', type=beta_range,                                                help='Lower end of the simulated speed region')
    parser.add_argument('-y', '--BetaMax',           required=True,  action='store', type=beta_range,                                                help='Upper end of the simulated speed region')
    parser.add_argument('-z', '--PowerLaw',          required=False, action='store', type=float, default=0,                                          help='Powerlaw factor to have signal simulation be weighted with beta^-powerlaw. NaN or 0 lead to flat distribution')
    parser.add_argument('--Mass',                    required=False, action='store', type=float, default=1.0e11*I3Units.GeV,                         help='Restmass of the monopole.')
    parser.add_argument('--DiskDist',                required=False, action='store', type=float, default=1000*I3Units.m,                             help='Distance of the generation disk from the detector. Default should be fine unless simulating very bright events')
    parser.add_argument('--DiskRad',                 required=False, action='store', type=float, default=850*I3Units.m,                              help='Radius of the generating disk. Default should be fine unless simulating very bright events.')
    parser.add_argument('--TauDnDe',                 required=False, action='store', type=tau_dnde, nargs="*",                                       help="Luminescence efficencys Tau is the decay time in ns and dnde is the light yield for this mde in photons/MeV. Multiple modes can be defined. Not setting anything will let to no light production between 0.1 and 0.5c ", )
    parser.add_argument('--MaxDistanceFromCenter',   required=False, action='store', type=float, default=1400,                                       help="Distance from the center of the detector there the propagation of the monopoles is halted")
    parser.add_argument('--StepSize',                required=False, action='store', type=float, default=float("nan"),                               help="Fixed stepsize for each monopole in I3MCTree, if nan length is automatically calculated to be roughtly every 0.1% energy loss")
    parser.add_argument('--MinLength',               required=False, action='store', type=float, default=float("nan"),                               help="Minimal length of one monopole in I3MCTree. Should be as small as possibe but might be required to be set for very bright speed regions")
    parser.add_argument('--MaxLength',               required=False, action='store', type=float, default=50,                                         help="Maximal length of one monopole in I3MCTree. Lowering increases storing requirements but might need be required for bright speed regions")
    parser.add_argument('--TotalFileNumberInDataset',required=False, action='store', type=u_int, default=1,                                          help="NStreams of I3SPRNGRandomServiceFactory, should be the total number of files of dataset ")
    parser.add_argument('--FileNumberInDataset',     required=False, action='store', type=u_int, default=0,                                          help="StreamNum of I3SPRNGRandomServiceFactory, should be file number in the dataset")
    parser.add_argument('--IceModel ',               required=False, action='store', type=is_valid_read_dir, default="$I3_BUILD/ppc/resources/ice/", help="Path to the icemodel to be used.")

    args=parser.parse_args()
    outname = gen_filename(args.RunID, args.NumEventsPerFile, args.TotalFileNumberInDataset, args.FileNumberInDataset, args.BetaMin, args.BetaMax, args.PowerLaw, "0")
    outpath = os.path.join(args.OutDir, outname)
    #some sanity checks
    if os.path.isfile(outpath):
        raise Exception("The outputfile already exists. Refusing to overwrite it. Abort.")

    if (args.BetaMin > args.BetaMax):
        argparse.ArgumentTypeError("Specified mininmal speed was greater than maximal speed")


    if args.TauDnDe is None:
        args.TauDnDe = []

    tmp = []
    for i in args.TauDnDe:
        tmp.append((i[0]*I3Units.ns, i[1] * I3Units.eV/I3Units.MeV))
    args.TauDnDe = tau_dnde_builder(tmp)
    del tmp

    tray = I3Tray.I3Tray()
    tray.AddService("I3SPRNGRandomServiceFactory",
                    Seed=args.RunID,
                    NStreams=args.TotalFileNumberInDataset,
                    StreamNum=args.FileNumberInDataset)

    tray.AddModule("I3InfiniteSource", Prefix=args.GCDFile)

    tray.Add(DefaultMonopoleSimulationTray, "MonopoleSim",
        RunID=args.RunID,
        NEvents=args.NumEventsPerFile,
        GCDFile=args.GCDFile,
        Mass=args.Mass,
        betaRange=(args.BetaMin, args.BetaMax),
        taudnde=args.TauDnDe,
        PowerLawIndex=args.PowerLaw,
        StepSize=args.StepSize,
        Disk_dist=args.DiskDist,
        Disk_rad=args.DiskRad,
        MinLength=args.MinLength,
        MaxLength=args.MaxLength,
        MaxDistanceFromCenter=args.MaxDistanceFromCenter,
        icemodel="$I3_BUILD/ppc/resources/ice/",
    )

    tray.AddModule("I3Writer",
                   filename=outpath,
                   streams=[icetray.I3Frame.DAQ],
                   SkipKeys=["I3MCPulseSeriesMapPrimaryIDMap"], #alex added this, no clue why //fhl
                   )

    tray.Execute()
