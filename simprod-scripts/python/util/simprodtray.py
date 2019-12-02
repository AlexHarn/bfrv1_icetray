#!/usr/bin/env python

"""
 Generic SimProd IceTray Execution and Utilities
"""

import logging
from icecube import icetray, dataio, phys_services, dataclasses
from I3Tray import I3Tray
from icecube.simprod.util import WriteI3Summary, ReadI3Summary
import os

# Set up default/root logging level
logging.basicConfig()
rootLogger = logging.getLogger('')
rootLogger.setLevel(logging.DEBUG)


def _add_i3_summary_service(tray, summaryfile, summaryin):
    if not summaryin is None:
        summary = summaryin
    elif os.path.exists(summaryfile):
        summary = ReadI3Summary(summaryfile)
    else:
        summary = dataclasses.I3MapStringDouble()
    tray.context['I3SummaryService'] = summary


def _add_i3_random_service(tray, usegslrng, seed, nstreams, streamnum):
    if (nstreams is None) or (streamnum is None):
        rand = phys_services.I3GSLRandomService(seed=seed)
    elif usegslrng:
        rand = phys_services.I3GSLRandomService(seed=seed * nstreams + streamnum)
    else:
        rand = phys_services.I3SPRNGRandomService(seed=seed, nstreams=nstreams, streamnum=streamnum)
    tray.context['I3RandomService'] = rand


def _add_i3_writer(tray, outputfile, outputstreams, outputskipkeys):
    if outputskipkeys:
        tray.AddModule("I3Writer", "writer", filename=outputfile,
                       Streams=outputstreams, SkipKeys=outputskipkeys)
    else:
        tray.AddModule("I3Writer", "writer", filename=outputfile,
                       Streams=outputstreams)


def _execute(tray, executionmaxcount):
    #print(tray)
    if executionmaxcount:
        tray.Execute(executionmaxcount)
    else:
        tray.Execute()


def _save_stats(tray, summary, stats):
    for k in tray.Usage():
        stats[str(k.key()) + ":usr"] = k.data().usertime
        summary[str(k.key()) + ":usr"] = k.data().usertime

        stats[str(k.key()) + ":sys"] = k.data().systime
        summary[str(k.key()) + ":sys"] = k.data().systime

        stats[str(k.key()) + ":ncall"] = k.data().ncall
        summary[str(k.key()) + ":ncall"] = k.data().ncall


def _print_stats(stats, logger, simulationname):
    header = "Stats produced by {0}:".format(simulationname)
    logger.info(header)
    logger.info("-" * len(header))
    for k, v in stats.items():
        logger.info("%s,%s" % (k, v))


OUTPUT_STREAMS = [icetray.I3Frame.TrayInfo,
                  icetray.I3Frame.DAQ,
                  icetray.I3Frame.Stream('S'),
                  icetray.I3Frame.Stream('M')]


