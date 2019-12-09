#!/usr/bin/env python

import os
from icecube.simprod.segments import ProduceNoiseTriggers
from icecube.simprod.util import ReadI3Summary, WriteI3Summary
from icecube.simprod.util import simprodtray
from icecube.simprod.util.simprodtray import RunI3Tray
import argparse
from icecube.simprod.util import BasicCounter, DAQCounter
from icecube import icetray, phys_services, dataclasses


def add_args(parser):
    """
    Args:
        parser (argparse.ArgumentParser): the command-line parser
    """
    simprodtray.add_argument_gcdfile(parser)
    simprodtray.add_argument_outputfile(parser)
    
    parser.add_argument("--summaryfile", dest="summaryfile",
                        default='summary.json', type=str, required=False,
                        help='JSON Summary filename')
    parser.add_argument("--Detector", dest="detector",
                        default="IC86:2011", type=str, required=False,
                        help="detector to simulate")
    parser.add_argument("--nevents", dest="nevents",
                        default=1000, type=int, required=False,
                        help='Number of events')
    parser.add_argument("--RunId", dest="runid",
                        default=0, type=int, required=False,
                        help="Configure run ID")
    parser.add_argument("--RNGSeed", dest="rngseed",
                        default=0, type=int, required=False,
                        help="RNG seed")


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
    tray.AddSegment(ProduceNoiseTriggers, "noise_triggers",
                    gcd_file=params['gcdfile'],
                    nevents=params['nevents'],
                    run_id=params['runid'])

    tray.AddModule(BasicCounter, "count_triggers",
                   Streams=[icetray.I3Frame.DAQ],
                   name="%s Triggered Events" % params['detector'],
                   Stats=stats)


def main():
    # Get Params
    parser = argparse.ArgumentParser(description="NoiseTriggers script")
    add_args(parser)
    params = vars(parser.parse_args())  # dict()

    # Execute Tray
    summary = RunI3Tray(params, configure_tray, "NoiseTriggers",
                        summaryfile=params['summaryfile'],
                        outputfile=params['outputfile'],
                        outputskipkeys=["I3Triggers", "MCPMTResponseMap", "MCTimeIncEventID"],
                        executionmaxcount=4 + params['nevents'],
                        seed=params['rngseed'])


if __name__ == "__main__":
    main()
