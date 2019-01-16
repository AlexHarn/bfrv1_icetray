
"""
Icetray module that generates the G/C/D frame.
"""

from icecube import icetray, dataclasses
from I3Tray import *

from icecube.gcdserver.I3GeometryBuilder import buildI3Geometry
from icecube.gcdserver.I3CalibrationBuilder import buildI3Calibration
from icecube.gcdserver.I3DetectorStatusBuilder import buildI3DetectorStatus
from icecube.gcdserver.I3FlasherSubrunMapBuilder import buildI3FlasherSubrunMap
from icecube.gcdserver.MongoDB import fillBlobDB, getDB
from icecube.gcdserver.I3Live import getLiveRunData, getLiveFlasherData
from icecube.gcdserver.util import setStartStopTime

class GCDGenerator(icetray.I3Module):
    def __init__(self, context):
        icetray.I3Module.__init__(self, context)

        from icecube.gcdserver.OptionParser import DEFAULT_DB_PASS, DEFAULT_DB_USER

        self.AddParameter('RunId',
                          'The run id')

        self.AddParameter('I3LiveHost',
                          'I3Live host',
                          'live.icecube.wisc.edu')

        self.AddParameter('MongoDBHost',
                          'Host of Mongo DB',
                          'mongodb-live')

        self.AddParameter('MongoDBUser',
                          'DB user',
                          DEFAULT_DB_USER)

        self.AddParameter('MongoDBPassword',
                          'DB password',
                          DEFAULT_DB_PASS)

        self.AddOutBox("OutBox")

    def Configure(self):
        self.run_id = self.GetParameter('RunId')
        self.i3live_host = self.GetParameter('I3LiveHost')
        self.mongo_db_host = self.GetParameter('MongoDBHost')
        self.mongo_db_user = self.GetParameter('MongoDBUser')
        self.mongo_db_password = self.GetParameter('MongoDBPassword')

    def Process(self):
        run_data = getLiveRunData(self.run_id, self.i3live_host)

        if run_data.startTime is None:
            icetray.logging.log_fatal("No run data available for run {0}".format(self.run_id))
            raise Exception("No run data available for run {0}".format(self.run_id))

        blob_db = fillBlobDB(getDB(self.mongo_db_host, self.mongo_db_user, self.mongo_db_password), run = self.run_id, configuration = run_data.configName)

        g = buildI3Geometry(blob_db)
        c = buildI3Calibration(blob_db)
        d = buildI3DetectorStatus(blob_db, run_data)

        fdata = buildI3FlasherSubrunMap(getLiveFlasherData(self.run_id, self.i3live_host))

        setStartStopTime(g, d)
        setStartStopTime(c, d)

        fr = icetray.I3Frame(icetray.I3Frame.Geometry)
        fr['I3Geometry'] = g
        self.PushFrame(fr)

        fr = icetray.I3Frame(icetray.I3Frame.Calibration)
        fr['I3Calibration'] = c
        self.PushFrame(fr)

        fr = icetray.I3Frame(icetray.I3Frame.DetectorStatus)
        fr['I3DetectorStatus'] = d
        fr['I3FlasherSubrunMap'] = fdata
        self.PushFrame(fr)

        self.RequestSuspension()

