
"""
file OptionParser.py: Provide an OptionParser with i3live
host name and database server name already added as options.
"""

DEFAULT_DB_SERVER = ("mongodb://mongodb-live-1.icecube.wisc.edu,"
                     "mongodb-live-2.icecube.wisc.edu/?"
                     "replicaSet=rs-live&readPreference=secondaryPreferred")

DEFAULT_I3LIVE_SERVER = "live.icecube.wisc.edu"

DEFAULT_DB_USER = "icecube"
DEFAULT_DB_PASS = "skua"


from optparse import OptionParser
from ConfigParser  import SafeConfigParser
from os.path import join
from os import environ


def getConfigOption(config, option, default):
    try:
        return config.get("gcdserver", option)
    except:
        return default


def GCDOptionParser():
    
    config = SafeConfigParser()
    config.read([join(environ['HOME'], '.gcdserver.conf')])
    
    parser = OptionParser()
    parser.add_option("--dbhost", dest="dbhost",
                      default=getConfigOption(config, "dbhost", DEFAULT_DB_SERVER),
                      help="MongoDB server hostname or URI")
    parser.add_option("--dbuser", dest="dbuser",
                      default=getConfigOption(config, "dbuser", DEFAULT_DB_USER),
                      help="MongoDB user name")
    parser.add_option("--dbpass", dest="dbpass",
                      default=getConfigOption(config, "dbpass", DEFAULT_DB_PASS),
                      help="MongoDB password")
    parser.add_option("--i3livehost", dest="i3livehost",
                      default=getConfigOption(config, "i3livehost", DEFAULT_I3LIVE_SERVER),
                      help="IceCube Live web server host name")
    parser.add_option("--i3mshost", dest="i3mshost",
                      default=getConfigOption(config, "i3mshost", None),
                      help="Host name of I3MS server")
    return parser
