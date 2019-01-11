
from icecube import icetray, dataclasses


def buildI3FlasherSubrunMap(flasherData):
    """
    Build an I3FlasherSubrunMap given data from I3Live.
    @flasherData: dictionary from JSON returned byI3Live flasher_data URL
    """
    # Flasher data container.  out[subrun][OMKey] returns the flasher data
    out = dataclasses.I3FlasherSubrunMap()
    for subrunDict in flasherData:
        subrun = int(subrunDict['subrun'])
        out[subrun] = dataclasses.I3FlasherStatusMap()
        for flashingDOM in subrunDict['flasherList']:
            status = dataclasses.I3FlasherStatus()
            status.brightness = int(flashingDOM['brightness'])
            status.window = int(flashingDOM['window'])
            status.rate = int(flashingDOM['rate'])
            status.mask = int(flashingDOM['mask'], 16)
            status.delay = int(flashingDOM['delay'])
            key = icetray.OMKey(int(flashingDOM['string']),
                                int(flashingDOM['position']))
            out[subrun][key] = status
    return out