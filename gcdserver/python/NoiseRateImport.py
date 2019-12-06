#!/usr/bin/env python

import os
import sys
import json

import icecube.gcdserver.Geometry as G
import icecube.gcdserver.Calibration as C
from icecube.gcdserver.OptionParser import GCDOptionParser
from icecube.gcdserver.MongoDB import getDB, fillBlobDB
from icecube.gcdserver.I3MS import calDBInserter


def importFile(geoDB, inserter, inputFile):
    
    # Parse the JSON file
    with open(inputFile) as file:
        data = json.load(file)
        for (id, rate) in data.iteritems():
            stringID = int(id.split(",")[0])
            positionID = int(id.split(",")[1])
            name = geoDB.omKeyMap(stringID, positionID)
            result = G.DataObject(name, C.ObjectType.NOISE_RATE)
            result.data[C.Keys.NOISE_RATE] = rate['rate_hz']
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
        print("No noise rate files specified")
        sys.exit(-1)
    errCode = doInsert(getDB(options.dbhost, options.dbuser, options.dbpass),
                       options.runValid, options.i3mshost, args)
    sys.exit(errCode)
