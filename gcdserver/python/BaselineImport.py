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


"""
Import DOM waveform baseline data
"""


def importFile(geoDB, inserter, inputFile):
    
    # Parse the XML tree
    tree = ElementTree.parse(inputFile)
    root = tree.getroot()
    if root.tag != "baselines":
        raise Exception("Unable to find baselines tag") 

    # Get the document date
    recordDate = (root.find('date')).text
    recordTime = (root.find('time')).text
    date = str(datetime.strptime("%s %s" % (recordDate, recordTime),
                                 "%Y-%m-%d %H:%M:%S"))

    # Read data for all DOMs
    for e in root.findall('dom'):
        string = int(e.attrib['StringId'])
        position = int(e.attrib['TubeId'])
        name = geoDB.omKeyMap(string, position)
        o = C.BeaconBaseline()
        o[C.Keys.DATE] = date
        o[C.Keys.FADC_BASELINE] = float((e.find('FADC')).text)
        for ch in range(C.ATWDConstants.N_CHANNELS):
            elementName = 'ATWDChipAChan%d' % ch
            o.setATWDBaseline(0, ch, float((e.find(elementName)).text))
            elementName = 'ATWDChipBChan%d' % ch
            o.setATWDBaseline(1, ch, float((e.find(elementName)).text))
        result = G.DataObject(name, C.ObjectType.BEACON_BASELINES, o.getdict())
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
