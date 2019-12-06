#!/usr/bin/env python

import os
import sys
from datetime import datetime
import xml.etree.cElementTree as ElementTree

import icecube.gcdserver.Geometry as G
import icecube.gcdserver.Calibration as C
from icecube.gcdserver.OptionParser import GCDOptionParser
from icecube.gcdserver.MongoDB import getDB, fillBlobDB
from icecube.gcdserver.I3MS import calDBInserter


def importFile(geoDB, inserter, inputFile):
    
    # Parse the XML tree
    tree = ElementTree.parse(inputFile)
    root = tree.getroot()
    if root.tag != "VEMCalibOm":
        raise Exception("Unable to find VEMCalibOm tag") 

    # Get the document date
    date = str(datetime.strptime((root.find('Date')).text,
                                 " %Y-%m-%d %H:%M:%S "))

    # Read data for all DOMs
    for e in root.findall('DOM'):
        string = int((e.find('StringId')).text)
        position = int((e.find('TubeId')).text)
        name = geoDB.omKeyMap(string, position)
        result = G.DataObject(name, C.ObjectType.VEMCAL)
        result.data[C.Keys.DATE] = date
        result.data[C.Keys.PE_PER_VEM] = float((e.find('pePerVEM')).text)
        result.data[C.Keys.MUON_PEAK_WIDTH] = float(
                                            (e.find('muPeakWidth')).text)
        result.data[C.Keys.HG_LG_CROSSOVER] = float(
                                            (e.find('hglgCrossOver')).text)
        result.data[C.Keys.CORR_FACTOR] = float((e.find('corrFactor')).text)
        inserter.insert(result)


def doInsert(db, runValid, i3msHost, files):

    # Get BlobDB instance loaded with geometry data
    geoDB = fillBlobDB(db, run=None)

    errCode = 0
    with calDBInserter(db, runValid, i3msHost) as inserter:
        for file in files:
            try:
                importFile(geoDB, inserter, os.path.abspath(file))
            except Exception as e:
                errCode = -1
                print("Unable to import file %s: %s" % (file, e))
        inserter.commit()
    return errCode


if __name__ == "__main__":
    parser = GCDOptionParser()
    parser.add_option("-r", "--runValid", dest="runValid",
                      help="runValid entry for calibration quantities")
    (options, args) = parser.parse_args()
    if options.runValid == None:
        print("Calibration runValid not specified")
        parser.print_help()
        sys.exit(-1)
    if len(args) == 0:
        print("No VEMCal files specified")
        sys.exit(-1)
    errCode = doInsert(getDB(options.dbhost, options.dbuser, options.dbpass),
                       options.runValid, options.i3mshost, args)
    sys.exit(errCode)
