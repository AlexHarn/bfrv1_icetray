#!/usr/bin/env python

"""
File I3OmDbRunImport.py:  Import calibration for one run from I3OmDb to Mongo
"""

import sys
import copy
from contextlib import contextmanager

import icecube.gcdserver.Geometry as G
import icecube.gcdserver.Calibration as C
from icecube.gcdserver.OptionParser import GCDOptionParser
from icecube.gcdserver.I3OmDb import I3OmDb, connectRepeated
from icecube.gcdserver.MongoDB import fillBlobDB, getDB
from icecube.gcdserver.I3MS import calDBInserter
from icecube.gcdserver.I3Live import getLiveRunData, I3LIVE_USER, I3LIVE_PASS


SNOW_DEPTH_TYPE_ID = 65 
VEMCAL_TYPE_ID = 91
BASELINE_TYPE_ID = 102
NOISE_RATE_TYPE_ID = 101
CHARGE_OM_TYPE_ID = 52
CHARGE_OM_ATWD_TYPE_ID = 53
CHARGE_OM_ATWD_CHANNELS_TYPE_ID = 56
ORIGIN_DATE = "2010-01-01 00:00:01" # Year of IC79
STOP_DATE = "2018-01-01 00:00:01"


@contextmanager
def live_db_context(hostname):
    connection = connectRepeated(I3LIVE_USER, hostname, passwd=I3LIVE_PASS)
    cursor = connection.cursor()
    cursor.execute('USE live;')
    try:
        yield cursor
        try:
            connection.commit()
        except:
            connection.rollback()
            raise
    finally:
        connection.close()


def getNextRunNumber(calTime, hostname):
    with live_db_context(hostname) as curs:
        sql = ("SELECT * FROM livedata_run where tStart>='%s' order "
               "by tStart limit 1" % calTime)
        curs.execute(sql)
        return curs.fetchone()[1]


def makeMonotonic(calDateArr):
    out = []
    for (calId, date) in calDateArr:
        if len([x for x in calDateArr if x[0] > calId and x[1] < date]) > 0:
            print "Skipping calID %s date %s" % (calId, date)
        else:
            out.append((calId, date))
    return out


def getCalIdDates(omdb, typeID):
    with omdb.db_context() as curs:
        sql = ("SELECT * FROM CalibrationDetail where TypeID=%d AND "
               "ValidityStartDate >= '%s' AND ValidityStartDate < '%s'" % 
               (typeID, str(ORIGIN_DATE), str(STOP_DATE)))
        curs.execute(sql)
        # Need to not process entries that were superseded
        return makeMonotonic([(row[1], row[7]) for row in curs.fetchall()])


def doCalImport(db, geoDB, omdb, i3LiveHostName,
                typeId, tableName, processRows):
    calIdDates = getCalIdDates(omdb, typeId)
    with omdb.db_context() as curs:
        for (calId, date) in calIdDates:
            runNumber = getNextRunNumber(date, i3LiveHostName)
            print ("Importing ID %s, run %s, table %s" % 
                                   (calId, runNumber, tableName))
            with calDBInserter(db, runNumber) as calInserter:
                curs.execute("SELECT * FROM %s where "
                             "CaId = %s" % (tableName, calId))
                for result in processRows(geoDB, date, curs.fetchall()):
                    calInserter.insert(result)
                calInserter.commit()


def doCalImportRow(db, geoDB, omdb, i3LiveHostName,
                   typeId, tableName, processRow):
    
    def processRows(geoDB, date, rows):
        results = []
        for row in rows:
            results.extend(processRow(geoDB, date, row))
        return results

    doCalImport(db, geoDB, omdb, i3LiveHostName,
                typeId, tableName, processRows)


def importSnowDepthRow(geoDB, date, row):
    stringID = int(row[2])
    tankLabel = row[3]
    name = G.getTankName(stringID, tankLabel)
    result = G.DataObject(name, C.ObjectType.SNOW_HEIGHT)
    result.data[C.Keys.SNOW_HEIGHT] = float(row[4])
    result.data[C.Keys.DATE] = str(date)
    return [result]


def importVEMCalRow(geoDB, date, row):
    stringID = int(row[2])
    positionID = int(row[3])
    name = geoDB.omKeyMap(stringID, positionID)
    result = G.DataObject(name, C.ObjectType.VEMCAL)
    result.data[C.Keys.DATE] = str(date)
    result.data[C.Keys.PE_PER_VEM] = float(row[4])
    result.data[C.Keys.MUON_PEAK_WIDTH] = float(row[5])
    result.data[C.Keys.HG_LG_CROSSOVER] = float(row[7])
    result.data[C.Keys.CORR_FACTOR] = float(row[6])
    return [result]


