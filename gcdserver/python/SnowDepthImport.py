#!/usr/bin/env python

import os
import sys
from datetime import datetime
import xml.etree.cElementTree as ElementTree

import icecube.gcdserver.Geometry as G
import icecube.gcdserver.Calibration as C
from icecube.gcdserver.OptionParser import GCDOptionParser
from icecube.gcdserver.MongoDB import getDB
from I3MS import calDBInserter


def importFile(inserter, inputFile):
    
    # Parse the XML tree
    tree = ElementTree.parse(inputFile)
    root = tree.getroot()
    if root.tag != "IceTopStationTankSnow":
        raise Exception("Unable to find IceTopStationTankSnow tag") 

    # Get the document date
    date = str(datetime.strptime((root.find('Date')).text,
                                 " %Y-%m-%d %H:%M:%S "))

    # Read data for all DOMs
    for e in root.findall('Tank'):
        string = int((e.find('StringId')).text)
        tankLabel = (e.find('TankLabel')).text
        name = G.getTankName(string, tankLabel)
        result = G.DataObject(name, C.ObjectType.SNOW_HEIGHT)
        result.data[C.Keys.SNOW_HEIGHT] = float((e.find('SnowHeight')).text)
        result.data[C.Keys.DATE] = date
        inserter.insert(result)


def doInsert(db, runValid, i3msHost, files):

    errCode = 0
    with calDBInserter(db, runValid, i3msHost) as inserter:
        for file in files:
            try:
                importFile(inserter, os.path.abspath(file))
            except Exception as e:
                errCode = -1
                print "Unable to import file %s: %s" % (file, e)
        inserter.commit()
    return errCode


if __name__ == "__main__":
    parser = GCDOptionParser()
    parser.add_option("-r", "--runValid", dest="runValid",
                      help="runValid entry for calibration quantities")
    (options, args) = parser.parse_args()
    if options.runValid == None:
        print "Calibration runValid not specified"
        parser.print_help()
        sys.exit(-1)
    if len(args) == 0:
        print "No snow depth files specified"
        sys.exit(-1)
    errCode = doInsert(getDB(options.dbhost, options.dbuser, options.dbpass),
                       options.runValid, options.i3mshost, args)
    sys.exit(errCode)