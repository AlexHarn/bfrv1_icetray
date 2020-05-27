#!/usr/bin/env python

"""
File I3OmDbDump.py:  Transfer I3OmDb contents into new mongoDB collections.
"""

import sys

import icecube.gcdserver.Geometry as G
import icecube.gcdserver.Calibration as C
from icecube.gcdserver.OptionParser import GCDOptionParser
from icecube.gcdserver.I3OmDb import I3OmDb
from icecube.gcdserver.MongoDB import getDB
from icecube.gcdserver.I3MS import geoDBInserter, calDBInserter

# A list of DOMs never entered into I3OmDb because they were never
# used.  Add them manually for completeness
unknownDOMs = [
("Balderdash", 7, 42, "e961eeeff5ba", "TP8P3665"),
("Are", 7, 41, "549d926550e2", "UP8P3602"),
("Dead_Tom", 7, 40, "e52acc7a09ed", "TP9P3725"),
("Heavenly", 7, 39, "b80eef4219e3", "UP8P3544"),
("Battle_Cry", 7, 38, "36a1c1fa2101", "TP8P3661"),
("Foo_Foo", 7, 37, "eb790ff351bc", "UP9P3732"),
("Pepe_the_King_Prawn", 7, 45, "7f7030c309ea", "UP9P3728"),
("Cave_Lion", 79, 56, "9bcf05fe4c28", "TP9P3827"),
("Wonambi", 79, 55, "44e9bf0ebdac", "UP9P3862")]


unknownDOMMap = {}
for dom in unknownDOMs:
    unknownDOMMap[dom[4]] = {"name": dom[0],
                             "string": dom[1],
                             "position": dom[2],
                             "mbid": dom[3]}


# Strings with HQE doms
hqeStrings = [36, 43, 79, 80, 81, 82, 83, 84, 85, 86]


# Strings that are fully HQE
fullHQE = [82, 83, 84, 85, 86]


# IceTop stations with Tyvek liners.  We never entered liner type into the DB!
tyvekStations = [21, 29, 30, 39]


def isIceAct(string, pos):
    """
    IceACT board is string 0, position 1
    """
    return string == 0 and pos == 1


def isScintillator(string, pos):
    """
    Scintillators are listed as position 65 & 66 on strings 12 & 62
    """
    return ((pos == 65 or pos == 66) and
            (string == 12 or string == 62))


def isAMANDASync(string, pos):
    """
    AMANDA synchronization board is string 0, position 91
    """
    return string == 0 and pos == 91


def isAMANDATrig(string, pos):
    """
    AMANDA trigger board is string 0, position 92
    """
    return string == 0 and pos == 92