def importBaselineRow(geoDB, date, row):
    stringID = int(row[2])
    positionID = int(row[3])
    name = geoDB.omKeyMap(stringID, positionID)
    o = C.BeaconBaseline()
    o[C.Keys.DATE] = str(date)
    o[C.Keys.FADC_BASELINE] = float(row[10])
    o.setATWDBaseline(0, 0, float(row[4]))
    o.setATWDBaseline(0, 1, float(row[5]))
    o.setATWDBaseline(0, 2, float(row[6]))
    o.setATWDBaseline(1, 0, float(row[7]))
    o.setATWDBaseline(1, 1, float(row[8]))
    o.setATWDBaseline(1, 2, float(row[9]))
    result = G.DataObject(name, C.ObjectType.BEACON_BASELINES, o.getdict())
    return [result]


def importNoiseRateRow(geoDB, date, row):
    stringID = int(row[2])
    positionID = int(row[3])
    name = None
    try:
        name = geoDB.omKeyMap(stringID, positionID)
    except KeyError:
        print ("Noise rate found for unknown "
               "DOM (%d, %d).  Skipping." % (stringID, positionID))
        return []
    result = G.DataObject(name, C.ObjectType.NOISE_RATE)
    result.data[C.Keys.NOISE_RATE] = float(row[4])
    return [result]


globalDOMCal = {}


def keepGlobals(name, date, temperature, version):
    if date not in globalDOMCal:
        globalDOMCal[date] = {}
    globalDOMCal[date][name] = {}
    globalDOMCal[date][name]["temperature"] = temperature
    globalDOMCal[date][name]["version"] = version


def finishDomCalResult(result, name, date):
    try:
        result.data[C.Keys.DATE] = date
        result.data[C.Keys.VERSION] = globalDOMCal[date][name]["version"]
        result.data[C.Keys.TEMPERATURE] = \
                                  globalDOMCal[date][name]["temperature"]
    except KeyError:
        print "Test key: %s" % date
        print "Keys: %s" % [x for x in globalDOMCal.keys()]
        sys.exit(1)
        # Handle data in ChargeOmATWDChannels table
        # without a corresponding entry in ChargeOm
        print "DOM %s has ATWD calibration but no entry in ChargeOm" % name
    return copy.deepcopy(result)


def importChargeOmRow(geoDB, date, row):
    stringID = int(row[2])
    positionID = int(row[3])
    name = geoDB.omKeyMap(stringID, positionID)
    keepGlobals(name, str(date), float(row[23]), row[25])

    results = []
    o = C.AmplifierCal()
    o.setGain(0, float(row[4]))
    o.setGain(1, float(row[5]))
    o.setGain(2, float(row[6]))
    result = G.DataObject(name, C.ObjectType.AMP_CAL)
    result.data = o.getdict()
    results.append(finishDomCalResult(result, name, str(date)))
            
    result = G.DataObject(name, C.ObjectType.FADC_GAIN_CAL)
    result.data[C.Keys.GAIN] = float(row[10])
    results.append(finishDomCalResult(result, name, str(date)))

    result = G.DataObject(name, C.ObjectType.FADC_DELTA_T_CAL)
    result.data[C.Keys.DELTA_T] = float(row[12])
    results.append(finishDomCalResult(result, name, str(date)))
            
    result = G.DataObject(name, C.ObjectType.FADC_BASELINE)
    result.data[C.Keys.INTERCEPT] = float(row[14])
    result.data[C.Keys.SLOPE] = float(row[15])
    result.data[C.Keys.REG_COEFF] = float(row[16])
    results.append(finishDomCalResult(result, name, str(date)))

    result = G.DataObject(name, C.ObjectType.PMT_TRANSIT_TIME_CAL)
    result.data[C.Keys.INTERCEPT] = float(row[17])
    result.data[C.Keys.SLOPE] = float(row[18])
    result.data[C.Keys.REG_COEFF] = float(row[19])
    results.append(finishDomCalResult(result, name, str(date)))

    result = G.DataObject(name, C.ObjectType.GAIN_CAL)
    result.data[C.Keys.INTERCEPT] = float(row[20])
    result.data[C.Keys.SLOPE] = float(row[21])
    result.data[C.Keys.REG_COEFF] = float(row[22])
    results.append(finishDomCalResult(result, name, str(date)))
            
    result = G.DataObject(name, C.ObjectType.FRONT_END_IMPEDANCE)
    result.data[C.Keys.FE_IMPEDANCE] = float(row[24])
    results.append(finishDomCalResult(result, name, str(date)))

    o = C.ATWDDeltaTCal()
    o.setDeltaT(0, float(row[26]))
    o.setDeltaT(1, float(row[27]))
    result = G.DataObject(name, C.ObjectType.ATWD_DELTA_T_CAL)
    result.data = o.getdict()
    results.append(finishDomCalResult(result, name, str(date)))

    result = G.DataObject(name, C.ObjectType.SPE_DISC_CAL)
    result.data[C.Keys.SLOPE] = float(row[28])
    result.data[C.Keys.INTERCEPT] = float(row[29])
    result.data[C.Keys.REG_COEFF] = 0
    results.append(finishDomCalResult(result, name, str(date)))

    result = G.DataObject(name, C.ObjectType.MPE_DISC_CAL)
    result.data[C.Keys.SLOPE] = float(row[30])
    result.data[C.Keys.INTERCEPT] = float(row[31])
    result.data[C.Keys.REG_COEFF] = 0
    results.append(finishDomCalResult(result, name, str(date)))
            
    result = G.DataObject(name, C.ObjectType.PMT_DISC_CAL)
    result.data[C.Keys.SLOPE] = float(row[32])
    result.data[C.Keys.INTERCEPT] = float(row[33])
    result.data[C.Keys.REG_COEFF] = float(row[34])
    results.append(finishDomCalResult(result, name, str(date)))
    return results


