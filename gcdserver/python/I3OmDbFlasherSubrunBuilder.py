
from icecube.gcdserver.I3OmDb import I3OmDb
from icecube import icetray, dataclasses


def getI3OmDbFlasherData(runNumber, hostname='dbs2', user='www'):
    """
    Build an I3FlasherSubrunMap object from I3OmDb for a given run number
    """
    out = dataclasses.I3FlasherSubrunMap()
    db = I3OmDb(host=hostname, user=user)
    with db.db_context() as curs:
        curs.execute("SELECT brightness, window, mask, rate, delay, subrun, "
                     "string_hub, dom_position FROM flasher_configuration "
                     "where run=%d" % runNumber)
        for row in curs.fetchall():
            subrun = int(row[5])
            if subrun not in out:
                out[subrun] = dataclasses.I3FlasherStatusMap()
            status = dataclasses.I3FlasherStatus()
            key = icetray.OMKey(int(row[6]), int(row[7]))
            status.brightness = int(row[0])
            status.window = int(row[1])
            status.rate = int(row[3])
            status.mask = int(row[2], 16)
            status.delay = int(row[4])
            out[subrun][key] = status
    return out