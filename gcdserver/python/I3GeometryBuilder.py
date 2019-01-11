
from I3Tray import I3Units
from icecube import dataclasses
from icecube.icetray import logging

import icecube.gcdserver.Geometry as G
import icecube.gcdserver.Calibration as C
import icecube.gcdserver.GeometryDefaults as GeometryDefaults
import icecube.gcdserver.util as util


def getTankType(o):
    """
    Return the IceTray OM type given the liner of GeometryObject tank 'o'
    """
    try:
        if o.data[G.Keys.TANK_LINER] == G.TankLiner.ZIRCONIUM:
            return dataclasses.I3TankGeo.Zirconium_Lined
        if o.data[G.Keys.TANK_LINER] == G.TankLiner.TYVEK:
            return dataclasses.I3TankGeo.Tyvek_Lined
    except KeyError:
        pass
    return dataclasses.I3TankGeo.NotSet


def getOMType(o):
    """
    Return the IceTray OM type given the type of GeometryObject 'o'
    """
    if o.type == G.ObjectType.ICECUBE_DOM:
        return dataclasses.I3OMGeo.IceCube
    if o.type == G.ObjectType.ICETOP_DOM:
        return dataclasses.I3OMGeo.IceTop
    if o.type == G.ObjectType.SCINTILLATOR:
        return dataclasses.I3OMGeo.Scintillator
    if o.type == G.ObjectType.ICEACT:
        return dataclasses.I3OMGeo.IceAct
    if (o.type == G.ObjectType.AMANDA_SYNC or
        o.type == G.ObjectType.AMANDA_TRIG):
        return dataclasses.I3OMGeo.AMANDA
    return dataclasses.I3OMGeo.UnknownType


def getOMGeo(o):
    """
    Build an I3OMGeo object from geometry object 'o'.  Throws
    KeyException if geometry keys are not available.
    @o: Geometry object
    @return: I3OMGeo
    """
    i3omgeo = dataclasses.I3OMGeo()
    i3omgeo.position = util.getPosition(o)
    i3omgeo.orientation = util.getOrientation(o)
    i3omgeo.omtype = getOMType(o)
    # Get the photosensitive area
    try:
        i3omgeo.area = float(o.data[G.Keys.SENSOR_AREA] * I3Units.m2)
    except KeyError:
        # No sensor area.  Get default values, if possible
        try:
            i3omgeo.area = 0.
            if (o.data[G.Keys.PMT_TYPE] in
                        (G.PMTType.R7081, G.PMTType.R7081_100)):
                i3omgeo.area = float(GeometryDefaults.R7081PMTArea)
        except KeyError:
            logging.log_warn("I3Geometry: No PMT area for device %s" % o.name)
    return i3omgeo


class TankData(object):
    """
    Helper class to group together and IceTop tank with the
    corresponding IceTop DOMs and snow depth measurement
    """
    def __init__(self):
        self.iceTopDOMs = []
        self.iceTopTanks = []
        self.snowHeight = None


def buildTankGeo(stringID, tankLabel, td):
    """
    Build an I3TankGeo object from TankData object 'td'
    """
    # Ensure we have both a tank and DOMs
    if len(td.iceTopTanks) < 1:
        logging.log_fatal("I3Geometry: Found DOMs with no tank entry "
                          "in string %s, tank: %s" % (stringID, tankLabel))
    if len(td.iceTopTanks) > 1:
        logging.log_fatal("I3Geometry: Found multiple entries for "
                          "string %s, tank: %s" % (stringID, tankLabel))
    if len(td.iceTopDOMs) < 1:
        logging.log_fatal("I3Geometry: Found tank with no DOMs for "
                          "string %s, tank: %s" % (stringID, tankLabel))

    tankgeo = dataclasses.I3TankGeo()
    # Here we assume that all tanks in a given station are identical
    # This is a constraint from our data structures.
    tank = td.iceTopTanks[0]
    tankgeo.tanktype = getTankType(tank)
    # To preserve I3Db behavior, don't write default for tank orientation
    if G.Keys.ORIENTATION in tank.data:
        tankgeo.orientation = tank.data.get(G.Keys.ORIENTATION)
    tankgeo.tankradius = tank.data.get(G.Keys.TANK_RADIUS,
                                GeometryDefaults.IceTopTankRadius)
    tankgeo.tankheight = tank.data.get(G.Keys.TANK_HEIGHT,
                                GeometryDefaults.IceTopTankHeight)
    tankgeo.fillheight = tank.data.get(G.Keys.TANK_WATER_HEIGHT,
                                GeometryDefaults.IceTopTankFillHeight)

    # Use supplied position if available
    try:
        tankgeo.position = util.getPosition(tank)
    except KeyError:
        # Use average DOM position as tank position
        logging.log_debug("I3Geometry: Using average DOM position for "
                          "string: %s, tank %s" % (stringID, tankLabel))
        tankgeo.position = dataclasses.I3Position(0., 0., 0.)
        for o in td.iceTopDOMs:
            try:
                tankgeo.position += util.getPosition(o)
            except KeyError:
                logging.log_fatal("I3Geometry: Found IceTop DOM with no "
                                  "geometry: %s" % o.name) 
        tankgeo.position /= len(td.iceTopDOMs)

    # Build OMKey list
    for o in td.iceTopDOMs:
        try:
            omkey = util.getOMKey(o)
            tankgeo.omkey_list.append(omkey)
        except KeyError:
            logging.log_fatal("I3Geometry: Found IceTop DOM with no "
                              "OMKey data: %s" % o.name) 

    tankgeo.snowheight = 0
    if td.snowHeight is None:
        logging.log_warn("I3Geometry: No snow height for "
                         "string: %s, tank %s" % (stringID, tankLabel))
    else:
        tankgeo.snowheight = td.snowHeight.data[C.Keys.SNOW_HEIGHT] * I3Units.m

    return tankgeo


