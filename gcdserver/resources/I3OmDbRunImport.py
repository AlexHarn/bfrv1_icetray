#!/usr/bin/env python

"""
File I3OmDbRunImport.py:  Import calibration for one run from I3OmDb to Mongo
"""

import sys

import icecube.gcdserver.Geometry as G
import icecube.gcdserver.Calibration as C
from icecube.gcdserver.OptionParser import GCDOptionParser
from icecube.gcdserver.I3OmDb import I3OmDb
from icecube.gcdserver.MongoDB import fillBlobDB, getDB
from icecube.gcdserver.I3MS import calDBInserter
from icecube.gcdserver.I3Live import getLiveRunData


SNOW_DEPTH_TYPE_ID = 65 
VEMCAL_TYPE_ID = 91
BASELINE_TYPE_ID = 102
NOISE_RATE_TYPE_ID = 101
CHARGE_OM_TYPE_ID = 52
CHARGE_OM_ATWD_TYPE_ID = 53
CHARGE_OM_ATWD_CHANNELS_TYPE_ID = 56

def getCalId(omdb, runStartTime, typeID):
    with omdb.db_context() as curs:
        sql = ("SELECT * FROM CalibrationDetail where TypeID=%d AND "
               "ValidityStartDate <= '%s' order by ValidityStartDate "
               "desc limit 1" % (typeID, str(runStartTime)))
        curs.execute(sql)
        return curs.fetchone()[1]


def importSnowDepth(geoDB, omdb, runStartTime, calInserter):
    calId = getCalId(omdb, runStartTime, SNOW_DEPTH_TYPE_ID)
    with omdb.db_context() as curs:
        curs.execute("SELECT * FROM IceTopStationTankSnow "
                     "where CaId=%d" % calId)
        for row in curs.fetchall():
            stringID = int(row[2])
            tankLabel = row[3]
            name = G.getTankName(stringID, tankLabel)
            result = G.DataObject(name, C.ObjectType.SNOW_HEIGHT)
            result.data[C.Keys.SNOW_HEIGHT] = float(row[4])
            result.data[C.Keys.DATE] = str(runStartTime)
            calInserter.insert(result)


def importVEMCal(geoDB, omdb, runStartTime, calInserter):
    calId = getCalId(omdb, runStartTime, VEMCAL_TYPE_ID)
    with omdb.db_context() as curs:
        curs.execute("SELECT * FROM VEMCalibOm where CaId=%d" % calId)
        for row in curs.fetchall():
            stringID = int(row[2])
            positionID = int(row[3])
            name = geoDB.omKeyMap(stringID, positionID)
            result = G.DataObject(name, C.ObjectType.VEMCAL)
            result.data[C.Keys.DATE] = str(runStartTime)
            result.data[C.Keys.PE_PER_VEM] = float(row[4])
            result.data[C.Keys.MUON_PEAK_WIDTH] = float(row[5])
            result.data[C.Keys.HG_LG_CROSSOVER] = float(row[7])
            result.data[C.Keys.CORR_FACTOR] = float(row[6])
            calInserter.insert(result)


def importBaseline(geoDB, omdb, runStartTime, calInserter):
    calId = getCalId(omdb, runStartTime, BASELINE_TYPE_ID)
    with omdb.db_context() as curs:
        curs.execute("SELECT * FROM ChargeBeaconBaselineOm "
                     "where CaId=%d" % calId)
        for row in curs.fetchall():
            stringID = int(row[2])
            positionID = int(row[3])
            name = geoDB.omKeyMap(stringID, positionID)
            o = C.BeaconBaseline()
            o[C.Keys.DATE] = str(runStartTime)
            o[C.Keys.FADC_BASELINE] = float(row[10])
            o.setATWDBaseline(0, 0, float(row[4]))
            o.setATWDBaseline(0, 1, float(row[5]))
            o.setATWDBaseline(0, 2, float(row[6]))
            o.setATWDBaseline(1, 0, float(row[7]))
            o.setATWDBaseline(1, 1, float(row[8]))
            o.setATWDBaseline(1, 2, float(row[9]))
            result = G.DataObject(name,
                                  C.ObjectType.BEACON_BASELINES, o.getdict())
            calInserter.insert(result)


