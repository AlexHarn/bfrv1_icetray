#!/usr/bin/env python

import os
import sys
import json

import icecube.gcdserver.Geometry as G
import icecube.gcdserver.Calibration as C
from icecube.gcdserver.OptionParser import GCDOptionParser
from icecube.gcdserver.MongoDB import getDB, fillBlobDB
from I3MS import calDBInserter


def getFitType(s):
    if s == "ATWD_fit":
        return C.ObjectType.ATWD_SPE_CORR
    if s == "FADC_fit":
        return C.ObjectType.FADC_SPE_CORR
    if s == "JOINT_fit":
        return C.ObjectType.JOINT_SPE_CORR
    raise Exception("Unknown fit type: %s" % s)


def importFile(geoDB, inserter, inputFile):
    
    # Parse the JSON file
    with open(inputFile) as file:
        data = json.load(file)
        for (id, fits) in data.iteritems():
            stringID = int(id.split(",")[0])
            positionID = int(id.split(",")[1])
            name = geoDB.omKeyMap(stringID, positionID)
            for (fit, data) in fits.iteritems():
                result = G.DataObject(name, getFitType(fit))
                result.data[C.Keys.CHI_2] = data["chi2"]
                result.data[C.Keys.ERROR] = data["error"]
                result.data[C.Keys.EXP_NORM] = data["exp_norm"]
                result.data[C.Keys.EXP_SCALE] = data["exp_scale"]
                result.data[C.Keys.GAUSS_MEAN] = data["gaus_mean"]
                result.data[C.Keys.GAUSS_NORM] = data["gaus_norm"]
                result.data[C.Keys.GAUSS_STDDEV] = data["gaus_stddev"]
                result.data[C.Keys.NDF] = data["ndf"]
                result.data[C.Keys.N_ENTRIES] = data["nentries"]
                result.data[C.Keys.VALID] = data["valid"]
                result.data[C.Keys.X_MAX_F] = data["x_max_f"]
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
        print "No SPE correction JSON files specified"
        sys.exit(-1)
    errCode = doInsert(getDB(options.dbhost, options.dbuser, options.dbpass),
                       options.runValid, options.i3mshost, args)
    sys.exit(errCode)