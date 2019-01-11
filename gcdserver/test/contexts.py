
from contextlib import contextmanager
import os

import icecube.gcdserver.MongoDB as MongoDB
from TestData import getRunConfigXML, TRIGGER_CONF_XML, DOM_CONFIG_XML


@contextmanager
def tempdir_context(path):
    try:
        os.mkdir(path)
        yield
    finally:
        os.rmdir(path)


@contextmanager
def test_db_context(dbName=MongoDB.TEST_DATABASE_NAME):
    # For now, assume all tests are run on localhost
    db = MongoDB.getTestDB('localhost', dbName)
    MongoDB.initDB(db)
    try:
        yield db
    finally:
        MongoDB.destroyDB(db)


@contextmanager
def file_context(fileData, fileName=None):
    if fileName is not None and type(fileName) is not str:
        raise Exception("Invalid file name: %s" % fileName)
    if fileName is None or len(fileName) == 0:
        fileName = os.urandom(16).encode('hex')
    if fileName[0] is not '/':
        fileName = "/tmp/%s" % fileName
    try:
        with open(fileName, "w") as f:
            f.write(fileData)
        yield fileName
    finally:
        try:
            os.remove(fileName)
        except:
            pass


DOM_CONF_NAME = "sps-11t-slc-pedsub-256"
TRIG_CONF_NAME = "sps-2016-icetop-infill-010"


@contextmanager
def config_file_context(configName, trigConfName=TRIG_CONF_NAME,
                        domConfName=DOM_CONF_NAME):
    """
    Create the pdaq configuration file hierarchy
    """
    # Make a temporary directory
    dirName = "/tmp/%s" % os.urandom(16).encode('hex')
    with tempdir_context(dirName):
        # Make the trigger and domconfig subdirs
        trigDir = "%s/trigger" % dirName
        with tempdir_context(trigDir):
            domConfDir = "%s/domconfigs" % dirName
            with tempdir_context(domConfDir):
                # Create the files
                configFile = "%s/%s.xml" % (dirName, configName)
                runXML = getRunConfigXML(trigConfName, domConfName)
                with file_context(runXML, configFile):
                    trigConf = "%s/%s.xml" % (trigDir, trigConfName)
                    with file_context(TRIGGER_CONF_XML, trigConf):
                        domConf = "%s/%s.xml" % (domConfDir, domConfName)
                        with file_context(DOM_CONFIG_XML, domConf):
                            yield configFile # Finally!
