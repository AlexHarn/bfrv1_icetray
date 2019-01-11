
import icecube.gcdserver.Geometry as G


class BlobDB(object):
    """
    Interface to GeometryObject, CalibrationObject, and DetectorStatus
    object instances given an array of dictionaries for each collection
    of data.  Essentially a wrapper on calls that could, potentially, be
    served directly from the database.
    """
    def __init__(self, geoData, calData, statusData):
        self.__geo = [G.DataObject.wrapdict(x) for x in geoData]
        self.__cal = [G.DataObject.wrapdict(x) for x in calData]
        self.__status = [G.DataObject.wrapdict(x) for x in statusData]
        self.__tankMap = None
        self.__mbidMap = None
        self.__omKeyMap = None
        self.__deployedNameMap = None
        self.__calDeviceMap = None

    def getDeploymentTuple(self, o):
        """
        Return (stringId, positionId, pmtId) for a given geometry object
        """
        try:
            return (o.data[G.Keys.STRING_ID],
                    o.data[G.Keys.POSITION_ID],
                    o.data[G.Keys.PMT_ID])
        except KeyError:
            return (o.data[G.Keys.STRING_ID],
                    o.data[G.Keys.POSITION_ID],
                    0)

    def deployedDevices(self):
        """
        Generator of ((stringId, positionId, pmtId), object) for each object
        in the geometry database with deployment data (at least string ID and
        position ID)
        """
        for o in self.__geo:
            try:
                key = self.getDeploymentTuple(o)
                yield (key, o)
            except KeyError:
                pass

    def geometryDocuments(self, objectType=None):
        """
        Get a generator into geometry documents, optionally selecting
        only a specific type
        """
        if objectType is None:
            return (o for o in self.__geo)
        else:
            return (o for o in self.__geo if o.type == objectType)

    def calibrationDocuments(self, objectType=None, deviceName=None):
        """
        Get a generator into calibration documents, optionally selecting
        only a specific type and/or only a specific device
        """
        source = self.__cal
        if deviceName is not None:
            if self.__calDeviceMap is None:
                self.__buildCalDeviceMap()
            try:
                source = self.__calDeviceMap[deviceName]
            except KeyError:
                source = []
        if objectType is None:
            return (o for o in source)
        else:
            return (o for o in source if o.type == objectType)
    
    def statusDocuments(self, objectType=None):
        """
        Get a generator into detector status documents, optionally selecting
        only a specific type
        """
        if objectType is None:
            return (o for o in self.__status)
        else:
            return (o for o in self.__status if o.type == objectType)

    def __buildCalDeviceMap(self):
        """
        Build the map of device name --> all associated calibration data
        """
        self.__calDeviceMap = {}
        for o in self.__cal:
            if not o.name in self.__calDeviceMap:
                self.__calDeviceMap[o.name] = []
            self.__calDeviceMap[o.name].append(o)

    def __buildTankMap(self):
        """
        Build a map of tank name --> (stringID, tankLabel) pair
        """
        self.__tankMap = {}
        for o in self.geometryDocuments(G.ObjectType.ICE_TOP_TANK):
            try:
                self.__tankMap[o.name] = (o.data[G.Keys.STRING_ID],
                                          o.data[G.Keys.TANK_LABEL])
            except KeyError:
                pass

    def __buildMBIDMap(self):
        """
        Build a map of mainboard ID --> device name
        """
        self.__mbidMap = {}
        for o in self.__geo:
            try:
                self.__mbidMap[o.data[G.Keys.MBID]] = o.name
            except KeyError:
                pass

    def __buildOMKeyMap(self):
        """
        Build a map of (string, position, pmt) --> device name.
        """
        self.__omKeyMap = {}
        for (key, o) in self.deployedDevices():
            self.__omKeyMap[key] = o.name

    def __buildDeployedNameMap(self):
        """
        Build a map of deployed device name --> object
        """
        self.__deployedNameMap = {}
        for (key, o) in self.deployedDevices():
            self.__deployedNameMap[o.name] = o

    def tankMap(self, tankName):
        """
        Map a tank name to a (stringID, tankLabel) pair
        """
        if self.__tankMap is None:
            self.__buildTankMap()
        return self.__tankMap[tankName]

    def mbidMap(self, mbid):
        """
        Map mbid --> device name
        """
        if self.__mbidMap is None:
            self.__buildMBIDMap()
        return self.__mbidMap[mbid]

    def omKeyMap(self, stringID, positionID, pmtID=None):
        """
        Map omKey --> device name
        """
        if self.__omKeyMap is None:
            self.__buildOMKeyMap()
        if pmtID is None:
            pmtID = 0
        key = (stringID, positionID, pmtID)
        return self.__omKeyMap[key]

    def deployedNameMap(self, name):
        """
        Map device name --> object
        """
        if self.__deployedNameMap is None:
            self.__buildDeployedNameMap()
        return self.__deployedNameMap[name]