def importChargeOmAtwdRows(geoDB, date, rows):
    results = {}
    for row in rows:
        stringID = int(row[2])
        positionID = int(row[3])
        name = geoDB.omKeyMap(stringID, positionID)
        if name not in results:
            results[name] = C.ATWDFrequencyCal()
        results[name].setFit(int(row[4]), float(row[5]), float(row[6]),
                                          float(row[7]), float(row[8]))

    ret = []
    for (name, data) in results.iteritems():
        result = G.DataObject(name, C.ObjectType.ATWD_FREQ_CAL)
        result.data = data.getdict()
        ret.append(finishDomCalResult(result, name, str(date)))
    return ret


def importChargeOmAtwdChannels(geoDB, date, rows):
    results = {}
    for row in rows:
        stringID = int(row[2])
        positionID = int(row[3])
        name = geoDB.omKeyMap(stringID, positionID)
        if name not in results:
            results[name] = C.ATWDCalibration()
        for i in range(128):
            results[name].setSlope(int(row[4]), int(row[5]),
                                       i, float(row[i+134]))

    ret = []
    for (name, data) in results.iteritems():
        result = G.DataObject(name, C.ObjectType.ATWD_CAL)
        result.data = data.getdict()
        ret.append(finishDomCalResult(result, name, str(date)))
    return ret


def doImport(db, userName, sqlHost, i3LiveHost):
    geoDB = fillBlobDB(db, run=None)
    omdb = I3OmDb(host=sqlHost, user=userName)
    doCalImportRow(db, geoDB, omdb, i3LiveHost, SNOW_DEPTH_TYPE_ID,
            "IceTopStationTankSnow", importSnowDepthRow)
    doCalImportRow(db, geoDB, omdb, i3LiveHost, VEMCAL_TYPE_ID,
            "VEMCalibOm", importVEMCalRow)
    doCalImportRow(db, geoDB, omdb, i3LiveHost, BASELINE_TYPE_ID,
            "ChargeBeaconBaselineOm", importBaselineRow)
    doCalImportRow(db, geoDB, omdb, i3LiveHost, NOISE_RATE_TYPE_ID,
            "PMTInfo", importNoiseRateRow)
    doCalImportRow(db, geoDB, omdb, i3LiveHost, CHARGE_OM_TYPE_ID,
            "ChargeOm", importChargeOmRow)
    doCalImport(db, geoDB, omdb, i3LiveHost, CHARGE_OM_ATWD_TYPE_ID,
            "ChargeOmAtwd", importChargeOmAtwdRows)
    doCalImport(db, geoDB, omdb, i3LiveHost, CHARGE_OM_ATWD_CHANNELS_TYPE_ID,
            "ChargeOmAtwdChannels ", importChargeOmAtwdChannels)


def main():
    parser = GCDOptionParser()
    parser.add_option("-d", "--sqlHost", dest="sqlHost",
                      help="I3OmDb MySQL host name")
    parser.add_option("-u", "--user", dest="user", default="www",
                      help="MySQL user name")
    (options, args) = parser.parse_args()
    if options.sqlHost == None:
        print "I3OmDb MySQL host not specified"
        parser.print_help()
        sys.exit(1)
    doImport(getDB(options.dbhost, options.dbuser, options.dbpass),
             options.user, options.sqlHost, options.i3livehost)

if __name__ == "__main__":
    main()
