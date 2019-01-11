#!/usr/bin/env python

import sys

from icecube.gcdserver.OptionParser import GCDOptionParser
from icecube.gcdserver.I3GeometryBuilder import buildI3Geometry
from icecube.gcdserver.I3CalibrationBuilder import buildI3Calibration
from icecube.gcdserver.I3DetectorStatusBuilder import buildI3DetectorStatus
from icecube.gcdserver.I3FlasherSubrunMapBuilder import buildI3FlasherSubrunMap
from icecube.gcdserver.MongoDB import fillBlobDB, getDB
from icecube.gcdserver.I3Live import getLiveRunData, getLiveFlasherData
from icecube.gcdserver.util import setStartStopTime

from icecube import icetray, dataio


def buildGCD(db, i3LiveHost, runNumber, outFilePath):
    """
    Build I3Geometry, I3Calibration, and I3DetectorStatus instances
    given a run number.  Query I3Live for configuration/time data.
    """
    runData = getLiveRunData(runNumber, i3LiveHost)
    if runData.startTime is None:
        print "No run data available for run %s" % runNumber
        return

    blobDB = fillBlobDB(db, run=int(runNumber),
                        configuration=runData.configName)

    f = dataio.I3File(outFilePath, 'w')
    g = buildI3Geometry(blobDB)
    c = buildI3Calibration(blobDB)
    d = buildI3DetectorStatus(blobDB, runData)
    setStartStopTime(g, d)
    setStartStopTime(c, d)
    fr = icetray.I3Frame(icetray.I3Frame.Geometry)
    fr['I3Geometry'] = g
    f.push(fr)
    fr = icetray.I3Frame(icetray.I3Frame.Calibration)
    fr['I3Calibration'] = c
    f.push(fr)
    fr = icetray.I3Frame(icetray.I3Frame.DetectorStatus)
    fr['I3DetectorStatus'] = d
    fr['I3FlasherSubrunMap'] = buildI3FlasherSubrunMap(
                                 getLiveFlasherData(runNumber, i3LiveHost))
    f.push(fr)
    f.close()


if __name__ == "__main__":
    parser = GCDOptionParser()
    parser.add_option("-r", "--run", dest="run",
                      help="run number to determine calibration data validity")
    parser.add_option("-o", "--outputFile", dest="outputFile",
                      help="Name of output GCD file")
    (options, args) = parser.parse_args()
    if options.run == None:
        print "Run number not specified"
        parser.print_help()
        sys.exit(1)
    if options.outputFile == None:
        print "Output file not specified"
        parser.print_help()
        sys.exit(1)

    buildGCD(getDB(options.dbhost, options.dbuser, options.dbpass),
             options.i3livehost, int(options.run), options.outputFile)