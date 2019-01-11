
from icecube.gcdserver.RunData import RunData

import urllib2
import json
import ssl

RUN_INFO_PAGE = "run_info"
FLASHER_DATA_PAGE = "flasher_data"
# Everyone already knows the following, so no shame in placing them here
I3LIVE_USER = "icecube"
I3LIVE_PASS = "skua"


class I3LiveException(Exception):
    pass


def getLiveData(runNumber, hostname, user, password, pageName):
    """
    Get configuration name and good start/stop time from I3Live URL
    """
    url = "https://%s/%s/" % (hostname, pageName)
    payload = "user=%s&pass=%s&run_number=%d" % (user, password, runNumber)
    req = urllib2.Request(url, payload)
    try:
        try:
            # First, try disabling the certificate check (Python 2.7+)
            ctx = ssl.create_default_context()
            ctx.check_hostname = False
            ctx.verify_mode = ssl.CERT_NONE
            return (urllib2.urlopen(req, context=ctx)).read()
        except AttributeError:
            # Python 2.6: Certificate check is broken. Don't need to avoid it.
            return (urllib2.urlopen(req)).read()
    except urllib2.HTTPError as err:
        if err.code == 404:
            raise I3LiveException("Invalid run number")
        elif err.code == 403:
            raise I3LiveException("Not authorized")
        raise I3LiveException("Error %s loading URL '%s'." % (err.code, url))


def doGetLiveRunData(runNumber, hostname, user, password):
    """
    Get configuration name and good start/stop time from I3Live URL
    """
    return getLiveData(runNumber, hostname, user, password, RUN_INFO_PAGE)


def getLiveRunData(runNumber, hostname,
                   user=I3LIVE_USER, password=I3LIVE_PASS):
    """
    Load I3Live run information into a RunData object.  If a start/stop time
    is not available it will be set to None
    """
    data = json.loads(doGetLiveRunData(runNumber, hostname, user, password))
    startFrac = 0
    if data.get("start_frac", 0) is not None:
        startFrac = data.get("start_frac", 0)
    stopFrac = 0
    if data.get("stop_frac", 0) is not None:
        stopFrac = data.get("stop_frac", 0)
    
    return RunData(configName=data.get("config"),
                   startTime=data.get("start"),
                   startFrac=startFrac,
                   stopTime=data.get("stop"),
                   stopFrac=stopFrac,
                   runMode=data.get("run_mode", None),
                   lightMode=data.get("lightmode", "UnknownLIDMode"),
                   filterMode=data.get("filter_mode", "UnknownPFMode"))


def getLiveFlasherData(runNumber, hostname,
                       user=I3LIVE_USER, password=I3LIVE_PASS):
    """
    Get flashing DOM list for a given run from the I3Live URL
    """
    data = getLiveData(runNumber, hostname, user, password, FLASHER_DATA_PAGE)
    return json.loads(data)
