#!/usr/bin/env python

"""
Add background coincidences to signal MC
"""

from os.path import expandvars
from I3Tray import I3Tray, I3Units
from icecube import icetray, dataclasses, simclasses, dataio
from icecube.icetray import I3Frame
from icecube.simprod import segments
from icecube.simprod.util import simprodtray
from icecube.simprod.util.simprodtray import RunI3Tray
import argparse
import json
from icecube import earthmodel_service, PROPOSAL, cmc, phys_services
from icecube import polyplopia
from icecube.production_histograms import ProductionHistogramModule
from icecube.production_histograms.histogram_modules.simulation.mcpe_module import I3MCPEModule


def add_args(parser):
    """
    Args:
        parser (argparse.ArgumentParser): the command-line parser
    """
    simprodtray.add_argument_gcdfile(parser)
    simprodtray.add_argument_inputfile(parser)
    simprodtray.add_argument_outputfile(parser)

    simprodtray.add_argument_nproc(parser)
    simprodtray.add_argument_procnum(parser)
    simprodtray.add_argument_seed(parser)
    simprodtray.add_argument_usegslrng(parser)
    
    parser.add_argument("--backgroundfile", dest="backgroundfile",
                        default='', type=str, required=False,
                        help='Background filename')
    parser.add_argument("--backgroundrate", dest="backgroundrate",
                        default=float('nan'), type=float, required=False,
                        help='Background rate (Hz) (don\'t use with corsika)')
    parser.add_argument("--mctype", dest="mctype",
                        default='corsika', type=str, required=False,
                        help='Type of primary simulation')
    parser.add_argument("--PROPOSALParams", dest="proposalparams",
                        default=dict(), type=json.loads, required=False,
                        help='any other parameters for proposal')
    parser.add_argument("--GPU", dest="gpu",
                        default=None, type=str, required=False,
                        help="Graphics Processing Unit number (shoud default to environment if None)")
    parser.add_argument("--no-UseGPUs", dest="usegpus",
                        default=True, action="store_false", required=False,
                        help="Use Graphics Processing Unit for photon propagation.")
    parser.add_argument("--UsePPC", dest="useppc",
                        default=False, action="store_true", required=False,
                        help="Use PPC for photon propagation instead of CLSim.")
    parser.add_argument("--oversize", dest="oversize",
                        default=5, type=int, required=False,
                        help="over-R: DOM radius oversize scaling factor")
    parser.add_argument("--holeiceparametrization", dest="holeiceparametrization",
                        default=expandvars("$I3_SRC/ice-models/resources/models/angsens/as.h2-50cm"),
                        type=str, required=False,
                        help="Location of hole ice param files")
    parser.add_argument("--efficiency", dest="efficiency",
                        default=[0.99], type=simprodtray.float_comma_list, required=False,
                        help="overall DOM efficiency correction")
    parser.add_argument("--IceModelLocation", dest="icemodellocation",
                        default=expandvars("$I3_BUILD/ice-models/resources/models"),
                        type=str, required=False,
                        help="Location of ice model param files")
    parser.add_argument("--IceModel", dest="icemodel",
                        default="spice_3.2", type=str, required=False,
                        help="ice model subdirectory")
    parser.add_argument("--PhotonSeriesName", dest="photonseriesname",
                        default="I3MCPESeriesMap", type=str, required=False,
                        help="Photon Series Name")
    parser.add_argument("--HistogramFilename", dest="histogramfilename",
                        default=None, type=str, required=False,
                        help='Histogram filename.')
    parser.add_argument("--TimeWindow", dest="timewindow",
                        default=40. * I3Units.microsecond, type=float, required=False,
                        help='Coincidence time window')


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
    tray.AddSegment(segments.PolyplopiaPhotons, "coincifypes",
                    RandomService=tray.context['I3RandomService'],
                    mctype=params['mctype'],
                    mctree_name="I3MCTree",
                    bgfile=params['backgroundfile'],
                    GCDFile=params['gcdfile'],
                    timewindow=params['timewindow'],
                    GPU=params['gpu'],
                    UseGPUs=params['usegpus'],
                    UsePPC=params['useppc'],
                    IceModel=params['icemodel'],
                    IceModelLocation=params['icemodellocation'],
                    DOMOversizeFactor=params['oversize'],
                    HoleIceParameterization=params['holeiceparametrization'],
                    Efficiency=params['efficiency'],
                    PhotonSeriesName=params['photonseriesname'],
                    PROPOSALParams=params['proposalparams'],
                    rate=params['backgroundrate'] * I3Units.hertz)

    if params['histogramfilename']:
        tray.AddModule(ProductionHistogramModule,
                       Histograms=[I3MCPEModule],
                       OutputFilename=params['histogramfilename'])


def main():
    """
    Add background coincidences to signal MC
    """
    # Get Params
    parser = argparse.ArgumentParser(description="PolyplopiaModule script")
    add_args(parser)
    params = vars(parser.parse_args())  # dict()

    # Execute Tray
    summary = RunI3Tray(params, configure_tray, "PolyplopiaModule",
                        inputfilenamelist=[params['gcdfile'], params['inputfile']],
                        outputfile=params['outputfile'],
                        seed=params['seed'],
                        nstreams=params['nproc'],
                        streamnum=params['procnum'],
                        usegslrng=params['usegslrng'])


if __name__ == "__main__":
    main()
