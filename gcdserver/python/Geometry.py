
"""
file: Geometry.py
Contains all of the string keys for geometry documents along with
simple routines and classes to access the data.  This file is
designed to internalize all knowledge of the geometry document.
"""


def getTankName(stringID, tankLabel):
    """
    Construct a tank name from a stringID and tankLabel
    """
    return "Tank%d%s" % (int(stringID), tankLabel.strip())


class ObjectType(object):
    """
    Enumeration of DB object type strings
    """
    ICECUBE_DOM = "IceCube DOM"
    ICETOP_DOM = "IceTop DOM"
    ICEACT = "IceACT"
    AMANDA_SYNC = "AMANDA Synchronization Board"
    AMANDA_TRIG = "AMANDA Trigger Board"
    SCINTILLATOR = "IceCube Scintillator"
    ICE_TOP_TANK = "IceTop Tank"
    ICE_TOP_STATION = "IceTop Station"
    IN_ICE_STRING = "IceCube String"
    POINT = "Point"


class Orientation(object):
    """
    Enumeration of device installation orientation strings
    """
    UP = "Up"
    DOWN = "Down"


class PMTType(object):
    """
    Enumeration of PMT model strings
    """
    R7081 = "R7081"
    R7081_100 = "R7081-100"


class ToroidType(object):
    """
    New vs old toroid in charge transformer
    """
    OLD = "Old"
    NEW = "New"


class TankLiner(object):
    """
    Enumeration of IceTop tank liner strings
    """
    ZIRCONIUM = "Zirconium"
    TYVEK = "Tyvek"


class Keys:
    """
    Set of keys used in GeometryObject.data()
    """
    GEOMETRY_X = "x"
    GEOMETRY_Y = "y"
    GEOMETRY_Z = "z"

    ORIENTATION_NX = "nx"
    ORIENTATION_NY = "ny"
    ORIENTATION_NZ = "nz"
    ORIENTATION_AZIMUTH = "orientationAzimuth"
    ORIENTATION = "orientation"

    STRING_ID = "string"
    POSITION_ID = "position"
    PMT_ID = "pmt"
    TANK_LABEL = "tank"
    DEPLOYMENT_DATE = "deploymentDate"
    DEPLOYMENT_COMMENT = "deploymentComment"

    TANK_LINER = "tankLiner"
    TANK_RADIUS = "tankRadius"
    TANK_HEIGHT = "tankHeight"
    TANK_WATER_HEIGHT = "tankWaterHeight"
    NICKNAME = "nickname"
    MBID = "mbid"
    SENSOR_AREA = "sensorArea"
    PMT_TYPE = "pmtType"
    TOROID_TYPE = "toroidType"


class DictionaryBacked(object):
    """
    Class with a backing dictionary used for serialization
    """
    def __init__(self):
        self._data = {}

    @classmethod
    def wrapdict(cls, d):
        obj = cls()
        obj._data = d
        return obj

    def getdict(self):
        return self._data


class DataObject(DictionaryBacked):
    """
    Base class for all GCD objects.  Contains name and a dictionary for
    underlying data.  Uses DictionaryBacked to manage the outer dictionary,
    containing an object name and a second dictionary for data.
    """
    OBJECT_NAME_KEY = "objectName"
    OBJECT_TYPE_KEY = "objectType"
    OBJECT_DATA_KEY = "data"

    def __init__(self, name=None, type=None, data=None):
        super(DataObject, self).__init__()
        self.name = name
        self.type = type
        if data is not None:
            self.data = data
        else:
            self.data = {}

    @property
    def name(self):
        return str(self.getdict()[self.OBJECT_NAME_KEY])

    @name.setter
    def name(self, val):
        self.getdict()[self.OBJECT_NAME_KEY] = str(val)

    @property
    def type(self):
        # type lives in the outer dictionary, not the inner
        return str(self.getdict()[self.OBJECT_TYPE_KEY])

    @type.setter
    def type(self, val):
        self.getdict()[self.OBJECT_TYPE_KEY] = str(val)

    @property
    def data(self):
        return self.getdict()[self.OBJECT_DATA_KEY]

    @data.setter
    def data(self, val):
        self.getdict()[self.OBJECT_DATA_KEY] = val


class GeometryObject(DataObject):
    """
    Base class for geometry objects.  Present a flat object that represents
    all possible geometry objects, and it is up to the user to understand
    which data is available based on the 'type' property.
    """


    def __init__(self, name=None, type=None, data=None, stringID=None,
                 positionID=None, tankLabel=None):
        super(GeometryObject, self).__init__(name, type, data)
        if stringID is not None:
            self.data[Keys.STRING_ID] = stringID
        if positionID is not None:
            self.data[Keys.POSITION_ID] = positionID
        if tankLabel is not None:
            self.data[Keys.TANK_LABEL] = tankLabel