def buildI3Geometry(db):
    """
    Build an I3Geometry instance from GCD data.  We need
    data from the calibration DB to get the IceTop snow depths.
    @db: Access to set of valid GeometryObject and CalibrationObject instances
    @return: dataclasses.I3Geometry instance.
    """
    geo = dataclasses.I3Geometry()

    # Discussion with Alex O.: the start/end times are only used by I3Muxer,
    # and it should be easy to switch to run-based validity.  Since this
    # feature is unused, set start/end times such that data is always valid.
    geo.start_time.set_daq_time(0, 0)
    geo.end_time.set_daq_time(9999, 0)

    # Build I3OMGeo table from list of all installed devices
    for (key, o) in db.deployedDevices():
        # Use any device that has geometry data
        try:
            omkey = util.getOMKey(o)
            devgeo = getOMGeo(o)
            geo.omgeo[omkey] = devgeo
        except KeyError:
            logging.log_warn("I3Geometry: Skipping device with OMKey but no "
                             "geometry data: %s" % o.name)

    # Now build the IceTop station geometry.
    # Group all relevant data by (stringID, tankLabel)
    iceTopData = {}
    for o in db.geometryDocuments(G.ObjectType.ICETOP_DOM):
        try:
            key = (o.data[G.Keys.STRING_ID], str(o.data[G.Keys.TANK_LABEL]))
            if not key in iceTopData:
                iceTopData[key] = TankData()
            iceTopData[key].iceTopDOMs.append(o)
        except KeyError:
            logging.log_fatal("I3Geometry: IceTop DOM with no "
                              "OMKey data: %s" % o.name)

    for o in db.geometryDocuments(G.ObjectType.ICE_TOP_TANK):
        try:
            key = (o.data[G.Keys.STRING_ID], str(o.data[G.Keys.TANK_LABEL]))
            if not key in iceTopData:
                iceTopData[key] = TankData()
            iceTopData[key].iceTopTanks.append(o)
        except KeyError:
            logging.log_fatal("I3Geometry: Found station with no "
                              "string ID: %s" % o.name)

    # Now get the snow depth from the calibration object list
    # Objects are named by the corresponding tank
    for cal in db.calibrationDocuments(C.ObjectType.SNOW_HEIGHT):
        try:
            iceTopData[db.tankMap(cal.name)].snowHeight = cal
        except KeyError:
            # We don't have tank/OM data for this tank
            logging.log_warn("I3Geometry: Found snow depth for non-existent "
                              "tank %s" % cal.name)

    # Build TankGeo objects one tank at a time.
    for ((stringID, tankLabel), data) in iceTopData.iteritems():
        try:
            # Build the I3TankGeo object
            tankGeo = buildTankGeo(stringID, tankLabel, data)
            if not stringID in geo.stationgeo:
                geo.stationgeo[stringID] = dataclasses.I3StationGeo()
            geo.stationgeo[stringID].append(tankGeo)
        except KeyError:
            logging.log_warn("I3Geometry: Skipping station "
                             "with insufficient geometry "
                             "at string %s" % str(stringID))

    return geo
