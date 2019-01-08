
from contextlib import contextmanager
import os
import sys
import copy
import bson
from threading import RLock

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

    if "-D" in sys.argv:
        # Use a real MongoDB instance at localhost
        db = MongoDB.getTestDB('localhost', dbName)
        MongoDB.initDB(db)
        try:
            yield db
        finally:
            MongoDB.destroyDB(db)
    else:
        # Use an in-memory database
        db = {}
        db[MongoDB.GEO_COLLECTION_NAME] = SimCollection()
        db[MongoDB.CAL_COLLECTION_NAME] = SimCollection()
        db[MongoDB.STATUS_COLLECTION_NAME] = SimCollection()
        db[MongoDB.TRANSACTION_COLLECTION_NAME] = SimCollection(
                                            unique=[MongoDB.TRANSACTION_KEY])
        yield db


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


class SimCursor(object):
    
    def __init__(self, data):
        self.__data = data

    def sort(self, match):
        # Python has a stable sort. Sort by last attribute --> first attribute
        if len(match) == 0:
            return self
        # Support both list and son.SON objects
        (k, v) = (None, None)
        try:
            for (k, v) in match.iteritems():
                pass
            match.pop(k)
        except:
            (k, v) = match[-1]
            match.pop()
        return (SimCursor(sorted(self.__data,
                    key=lambda x: x[k], reverse=(v < 0)))).sort(match)

    def limit(self, cnt):
        if cnt <= 0:
            return self
        return SimCursor(self.__data[:cnt])

    def count(self):
        return len(self.__data)

    def __iter__(self):
        return iter(self.__data)

    def __getitem__(self, idx):
        return self.__data[idx]


class SimInsertResult(object):

    def __init__(self, ids):
        self.inserted_ids = ids


class SimUpdateResult(object):

    def __init__(self, n):
        self.raw_result = {}
        self.raw_result['n'] = n


def simKeyRecursion(doc, key, newValue=None):
    keys = key.split('.')
    ret = doc
    # Get the relevant inner dict
    for k in keys[:-1]:
        ret = ret[k]
    # Set the new value if requested
    if newValue is not None:
        ret[keys[-1]] = copy.deepcopy(newValue)
    return ret[keys[-1]]


def simDoMatch(doc, k, v):
    x = None
    try:
        x = simKeyRecursion(doc, k)
    except:
        return False

    if type(v) is dict and any(key.startswith("$") for key in v):
        # Handle $lte, $gte, $in, etc. Logically AND by convention.
        for (nk, nv) in v.iteritems():
            if nk == "$lte":
                if not type(nv)(x) <= nv:
                    return False
            elif nk == "$lt":
                if not type(nv)(x) < nv:
                    return False
            elif nk == "$gt":
                if not type(nv)(x) > nv:
                    return False
            elif nk == "$gte":
                if not type(nv)(x) >= nv:
                    return False
            elif nk == "$in":
                if not x in nv:
                    return False
            elif nk == "$not":
                if doMatch(doc, k, nv):
                    return False
            else:
                raise Exception("Unsupported simDB operation: %s" % nk)
        return True
    return x == v


def simMatch(x, match):
    return all(simDoMatch(x, k, v) for (k, v) in match.iteritems())


def simFind(data, match):
    if len(match) == 0:
        return data
    return [x for x in data if simMatch(x, match)]


class SimCollection(object):
    
    def __init__(self, unique=[]):
        self.__data = []
        self.__unique = unique
        self.__lock = RLock()

    def _check_unique(self, doc):
        for key in self.__unique:
            value = None
            try:
                value = simKeyRecursion(doc, key)
            except:
                continue
            if len(simFind(self.__data, {key: value})) > 0:
                raise Exception("DBConsistency")

    def insert_one(self, doc):
        with self.__lock:
            self._check_unique(doc)
            newdoc = copy.deepcopy(doc)
            if "_id" not in newdoc:
                newdoc['_id'] = bson.ObjectId(os.urandom(12).encode('hex'))
            self.__data.append(newdoc)
            self.find()
            return newdoc['_id']

    def insert_many(self, docs):
        with self.__lock:
            return SimInsertResult([self.insert_one(x) for x in docs])

    def delete_many(self, match):
        docs = simFind(self.__data, match)
        if len(docs) > 0:
            # This is ugly
            self.__data = [x for x in self.__data if x not in docs]

    def update_many(self, match, update):
        with self.__lock:
            # Only handle $set
            docs = simFind(self.__data, match)
            mod = update["$set"]
            for (k,v) in mod.iteritems():
                for doc in docs:
                    simKeyRecursion(doc, k, v)
            return SimUpdateResult(len(docs))
    
    def find(self, match={}, limit=0):
        with self.__lock:
            out = SimCursor(simFind(self.__data, match)).limit(limit)
            return copy.deepcopy(out)

    def count(self):
        with self.__lock:
            return len(self.__data)
    
    def ensure_index(self, arg):
        pass

    def _doAggregate(self, result, cmd, data):
        # Support $sort, $match, $group
        if cmd == "$match":
            return SimCursor(simFind(result, data))
        elif cmd == "$sort":
            return result.sort(data)
        elif cmd == "$group":
            assert "_id" in data
            ink = data["_id"]
            def getKey(doc):
                # This is super inefficient but clean and good enough
                ret = []
                for (k, v) in ink.iteritems():
                    ret.append(simKeyRecursion(doc, v.strip('$')))
                return tuple(ret)

            # Make the assumption here of "data": {"$last": "$$ROOT"}
            # This is specific to the existing gcdserver pipeline and
            # will break if it is changed.
            data.pop("_id")
            assert len(data) == 1
            outk = data.keys()[0]
            # Aggregate
            out = {}
            for doc in result:
                out[getKey(doc)] = doc
            return SimCursor([{outk: x} for x in out.itervalues()])
    
    def aggregate(self, pipeline, allowDiskUse=False, cursor={}):
        with self.__lock:
            result = SimCursor(self.__data)
            for entry in pipeline:
                assert len(entry) == 1
                (cmd, data) = next(copy.deepcopy(entry).iteritems())
                result = self._doAggregate(result, cmd, data)
            return result