def RunI3Tray(params, configure_tray, simulationname, stats=dict(),
              summaryfile="", summaryin=None, doprintstats=True,
              inputfilenamelist=None, outputfile="", outputskipkeys=None,
              outputstreams=OUTPUT_STREAMS, executionmaxcount=None,
              seed=None, nstreams=None, streamnum=None, usegslrng=False):
    """
    Creates an I3Tray. Adds I3SummaryService context, I3RandomService context,
        I3Reader module, and I3Writer module. Calls the passed configure_tray 
        function for further tray configuration. Then, executes the tray, 
        writes out summary to file, and prints stats/tray usage.

    Args:
        params
            (dict) : parameters; forwarded to configure_tray function
        configure_tray
            (function) : the *function* called to further configure the tray; signature: (tray, params, stats, logger)
        simulationname
            (str) : the name of the simulation, used for logger
        stats
            (dict) : the stats dict
        summaryfile
            (str) : the filename to read/write the summary; if "", I3SummaryService is not added.
        summaryin
            (I3MapStringDouble) : the I3 summary inputted to I3SummaryService; if None, summary is read in from summaryfile 
        doprintstats
            (bool) : toggles whether to write out stats
        inputfilenamelist
            (list) : I3Reader module files; if None, doesn't add I3Reader
        outputfile
            (str) : I3Writer module file; if "", doesn't add I3Writer
        outputskipkeys
            (list) : I3Writer module skipkeys list
        outputstreams
            (list) : I3Writer streams; default OUTPUT_STREAMS
        executionmaxcount
            (int) : I3Tray exection max count; default None
        seed
            (int) : I3RandomService rng seed
        nstreams
            (int) : I3RandomService number of streams
        streamnum
            (int) : I3RandomService stream number
        usegslrng
            (bool) : toggles whether to use I3GSLRandomService or I3SPRNGRandomService for I3RandomService
    """
    logger = logging.getLogger(simulationname)
    for param, value in sorted(params.items()):
        logger.info("{0}:{1}".format(param, value))

    # Set Up Tray
    icetray.I3Logger.global_logger.set_level(icetray.I3LogLevel.LOG_DEBUG)
    tray = I3Tray()

    # Configure Tray
    logger.info("Configuring IceProd: {0}".format(simulationname))
    if summaryfile:
        _add_i3_summary_service(tray, summaryfile, summaryin)
    if inputfilenamelist:
        tray.AddModule("I3Reader", "reader", filenamelist=inputfilenamelist)
    if not seed is None:
        _add_i3_random_service(tray, usegslrng, seed, nstreams, streamnum)
    configure_tray(tray, params, stats, logger)
    if outputfile:
        _add_i3_writer(tray, outputfile, outputstreams, outputskipkeys)

    # Execute Tray
    logger.info("Executing {0}".format(simulationname))
    #logger.info("\n{}".format(tray))
    _execute(tray, executionmaxcount)

    # Summarize
    summaryout = tray.context['I3SummaryService']
    if summaryout:
        _save_stats(tray, summaryout, stats)
        if summaryfile:
            WriteI3Summary(summaryout, summaryfile)

    # Print Stats and Tray Usage
    if doprintstats:
        if stats:
            _print_stats(stats, logger, simulationname)
        tray.PrintUsage()

    # Free Memory
    del tray

    return summaryout


# split up a comma-delimited string into list
def _comma_list(arg_list, type):
    return [type(a) for a in arg_list.split(',')]

def int_comma_list(arg_list):
    return _comma_list(arg_list, int)

def float_comma_list(arg_list):
    return _comma_list(arg_list, float)

def str_comma_list(arg_list):
    return _comma_list(arg_list, str)
    

# common arguments    
def add_argument_gcdfile(parser):
    parser.add_argument("--gcdfile", dest="gcdfile",
                        default='', type=str, required=True,
                        help='GeoCalibDetStatus filename')
                        
def add_argument_inputfile(parser):
    parser.add_argument("--inputfile", dest="inputfile",
                        default='', type=str, required=True,
                        help='Input filename')
               
def add_argument_outputfile(parser):         
    parser.add_argument("--outputfile", dest="outputfile",
                        default='', type=str, required=True,
                        help='Output filename')

def add_argument_inputfilelist(parser):
    parser.add_argument("--inputfilelist", dest="inputfilelist",
                        default=[], type=str_comma_list, required=True,
                        help='List of input filenames')

def add_argument_nproc(parser):
    parser.add_argument("--nproc", dest="nproc",
                        default=1, type=int, required=False,
                        help='Number of jobs (Number of RNG streams)')

def add_argument_procnum(parser):
    parser.add_argument("--procnum", dest="procnum",
                        default=0, type=int, required=False,
                        help='job number (RNG stream number)')

def add_argument_seed(parser):
    parser.add_argument("--seed", dest="seed",
                        default=0, type=int, required=False,
                        help='RNG seed')

def add_argument_usegslrng(parser):
    parser.add_argument("--UseGSLRNG", dest="usegslrng",
                        default=False, action="store_true", required=False,
                        help="Use I3GSLRandomService")
