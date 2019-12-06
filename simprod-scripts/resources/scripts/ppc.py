#!/usr/bin/env python

"""
 GPU Photon propagation
"""

import os
from os.path import expandvars
from I3Tray import I3Tray, I3Units
from icecube.simprod.util import ReadI3Summary, WriteI3Summary
from icecube.simprod.util import simprodtray
from icecube.simprod.util.simprodtray import RunI3Tray
import argparse
import json
from icecube import icetray, dataclasses, simclasses, sim_services
from icecube import polyplopia
from icecube import ppc
from icecube.simclasses import I3MCPESeriesMap
from icecube import dataio, phys_services, interfaces
from icecube.simprod import segments
from icecube.production_histograms import ProductionHistogramModule
from icecube.production_histograms.histogram_modules.simulation.mcpe_module import I3MCPEModule


def add_args(parser):
    """
    Args:
        parser (argparse.ArgumentParser): the command-line parser
    """
    simprodtray.add_argument_gcdfile(parser)
    simprodtray.add_argument_inputfilelist(parser)
    simprodtray.add_argument_outputfile(parser)

    simprodtray.add_argument_nproc(parser)
    simprodtray.add_argument_procnum(parser)
    simprodtray.add_argument_seed(parser)
    simprodtray.add_argument_usegslrng(parser)
    
    parser.add_argument("--summaryfile", dest="summaryfile",
                        default='summary.json', type=str, required=False,
                        help='JSON Summary filename')
    parser.add_argument("--GPU", dest="gpu",
                        default=-1, type=float, required=False,
                        help="Graphics Processing Unit number (shoud default to environment if None)")
    parser.add_argument("--no-UseGPUs", dest="usegpus",
                        default=True, action="store_false", required=False,
                        help="Use Graphics Processing Unit")
    parser.add_argument("--no-RunMPHitFilter", dest="runmphitfilter",
                        default=True, action="store_false", required=False,
                        help="Run polyplopia's mphitfilter")
    parser.add_argument("--oversize", dest="oversize",
                        default=5, type=int, required=False,
                        help="over-R: DOM radius oversize scaling factor")
    parser.add_argument("--efficiency", dest="efficiency",
                        default=1.00, type=float, required=False,
                        help="overall DOM efficiency scaling factor (systematics)")
    parser.add_argument("--gpulib", dest="gpulib",
                        default="opencl", type=str, required=False,
                        help="set gpu library to load (defaults to cuda)")
    parser.add_argument("--no-volumecyl", dest="volumecyl",
                        default=True, action="store_false", required=False,
                        help="set volume to regular cylinder (set to False for 300m spacing from the DOMs)")
    parser.add_argument("--PhotonSeriesName", dest="photonseriesname",
                        default="I3MCPESeriesMap", type=str, required=False,
                        help="Photon Series Name")
    parser.add_argument("--IceModelLocation", dest="icemodellocation",
                        default=expandvars("$I3_BUILD/ice-models/resources/models"),
                        type=str, required=False,
                        help="Location of ice model param files")
    parser.add_argument("--IceModel", dest="icemodel",
                        default="SpiceMie", type=str, required=False,
                        help="ice model subdirectory")
    parser.add_argument("--holeiceparametrization", dest="holeiceparametrization",
                        default=expandvars("$I3_BUILD/ice-models/resources/models/angsens/as.h2-50cm"),
                        type=str, required=False,
                        help="Location of hole ice param files")
    parser.add_argument("--MCTreeName", dest="mctreename",
                        default="I3MCTree", type=str, required=False,
                        help="Name of MCTree frame object")
    parser.add_argument("--KeepEmptyEvents", dest="keepemptyevents",
                        default=False, action="store_true", required=False,
                        help="Don't discard events with no MCPEs")
    parser.add_argument("--HistogramFilename", dest="histogramfilename",
                        default=None, type=str, required=False,
                        help='Histogram filename.')
    parser.add_argument("--EnableHistogram", dest="enablehistogram",
                        default=False, action="store_true", required=False,
                        help='Write a SanityChecker histogram file.')
    parser.add_argument("--no-PropagateMuons", dest="propagatemuons",
                        default=True, action="store_false", required=False,
                        help='Run PROPOSAL to do in-ice propagation')
    parser.add_argument("--PROPOSALParams", dest="proposalparams",
                        default=dict(), type=json.loads, required=False,
                        help='any other parameters for proposal')
    parser.add_argument("--TempDir", dest="tempdir",
                        default=None, type=str, required=False,
                        help='Temporary working directory with the ice model')


def configure_tray(tray, params, stats, logger):
    """
    Configures the I3Tray instance: adds modules, segments, services, etc.

    Args:
        tray (I3Tray): the IceProd tray instance
        params (dict): command-line arguments (and default values)
                            referenced as dict entries; see add_args()
        stats (dict): dictionary that collects run-time stats
        logger (logging.Logger): the logger for this script
    """
    if params['propagatemuons']:
        if params['usegslrng']:
            randomServiceForPropagators = phys_services.I3GSLRandomService(seed=params['seed'] * params['nproc'] + params['procnum'])
        else:
            randomServiceForPropagators = phys_services.I3SPRNGRandomService(seed=params['seed'], nstreams=params['nproc'] * 2, streamnum=params['nproc'] + params['procnum'])

        tray.context['I3PropagatorRandomService'] = randomServiceForPropagators
        tray.AddModule("Rename", "rename_corsika_mctree", Keys=['I3MCTree', 'I3MCTree_preMuonProp'])
        tray.AddSegment(segments.PropagateMuons, 'propagator',
                        RandomService=randomServiceForPropagators,
                        **params['proposalparams'])

    tray.AddSegment(segments.PPCTraySegment, "ppc_photons",
                    gpu=params['gpu'],
                    usegpus=params['usegpus'],
                    UnshadowedFraction=params['efficiency'],
                    DOMOversizeFactor=params['oversize'],
                    IceModelLocation=params['icemodellocation'],
                    HoleIceParameterization=params['holeiceparametrization'],
                    IceModel=params['icemodel'],
                    volumecyl=params['volumecyl'],
                    gpulib=params['gpulib'],
                    InputMCTree=params['mctreename'],
                    keep_empty_events=params['keepemptyevents'],
                    MCPESeriesName=params['photonseriesname'],
                    tempdir=params['tempdir'])

    if params['runmphitfilter']:
        tray.AddModule("MPHitFilter", "hitfilter",
                       HitOMThreshold=1,
                       RemoveBackgroundOnly=False,
                       I3MCPESeriesMapName=params['photonseriesname'])

    if params['enablehistogram'] and params['histogramfilename']:
        tray.AddModule(ProductionHistogramModule,
                       Histograms=[I3MCPEModule],
                       OutputFilename=params['histogramfilename'])


def main():
    """
     GPU Photon propagation
    """
    # Get Params
    parser = argparse.ArgumentParser(description="PPC script")
    add_args(parser)
    params = vars(parser.parse_args())  # dict()

    # Process Params
    inputfilenamelist = [params['gcdfile']] + params['inputfilelist']

    # Execute Tray
    summary = RunI3Tray(params, configure_tray, "PPC",
                        summaryfile=params['summaryfile'],
                        inputfilenamelist=inputfilenamelist,
                        outputfile=params['outputfile'])


if __name__ == "__main__":
    main()