def importNoiseRate(geoDB, omdb, runStartTime, calInserter):
    calId = getCalId(omdb, runStartTime, NOISE_RATE_TYPE_ID)
    with omdb.db_context() as curs:
        curs.execute("SELECT * FROM PMTInfo where CaId=%d" % calId)
        for row in curs.fetchall():
            stringID = int(row[2])
            positionID = int(row[3])
            name = None
            try:
                name = geoDB.omKeyMap(stringID, positionID)
            except KeyError:
                print ("Noise rate found for unknown "
                       "DOM (%d, %d).  Skipping." % (stringID, positionID))
                continue
            result = G.DataObject(name, C.ObjectType.NOISE_RATE)
            result.data[C.Keys.NOISE_RATE] = float(row[4])
            calInserter.insert(result)


globalDOMCal = {}


def finishDomCalResult(result, name, calInserter, runStartTime):
    try:
        result.data[C.Keys.DATE] = globalDOMCal[name]["date"]
        result.data[C.Keys.VERSION] = globalDOMCal[name]["version"]
        result.data[C.Keys.TEMPERATURE] = globalDOMCal[name]["temperature"]
    except KeyError:
        # Handle data in ChargeOmATWDChannels table
        # without a corresponding entry in ChargeOm
        print "DOM %s has ATWD calibration but no entry in ChargeOm" % name
        result.data[C.Keys.DATE] = str(runStartTime)
        result.data[C.Keys.VERSION] = "UNKNOWN"
        result.data[C.Keys.TEMPERATURE] = 0.
    calInserter.insert(result)


def importChargeOm(geoDB, omdb, runStartTime, calInserter):
    calId = getCalId(omdb, runStartTime, CHARGE_OM_TYPE_ID)
    with omdb.db_context() as curs:
        curs.execute("SELECT * FROM ChargeOm where CaId=%d" % calId)
        for row in curs.fetchall():
            stringID = int(row[2])
            positionID = int(row[3])
            name = geoDB.omKeyMap(stringID, positionID)
            globalDOMCal[name] = {}
            globalDOMCal[name]["temperature"] = float(row[23])
            globalDOMCal[name]["version"] = row[25]
            globalDOMCal[name]["date"] = str(runStartTime)

            o = C.AmplifierCal()
            o.setGain(0, float(row[4]))
            o.setGain(1, float(row[5]))
            o.setGain(2, float(row[6]))
            result = G.DataObject(name, C.ObjectType.AMP_CAL)
            result.data = o.getdict()
            finishDomCalResult(result, name, calInserter, runStartTime)
            
            result = G.DataObject(name, C.ObjectType.FADC_GAIN_CAL)
            result.data[C.Keys.GAIN] = float(row[10])
            finishDomCalResult(result, name, calInserter, runStartTime)

            result = G.DataObject(name, C.ObjectType.FADC_DELTA_T_CAL)
            result.data[C.Keys.DELTA_T] = float(row[12])
            finishDomCalResult(result, name, calInserter, runStartTime)
            
            result = G.DataObject(name, C.ObjectType.FADC_BASELINE)
            result.data[C.Keys.INTERCEPT] = float(row[14])
            result.data[C.Keys.SLOPE] = float(row[15])
            result.data[C.Keys.REG_COEFF] = float(row[16])
            finishDomCalResult(result, name, calInserter, runStartTime)

            result = G.DataObject(name, C.ObjectType.PMT_TRANSIT_TIME_CAL)
            result.data[C.Keys.INTERCEPT] = float(row[17])
            result.data[C.Keys.SLOPE] = float(row[18])
            result.data[C.Keys.REG_COEFF] = float(row[19])
            finishDomCalResult(result, name, calInserter, runStartTime)

            result = G.DataObject(name, C.ObjectType.GAIN_CAL)
            result.data[C.Keys.INTERCEPT] = float(row[20])
            result.data[C.Keys.SLOPE] = float(row[21])
            result.data[C.Keys.REG_COEFF] = float(row[22])
            finishDomCalResult(result, name, calInserter, runStartTime)
            
            result = G.DataObject(name, C.ObjectType.FRONT_END_IMPEDANCE)
            result.data[C.Keys.FE_IMPEDANCE] = float(row[24])
            finishDomCalResult(result, name, calInserter, runStartTime)

            o = C.ATWDDeltaTCal()
            o.setDeltaT(0, float(row[26]))
            o.setDeltaT(1, float(row[27]))
            result = G.DataObject(name, C.ObjectType.ATWD_DELTA_T_CAL)
            result.data = o.getdict()
            finishDomCalResult(result, name, calInserter, runStartTime)

            result = G.DataObject(name, C.ObjectType.SPE_DISC_CAL)
            result.data[C.Keys.SLOPE] = float(row[28])
            result.data[C.Keys.INTERCEPT] = float(row[29])
            result.data[C.Keys.REG_COEFF] = 0
            finishDomCalResult(result, name, calInserter, runStartTime)

            result = G.DataObject(name, C.ObjectType.MPE_DISC_CAL)
            result.data[C.Keys.SLOPE] = float(row[30])
            result.data[C.Keys.INTERCEPT] = float(row[31])
            result.data[C.Keys.REG_COEFF] = 0
            finishDomCalResult(result, name, calInserter, runStartTime)
            
            result = G.DataObject(name, C.ObjectType.PMT_DISC_CAL)
            result.data[C.Keys.SLOPE] = float(row[32])
            result.data[C.Keys.INTERCEPT] = float(row[33])
            result.data[C.Keys.REG_COEFF] = float(row[34])
            finishDomCalResult(result, name, calInserter, runStartTime)


