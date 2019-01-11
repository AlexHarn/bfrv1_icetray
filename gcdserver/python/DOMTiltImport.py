#!/usr/bin/env python

import os
import sys
import json

import icecube.gcdserver.Geometry as G
import icecube.gcdserver.Calibration as C
from icecube.gcdserver.OptionParser import GCDOptionParser
from icecube.gcdserver.MongoDB import getDB, fillBlobDB
from I3MS import geoDBInserter


def importFile(geoDB, inserter, tiltFile):
    
    # Parse the JSON file
    with open(tiltFile) as file:
        data = json.load(file)
        for (id, vals) in data.iteritems():
            stringID = int(id.split(",")[0])
            positionID = int(id.split(",")[1])
            name = geoDB.omKeyMap(stringID, positionID)
            result = geoDB.deployedNameMap(name)
            result.data.pop(G.Keys.ORIENTATION, None)
            result.data[G.Keys.ORIENTATION_NX] = vals["nx"]
            result.data[G.Keys.ORIENTATION_NY] = vals["ny"]
            result.data[G.Keys.ORIENTATION_NZ] = vals["nz"]
            result.data[G.Keys.ORIENTATION_AZIMUTH] = vals["azimuth"]
            inserter.insert(result)


def doInsert(db, i3msHost, tiltFile):

    # Get BlobDB instance loaded with geometry data
    geoDB = fillBlobDB(db, run=None)

    errCode = 0
    with geoDBInserter(db, i3msHost) as inserter:
        try:
            importFile(geoDB, inserter, os.path.abspath(tiltFile))
        except Exception as e:
            errCode = -1
            print "Unable to import file %s: %s" % (tiltFile, e)
        inserter.commit()
    return errCode


if __name__ == "__main__":
    parser = GCDOptionParser()
    (options, args) = parser.parse_args()
    if len(args) == 0:
        print "No tilt JSON file specified"
        sys.exit(-1)
    if len(args) > 1:
        print "Too many tilt JSON files specified.  Expect one."
        sys.exit(-1)
    errCode = doInsert(getDB(options.dbhost, options.dbuser, options.dbpass),
                       options.i3mshost, args[0])
    sys.exit(errCode)