def dumpI3OmDb(geoInserter, calInserter, omdb):
    # Fetch geometryOM data.  First find the latest calibration.
    # 2D map by [stringID][position]
    geometryOM = {}
    with omdb.db_context() as curs:
        curs.execute("SELECT distinct CaId FROM GeometryOm "
                     "order by CaId desc limit 1")
        calId = curs.fetchone()[0]
        curs.execute("SELECT * FROM GeometryOm where CaId=%d" % calId)
        for row in curs.fetchall():
            stringID = row[2]
            if stringID not in geometryOM:
                geometryOM[stringID] = {}
            stringData = geometryOM[stringID]
            stringData[row[3]] = {"x": row[4],
                                  "y": row[5],
                                  "z": row[6],
                                  "area": row[7],
                                  "orientation": row[8]}

    # Fetch production data, use DOMID as the key
    prod = {}
    with omdb.db_context() as curs:
        curs.execute("SELECT * FROM DomName")
        for row in curs.fetchall():
            prod[row[1]] = {"mbid": str(row[0]).lower(), "name": row[2]}
    # Add the unknown DOMs
    for dom in unknownDOMMap:
        if dom in prod:
            raise Exception("We already have a prod entry for %s" % dom)
        prod[dom] = {"mbid": str(unknownDOMMap[dom]["mbid"]).lower(),
                     "name": unknownDOMMap[dom]["name"]}

    # Serial --> location mapping.  Use mbid as the key
    location = {}
    with omdb.db_context() as curs:
        curs.execute("SELECT * FROM DomLocation")
        for row in curs.fetchall():
            mbid = str(row[0]).lower()
            location[mbid] = {"string": row[1], "position": row[2]}
    # Add the unknown DOMs
    for dom in unknownDOMMap:
        mbid = unknownDOMMap[dom]["mbid"]
        if mbid in location:
            raise Exception("We have a location entry for %s" % mbid)
        location[mbid] = {"string": unknownDOMMap[dom]["string"],
                          "position": unknownDOMMap[dom]["position"]}

    # PMT data.  First get the latest calibration.
    # Map by [stringID][position].
    pmtInfo = {}
    with omdb.db_context() as curs:
        curs.execute("SELECT distinct CaId FROM "
                     "PMTInfo order by CaId desc limit 1")
        calId = curs.fetchone()[0]
        curs.execute("SELECT * FROM PMTInfo where CaId=%d" % calId)
        for row in curs.fetchall():
            stringID = row[2]
            if not stringID in pmtInfo:
                pmtInfo[stringID] = {}
            stringData = pmtInfo[stringID]
            stringData[row[3]] = {"NoiseRate": row[4],
                                  "RelativeDomEff": row[5]}

    # Track IceTop tanks.  Not all stations have installation records
    iceTopTanks = {}
    mbidMap = {}
    for domid in prod:
        mbid = prod[domid]["mbid"]
        stringID = location[mbid]["string"]
        position = location[mbid]["position"]
        location.pop(mbid)
        o = G.GeometryObject(domid,
                             G.ObjectType.ICECUBE_DOM, stringID=stringID)

        # Ignore AMANDA strings
        if stringID < 0:
            continue
        elif isIceAct(stringID, position):
            o.type = G.ObjectType.ICEACT
        elif isAMANDASync(stringID, position):
            o.type = G.ObjectType.AMANDA_SYNC
        elif isAMANDATrig(stringID, position):
            o.type = G.ObjectType.AMANDA_TRIG
        elif isScintillator(stringID, position):
            o.type = G.ObjectType.SCINTILLATOR
        elif position in [61, 62, 63, 64]:
            tankLabel = "A"
            if position in [63, 64]:
                tankLabel = "B"
            o.type = G.ObjectType.ICETOP_DOM
            o.data[G.Keys.TANK_LABEL] = tankLabel
            if stringID not in iceTopTanks:
                iceTopTanks[stringID] = []
            if tankLabel not in iceTopTanks[stringID]:
                iceTopTanks[stringID].append(tankLabel)

        o.data[G.Keys.POSITION_ID] = position

        # AMANDA boards do not have orientation info
        if not (isAMANDASync(stringID, position) or
                isAMANDATrig(stringID, position)):
            o.data[G.Keys.ORIENTATION] = G.Orientation.DOWN
            if geometryOM[stringID][position]["orientation"] > 0:
                o.data[G.Keys.ORIENTATION] = G.Orientation.UP

        o.data[G.Keys.MBID] = mbid
        mbidMap[mbid] = domid
        o.data[G.Keys.NICKNAME] = str(prod[domid]["name"])
        # Scintillators have an area
        if isScintillator(stringID, position) or isIceAct(stringID, position):
            o.data[G.Keys.SENSOR_AREA] = geometryOM[stringID][position]["area"]
        # AMANDA Sync/Trig, IceACT, Scintillators do not have a standard PMT
        if not (isAMANDASync(stringID, position) or
                isAMANDATrig(stringID, position) or
                isIceAct(stringID, position) or
                isScintillator(stringID, position)):
            o.data[G.Keys.PMT_TYPE] = G.PMTType.R7081
            try:
                if pmtInfo[stringID][position]["RelativeDomEff"] > 1.1:
                    o.data[G.Keys.PMT_TYPE] = G.PMTType.R7081_100
            except:
                if stringID in hqeStrings:
                    if stringID in fullHQE and position <= 60:
                        o.data[G.Keys.PMT_TYPE] = G.PMTType.R7081_100
                    elif stringID==79 and position > 50:
                        # Not HQE
                        pass
                    else:
                        # Don't know if this DOM is HQE.  Quit and find out.
                        raise

        # AMANDA boards do not have a location
        if not (isAMANDASync(stringID, position) or
                isAMANDATrig(stringID, position)):
            o.data[G.Keys.GEOMETRY_X] = geometryOM[stringID][position]["x"]
            o.data[G.Keys.GEOMETRY_Y] = geometryOM[stringID][position]["y"]
            o.data[G.Keys.GEOMETRY_Z] = geometryOM[stringID][position]["z"]
            geometryOM[stringID].pop(position)

        # Make note if this board has an the old-type toroid
        o.data[G.Keys.TOROID_TYPE] = G.ToroidType.NEW
        if len(domid) > 4:
            # Year 4 and year 5 DOMs (except 'Alaska') have the old toroid type
            if domid[2] == '4' or (domid[2] == '5' and domid != "UP5P0970"):
                 o.data[G.Keys.TOROID_TYPE] = G.ToroidType.OLD

        geoInserter.insert(o)

    # Store geometry
    with omdb.db_context() as curs:
        curs.execute("SELECT * FROM Geometry")
        row = curs.fetchone()
        o = G.GeometryObject("IceCube Reference", G.ObjectType.POINT)
        o.data[G.Keys.GEOMETRY_X] = row[2]
        o.data[G.Keys.GEOMETRY_Y] = row[3]
        o.data[G.Keys.GEOMETRY_Z] = row[4]
        geoInserter.insert(o)

    # String installations.  Map by stringID
    with omdb.db_context() as curs:
        curs.execute("SELECT * FROM StringInstallation")
        for row in curs.fetchall():
            stringID = row[1]
            o = G.GeometryObject("String%d" % stringID,
                                 G.ObjectType.IN_ICE_STRING, stringID=stringID)
            o.data[G.Keys.DEPLOYMENT_DATE] = str(row[2])
            o.data[G.Keys.DEPLOYMENT_COMMENT] = str(row[3])
            geoInserter.insert(o)

    # IceTop installations.  Map by stringID
    with omdb.db_context() as curs:
        curs.execute("SELECT * FROM StationInstallation")
        for row in curs.fetchall():
            stringID = row[1]
            o = G.GeometryObject("Station%d" % stringID,
                        G.ObjectType.ICE_TOP_STATION, stringID=stringID)
            o.data[G.Keys.DEPLOYMENT_DATE] = str(row[2])
            o.data[G.Keys.DEPLOYMENT_COMMENT] = str(row[3])
            geoInserter.insert(o)

    # IceTop tanks.  Map by string/tank.  We need to get this from
    # the OM database because the station database is incomplete
    for (stringID, tanks) in iceTopTanks.items():
        for tankLabel in tanks:
            name = G.getTankName(stringID, tankLabel)
            o = G.GeometryObject(name, G.ObjectType.ICE_TOP_TANK,
                                 stringID=stringID, tankLabel=tankLabel)
            o.data[G.Keys.TANK_LINER] = G.TankLiner.ZIRCONIUM
            if stringID in tyvekStations:
                 o.data[G.Keys.TANK_LINER] = G.TankLiner.TYVEK
            geoInserter.insert(o)

    # DOM toroid droop constants
    with omdb.db_context() as curs:
        curs.execute("SELECT * FROM DomDroop")
        for row in curs.fetchall():
            mbid = row[0]
            domid = None
            try:
                domid = mbidMap[mbid]
            except KeyError:
                print ("DomDroop: Skipping unassociated mbid: %s" % mbid)
                continue
            o = G.DataObject(domid, C.ObjectType.TOROID_DROOP)
            o.data[C.Keys.TAU0] = row[1]
            o.data[C.Keys.TAU1] = row[2]
            o.data[C.Keys.TAU2] = row[3]
            o.data[C.Keys.TAU3] = row[4]
            o.data[C.Keys.TAU4] = row[5]
            o.data[C.Keys.TAU5] = row[6]
            o.data[C.Keys.TAU_FRACTION] = row[7]
            o.data[C.Keys.ATWD_SIGMA] = row[8]
            o.data[C.Keys.FADC_SIGMA] = row[9]
            calInserter.insert(o)


def main():

    parser = GCDOptionParser()
    parser.add_option("-d", "--sqlHost", dest="sqlHost",
                      help="I3OmDb MySQL host name")
    parser.add_option("-r", "--runValid", dest="runValid",
                      help="runValid entry for calibration quantities")
    parser.add_option("-u", "--user", dest="user", default="www",
                      help="MySQL user name")
    (options, args) = parser.parse_args()
    if options.sqlHost == None:
        print ("I3OmDb MySQL host not specified")
        parser.print_help()
        sys.exit(1)
    if options.runValid == None:
        print ("Calibration runValid not specified")
        parser.print_help()
        sys.exit(1)

    db = getDB(options.dbhost, options.dbuser, options.dbpass)
    i3msHost = options.i3mshost
    with geoDBInserter(db, i3msHost) as geoInserter:
        with calDBInserter(db, options.runValid, i3msHost) as calInserter:
            dumpI3OmDb(geoInserter, calInserter,
                       I3OmDb(host=options.sqlHost, user=options.user))
            calInserter.commit()
        geoInserter.commit()


if __name__ == "__main__":
    main()