def importChargeOmAtwd(geoDB, omdb, runStartTime, calInserter):
    calId = getCalId(omdb, runStartTime, CHARGE_OM_ATWD_TYPE_ID)
    results = {}
    with omdb.db_context() as curs:
        curs.execute("SELECT * FROM ChargeOmAtwd where CaId=%d" % calId)
        for row in curs.fetchall():
            stringID = int(row[2])
            positionID = int(row[3])
            name = geoDB.omKeyMap(stringID, positionID)
            if name not in results:
                results[name] = C.ATWDFrequencyCal()
            results[name].setFit(int(row[4]), float(row[5]), float(row[6]),
                                              float(row[7]), float(row[8]))

    for (name, data) in results.items():
        result = G.DataObject(name, C.ObjectType.ATWD_FREQ_CAL)
        result.data = data.getdict()
        finishDomCalResult(result, name, calInserter, runStartTime)


def importChargeOmAtwdChannels(geoDB, omdb, runStartTime, calInserter):
    calId = getCalId(omdb, runStartTime, CHARGE_OM_ATWD_CHANNELS_TYPE_ID)
    results = {}
    with omdb.db_context() as curs:
        curs.execute("SELECT * FROM ChargeOmAtwdChannels "
                     "where CaId=%d" % calId)
        for row in curs.fetchall():
            stringID = int(row[2])
            positionID = int(row[3])
            name = geoDB.omKeyMap(stringID, positionID)
            if name not in results:
                results[name] = C.ATWDCalibration()
            for i in range(128):
                results[name].setSlope(int(row[4]), int(row[5]),
                                       i, float(row[i+134]))

    for (name, data) in results.items():
        result = G.DataObject(name, C.ObjectType.ATWD_CAL)
        result.data = data.getdict()
        finishDomCalResult(result, name, calInserter, runStartTime)


def importDOMCal(geoDB, omdb, runStartTime, calInserter):
    importChargeOm(geoDB, omdb, runStartTime, calInserter)
    importChargeOmAtwd(geoDB, omdb, runStartTime, calInserter)
    importChargeOmAtwdChannels(geoDB, omdb, runStartTime, calInserter)


def importRun(db, userName, runNumber, sqlHost, i3LiveHost, i3msHost):
    runData = getLiveRunData(runNumber, i3LiveHost)
    runStartTime = runData.startTime
    if runStartTime is None:
        print "No start time available for run %s" % runNumber
        return
    geoDB = fillBlobDB(db, run=None)
    omdb = I3OmDb(host=sqlHost, user=userName)
    with calDBInserter(db, runNumber, i3msHost) as calInserter:
        importSnowDepth(geoDB, omdb, runStartTime, calInserter)
        importVEMCal(geoDB, omdb, runStartTime, calInserter)
        importBaseline(geoDB, omdb, runStartTime, calInserter)
        importNoiseRate(geoDB, omdb, runStartTime, calInserter)
        importDOMCal(geoDB, omdb, runStartTime, calInserter)
        calInserter.commit()
    print ("Import complete.  Run configuration "
           "%s not imported" % runData.configName)


def main():
    parser = GCDOptionParser()
    parser.add_option("-d", "--sqlHost", dest="sqlHost",
                      help="I3OmDb MySQL host name")
    parser.add_option("-r", "--run", dest="run",
                      help="run to import calibration data")
    parser.add_option("-u", "--user", dest="user", default="www",
                      help="MySQL user name")
    (options, args) = parser.parse_args()
    if options.sqlHost == None:
        print "I3OmDb MySQL host not specified"
        parser.print_help()
        sys.exit(1)
    if options.run == None:
        print "Calibration runValid not specified"
        parser.print_help()
        sys.exit(1)
    importRun(getDB(options.dbhost, options.dbuser, options.dbpass),
              options.user, int(options.run),
              options.sqlHost, options.i3livehost, options.i3mshost)

if __name__ == "__main__":
    main()
