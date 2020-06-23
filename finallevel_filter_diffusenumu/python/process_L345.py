#!/usr/bin/env python

import os, sys, tempfile, shutil
import time

from I3Tray import *
from icecube import dataio

# L3 processing
from icecube.level3_filter_muon.MuonL3TraySegment import MuonL3

# L4 processing
from icecube.finallevel_filter_diffusenumu import level4

# L5 processing
from icecube.finallevel_filter_diffusenumu import level5

# Post-L5 processing
from icecube.finallevel_filter_diffusenumu import post_level5

# hdf writer
from icecube.finallevel_filter_diffusenumu.write_hdf import write_hdf

# import mapping of SnowStorm parameters
import snobo_parameters

def str2bool(v):
    if isinstance(v, bool):
        return v
    if v.lower() in ('yes', 'true', 't', 'y', '1'):
        return True
    elif v.lower() in ('no', 'false', 'f', 'n', '0'):
        return False
    else:
        raise argparse.ArgumentTypeError('Boolean value expected.')

def make_parser():
    """Make the argument parser"""
    from argparse import ArgumentParser
    parser = ArgumentParser()

    parser.add_argument("-i", "--input", action="store", required=True,
        type=str, dest="infile", nargs="+",
        help="Input i3 file(s)  (use comma separated list for multiple files)")

    parser.add_argument("-g", "--gcd", action="store", required=True,
        type=str, default="", dest="gcdfile",
        help="GCD file for input i3 file")

    parser.add_argument("-o", "--output", action="store", required=True,
        type=str, default="", dest="outfile",
        help="Main output file, name only")

    parser.add_argument(
        "--do_postL5",
        type=str2bool,
        nargs='?',
        const=True,
        default=False,
        dest="do_postL5",
        help="Run the PostL5 segments (HesePassed + PSRecos)?")

    parser.add_argument(
        "--is_mc",
        type=str2bool,
        nargs='?',
        const=True,
        default=False,
        dest="is_MC",
        help="Is this MC? Only then, do MuonEnergy at entry calc.")

    parser.add_argument("--hd5output", action="store",
        type=str, default="", dest="output_hd5",
        help="hd5 Output for MuonL3 (leave false if not explicitely needed)")

    parser.add_argument("--rootoutput", action="store",
        type=str, default="", dest="output_root",
        help="root Output for MuonL3 (leave false if not explicitely needed)")

    parser.add_argument("--i3output", action="store",
        type=str, default="", dest="output_i3",
        help="i3 Output for MuonL3 (leave false if not explicitely needed)")

    parser.add_argument("-n", "--num", action="store",
        type=int, default=-1, dest="num",
        help="Number of frames to process")

    parser.add_argument("--photonicsdir", action="store",
        type=str, default="/cvmfs/icecube.opensciencegrid.org/data/photon-tables/SPICEMie/",
        dest="photonicsdir", help="Directory with photonics tables")

    parser.add_argument("--photonicsdriverdir", action="store",
        type=str, default="/cvmfs/icecube.opensciencegrid.org/data/photon-tables/SPICEMie/driverfiles",
        dest="photonicsdriverdir", help="Directory with photonics tables driver files")

    parser.add_argument("--photonicsdriverfile", action="store",
        type=str, default="mu_photorec.list",
        dest="photonicsdriverfile", help="photonics driver file")

    parser.add_argument("--infmuonampsplinepath", action="store",
        type=str, default="/cvmfs/icecube.opensciencegrid.org/data/photon-tables/splines/InfBareMu_mie_abs_z20a10_V2.fits",
        dest="infmuonampsplinepath", help="InfMuonAmpSplinePath")

    parser.add_argument("--infmuonprobsplinepath", action="store",
        type=str, default="/cvmfs/icecube.opensciencegrid.org/data/photon-tables/splines/InfBareMu_mie_prob_z20a10_V2.fits",
        dest="infmuonprobsplinepath", help="InfMuonProbSplinePath")

    parser.add_argument("--cascadeampsplinepath", action="store",
        type=str, default="/cvmfs/icecube.opensciencegrid.org/data/photon-tables/splines/ems_mie_z20_a10.abs.fits",
        dest="cascadeampsplinepath", help="CascadeAmpSplinePath")

    parser.add_argument("--cascadeprobsplinepath", action="store",
        type=str, default="/cvmfs/icecube.opensciencegrid.org/data/photon-tables/splines/ems_mie_z20_a10.prob.fits",
        dest="cascadeprobsplinepath", help="CascadeProbSplinePath")

    parser.add_argument("-r", "--restoretwformc", action="store_true",
        default=False, dest="restoretwformc",
        help="Restore TimeWindow for MC?")

    parser.add_argument("--stochampsplinepath", action="store",
        type=str, default="/cvmfs/icecube.opensciencegrid.org/data/photon-tables/splines/InfHighEStoch_mie_abs_z20a10.fits",
        dest="stochampsplinepath", help="StochAmpSplinePath")

    parser.add_argument("--stochprobsplinepath", action="store",
        type=str, default="/cvmfs/icecube.opensciencegrid.org/data/photon-tables/splines/InfHighEStoch_mie_prob_z20a10.fits",
        dest="stochprobsplinepath", help="StochProbSplinePath")

    return parser

