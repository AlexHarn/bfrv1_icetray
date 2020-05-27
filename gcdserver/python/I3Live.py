from __future__ import absolute_import
from __future__ import division
from __future__ import print_function
from __future__ import unicode_literals

import urllib
import json
import ssl

try: 
    from urllib.parse import urlencode 
    from urllib.request import Request
    from urllib.request import urlopen
    from urllib.error import HTTPError 
except ImportError as err:
    from urllib import urlencode
    from urllib2 import Request
    from urllib2 import urlopen
    from urllib2 import HTTPError 


from icecube.gcdserver.RunData import RunData

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
    params = {'user': user, 
          'pass': password, 
          'run_number': runNumber}
    data = urlencode(params).encode("utf-8")
    req = Request(url)
    try:
        # First, try disabling the certificate check (Python 2.7+)
        ctx = ssl._create_unverified_context()

        return urlopen(req,data=data,context=ctx).read()

    except HTTPError as err:
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