# parse arguments
parser = make_parser()
options = vars(parser.parse_args())
print("Called with:")
for key, v in options.items():
    print("{:24s}: {}".format(key, v))
print("")

# set icetray logging level "WARN", "INFO", ...
icetray.logging.set_level("WARN")

# start a timer
timer_start = time.time()

# make an infile list with gcdfile first
infiles = [options["gcdfile"]]
if isinstance(options["infile"], str):
    infiles.append(options["infile"])
else:
    for f in options["infile"]:
        infiles.append(f)

# set outfile name
outfile = options["outfile"]
if ".i3" in outfile:
    outfile = outfile.split(".i3")[0]

# logbook
print("working on: {}".format(" ".join(infiles)))
print("writing output to {}.i3.zst/.hdf".format(outfile))

# initialize I3Tray
tray = I3Tray()

# use I3FileStager for better IO performance
tray.context['I3FileStager'] = dataio.get_stagers()
tray.Add("I3Reader", FilenameList=infiles)

# L3 processing
tray.AddSegment(MuonL3,
                gcdfile=None,
                infiles=None,
                output_i3=options['output_i3'],
                output_hd5=options['output_hd5'],
                output_root=options['output_root'],
                photonicsdir=options['photonicsdir'],
                photonicsdriverdir=options['photonicsdriverdir'],
                photonicsdriverfile=options['photonicsdriverfile'],
                infmuonampsplinepath=options['infmuonampsplinepath'],
                infmuonprobsplinepath=options['infmuonprobsplinepath'],
                cascadeampsplinepath=options['cascadeampsplinepath'],
                cascadeprobsplinepath=options['cascadeprobsplinepath'],
                restore_timewindow_forMC = options['restoretwformc']
                )

cache_dir = tempfile.mkdtemp()
cache_file = cache_dir+"/cachefile_L3output.i3.zst"

tray.AddModule("I3Writer",
    Filename = cache_file,
    DropOrphanStreams=[icetray.I3Frame.DAQ, 
                       icetray.I3Frame.Stream('M'),
                       icetray.I3Frame.TrayInfo],
               Streams=[icetray.I3Frame.DAQ,
              icetray.I3Frame.Physics,
              icetray.I3Frame.TrayInfo,
              icetray.I3Frame.Simulation,
              icetray.I3Frame.Stream('M')])

# execute I3Tray
if options["num"] > 0:
    tray.Execute(int(options["num"]))
else:
    tray.Execute()

del tray

tray2 = I3Tray()
## read the L3output as input for L4+L5
tray2.context['I3FileStager'] = dataio.get_stagers()
tray2.Add("I3Reader", FilenameList=[options["gcdfile"], cache_file])

# L4 processing
tray2.AddSegment(level4.IC12L4,
                gcdfile=None,
                infiles=None,
                table_paths=options,
                is_numu=False,
                pulsemap_name="TWSRTHVInIcePulsesIC")

# L5 processing
tray2.Add(level5.segments.Scorer,
         CutFunc=level5.segments.CutFunc,
         CascCut=0.5)
## millipede
tray2.Add(level5.segments.millipede_segment, "MillipedeLosses",
         table_paths=options)
## paraboloid
tray2.Add(level5.segments.paraboloid_segment, "Paraboloid",
         table_paths=options,
         pulses="TWSRTHVInIcePulsesIC")

## postL5 (Renes PS-recos and PassedHESE bool)
if options["do_postL5"]:
    tray2.Add(post_level5.pass1_ps_reco_paraboloid, "PostDiffusePSRecoSplineMPE",
             PulsesName          = "TWSRTHVInIcePulsesIC",
             configuration       = "max",
             TrackSeedList       = ["SplineMPEIC"],
             EnergyEstimators    = ["SplineMPEICTruncatedEnergySPICEMie_AllDOMS_Muon"],
            )
    if options["is_MC"]:
        tray2.Add(post_level5.muon_energies, "getMuonEnergies")
    tray2.Add(post_level5.add_hese_tag, "AddHESETag")

## add the Snowstorm parameters as I3Doubles to the frame:
tray2.AddModule(snobo_parameters.map_parameters, "SnowstormParameterMapper",
                Streams=[icetray.I3Frame.Stream('M')])

# write output
## hdf file
tray2.Add(write_hdf,
         hdf_out=outfile+".hdf")
## i3file
tray2.AddModule("I3Writer",
    Filename = outfile+".i3.zst",
    DropOrphanStreams=[icetray.I3Frame.DAQ,
                       icetray.I3Frame.Stream('M'),
                       icetray.I3Frame.TrayInfo],
     Streams=[icetray.I3Frame.DAQ,
              icetray.I3Frame.Physics,
              icetray.I3Frame.TrayInfo,
              icetray.I3Frame.Simulation,
              icetray.I3Frame.Stream('M')])

# Dump frames to see progress
#tray.Add("Dump")

# execute I3Tray
if options["num"] > 0:
    tray2.Execute(int(options["num"]))
else:
    tray2.Execute()

#remove the tmpfile (cache of L3 output)
try:
    shutil.rmtree(cache_dir)
except OSError as e:
    print("Error: %s - %s." % (e.filename, e.strerror))

# logbook
print("finished in {:.2f}s".format(time.time() - timer_start))
