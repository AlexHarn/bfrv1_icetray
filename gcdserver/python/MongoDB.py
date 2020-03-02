
"""
MongoDB.py:  Contains all MongoDB access/insert routines,
internalizes all knowledge of the database structure, and adds/removes
document wrappers as needed to provide functionality
"""

import pymongo
import sys
import os
import time
import copy
import socket
import json
from datetime import datetime
from bson import son

from icecube.gcdserver.BlobDB import BlobDB
import icecube.gcdserver.Geometry as G
import icecube.gcdserver.DetectorStatus as D

DATABASE_NAME = "omdb"
TEST_DATABASE_NAME = "omdbTest"
GEO_COLLECTION_NAME = "geometry"
CAL_COLLECTION_NAME = "calibration"
STATUS_COLLECTION_NAME = "status"
TRANSACTION_COLLECTION_NAME = "transaction"
COLLECTION_NAMES = [GEO_COLLECTION_NAME,
                    CAL_COLLECTION_NAME,
                    STATUS_COLLECTION_NAME,
                    TRANSACTION_COLLECTION_NAME]

REVISION_KEY = "revision"
RUN_VALID_KEY = "runValid"
OBJECT_KEY = "object"

TRANSACTION_KEY = "transaction"
TRANSACTION_DATE_KEY = "insertDate"
TRANSACTION_STATUS_KEY = "status"
TRANSACTION_CWD_KEY = "cwd"
TRANSACTION_CMDLINE_KEY = "cmdline"
TRANSACTION_HOSTNAME_KEY = "hostName"

TRANSACTION_STATUS_NEW = "new"
TRANSACTION_STATUS_COMMITTED = "committed"
TRANSACTION_STATUS_ROLLBACK = "rollback"
TRANSACTION_STATUS_ERROR = "error"

DATA_AGGREGATION_KEY = "data"

OBJECT_NAME_KEY = "%s.%s" % (OBJECT_KEY, G.DataObject.OBJECT_NAME_KEY)
OBJECT_TYPE_KEY = "%s.%s" % (OBJECT_KEY, G.DataObject.OBJECT_TYPE_KEY)

def getGeoPipeline():
    """
    MongoDB aggregation pipeline to get latest revision of geometry objects
    """
    return ([
                {"$sort": {REVISION_KEY: 1}},
                {"$group": 
                    {
                        "_id": {"objectName": "$%s" % OBJECT_NAME_KEY},
                        DATA_AGGREGATION_KEY: {"$last": "$$ROOT"}
                    }
                }])


def getCalPipeline(runNumber):
    """
    MongoDB aggregation pipeline to get latest revision of calibration
    objects, with valid run number not larger than the requested run number.
    Use son.SON to preserve sort order, since we need runValid sorted
    first, then revision
    """
    return ([
                {"$sort": son.SON([(RUN_VALID_KEY, 1),
                                   (REVISION_KEY, 1)])},
                {"$match":
                    {
                        RUN_VALID_KEY: {"$lte": runNumber},
                    }
                },
                {"$group": 
                    {
                        "_id": {"objectName": "$%s" % OBJECT_NAME_KEY,
                                "objectType": "$%s" % OBJECT_TYPE_KEY},
                        DATA_AGGREGATION_KEY: {"$last": "$$ROOT"}
                    }
                }
            ])


def getDB(dbhost, dbuser, dbpass):
    db = pymongo.MongoClient(dbhost)[DATABASE_NAME]
    # First, try without authentication
    try:
        db[GEO_COLLECTION_NAME].index_information()
        # We have all privileges on db
    except:
        # We need to authenticate
        db.authenticate(dbuser, dbpass)
    return db


def getTestDB(dbhost, dbName=TEST_DATABASE_NAME):
    # Assume we don't need to deal with auth for the test DB
    return pymongo.MongoClient(dbhost)[dbName]


def initGeoDB(db):
    geo = db[GEO_COLLECTION_NAME]
    geo.ensure_index([(REVISION_KEY, pymongo.ASCENDING)])
    geo.ensure_index([(TRANSACTION_KEY, pymongo.ASCENDING)])
    geo.ensure_index([(OBJECT_NAME_KEY, pymongo.ASCENDING)])
    geo.ensure_index([(OBJECT_TYPE_KEY, pymongo.ASCENDING)])


def initCalDB(db):
    cal = db[CAL_COLLECTION_NAME]
    cal.ensure_index([(RUN_VALID_KEY, pymongo.ASCENDING),
                      (REVISION_KEY, pymongo.ASCENDING)])
    cal.ensure_index([(TRANSACTION_KEY, pymongo.ASCENDING)])
    cal.ensure_index([(OBJECT_NAME_KEY, pymongo.ASCENDING)])
    cal.ensure_index([(OBJECT_TYPE_KEY, pymongo.ASCENDING)])


def initStatusDB(db):
    cal = db[STATUS_COLLECTION_NAME]
    cal.ensure_index([(TRANSACTION_KEY, pymongo.ASCENDING)])
    cal.ensure_index([(OBJECT_NAME_KEY, pymongo.ASCENDING)], unique=True)
    cal.ensure_index([(OBJECT_TYPE_KEY, pymongo.ASCENDING)])


def initTransactionDB(db):
    trans = db[TRANSACTION_COLLECTION_NAME]
    trans.ensure_index([(TRANSACTION_KEY, pymongo.ASCENDING)], unique=True)


def initDB(db):
    initGeoDB(db)
    initCalDB(db)
    initStatusDB(db)
    initTransactionDB(db)


def destroyDB(db):
    for collectionName in COLLECTION_NAMES:
        coll = db[collectionName]
        coll.drop()


def getAggregationResult(cursor):
    return [x[DATA_AGGREGATION_KEY][OBJECT_KEY] for x in cursor]


class DBMissingConfiguration(Exception):
    pass


def fillBlobDB(db, run=sys.maxsize, configuration=None):
    """
    Get a BlobDB instance backed by documents loaded from MongoDB.
    @run: Run number for calibration data
    default: Use latest data
    If run or configuration are None, skip loading calibration
    or detector status, respectively.  Also may need to support a
    get-by-insertion-date routine to get data from older revisions.
    Consider this as YAGNI for now and implement when needed.
    """
    geo = db[GEO_COLLECTION_NAME]
    cal = db[CAL_COLLECTION_NAME]
    status = db[STATUS_COLLECTION_NAME]

    # Need to specify cursor={} to ensure a cursor is returned in pymongo < 3
    geoObs = getAggregationResult(geo.aggregate(getGeoPipeline(), cursor={}))
    calObs = []
    if run is not None:
        calObs = getAggregationResult(cal.aggregate(
                     getCalPipeline(int(run)), allowDiskUse=True, cursor={}))
    statusObs = []
    if configuration is not None:
        # Need to parse the run config file to get all config file names
        config = list(status.find({OBJECT_NAME_KEY: str(configuration)}))
        if len(config) > 0:
            runConfig = G.DataObject.wrapdict(config[0][OBJECT_KEY])
            configData = D.RunConfig.wrapdict(runConfig.data)
            result = status.find({OBJECT_NAME_KEY:
                                  configData.getTriggerConfigListName()})
            statusObs.extend([x[OBJECT_KEY] for x in result])
            result = status.find({OBJECT_NAME_KEY:
                                 {"$in": configData.getDOMConfigListNames()}})
            statusObs.extend([x[OBJECT_KEY] for x in result])
        else:
            raise DBMissingConfiguration()

    return BlobDB(geoObs, calObs, statusObs)


def countObj(obj):
    # Count a pymongo collection or cursor in a way
    # that is compatible with pymongo >= 3.7
    try:
        # First try the 3.7 way
        return obj.estimated_document_count()
    except:
        # OK, do it the < 3.7 way
        return obj.count()


def getDocumentCounts(db):
    ret = {}
    for coll in COLLECTION_NAMES:
        ret[coll] = countObj(db[coll])
    return ret


class DBTransactionError(Exception):
    pass


class DBTransaction(object):
    """
    Contains the details associated with a specific database insertion
    and logic to ensure each insertion is associated with a unique integer.
    """
    def __init__(self, db, transaction=None):
        self.__collection = db[TRANSACTION_COLLECTION_NAME]
        if transaction is None:
            transaction = self.__newTransaction()
        self.__match = {TRANSACTION_KEY: transaction}
        # Ensure transaction is in the DB
        if len(list(self.__collection.find(self.__match))) == 0:
            raise DBTransactionError(
                        ("Unable to find transaction %s") % transaction)

    def __nextTransaction(self):
        matches = self.__collection.find().sort(
                                  [(TRANSACTION_KEY, -1)]).limit(1)
        if countObj(matches) > 0:
            return matches[0][TRANSACTION_KEY] + 1
        return 0

    def __insertNewTransaction(self):
        timeStr = datetime.strftime(datetime.utcnow(), "%Y-%m-%d %H:%M:%S")
        doc = {TRANSACTION_KEY: self.__nextTransaction(),
               TRANSACTION_DATE_KEY: timeStr,
               TRANSACTION_STATUS_KEY: TRANSACTION_STATUS_NEW,
               TRANSACTION_CWD_KEY: os.getcwd(),
               TRANSACTION_HOSTNAME_KEY: socket.gethostname(),
               TRANSACTION_CMDLINE_KEY: " ".join(sys.argv)}
        self.__collection.insert_one(doc)
        return doc[TRANSACTION_KEY]

    def __newTransaction(self):
        """
        Prevent race condition on transaction number by using
        a unique index.  But this means an unsuccessful attempt to
        obtain a transaction number may raise an exception.  Try
        multiple times before giving up.
        """
        error = None
        for _ in range(100):
            try:
                return self.__insertNewTransaction()
            except Exception as e:
                time.sleep(0.1)
                error = e
        raise DBTransactionError("Error creating transaction: %s" % str(error))

    def getTransactionNumber(self):
        return self.__match[TRANSACTION_KEY]

    def __setState(self, state):
        change = {"$set": {TRANSACTION_STATUS_KEY: state}}
        result = self.__collection.update_many(self.__match, change)
        if result.raw_result['n'] != 1:
            raise DBTransactionError(
                "Unable to find transaction %d" % self.getTransactionNumber())

    def setCommitted(self):
        self.__setState(TRANSACTION_STATUS_COMMITTED)

    def setRollback(self):
        self.__setState(TRANSACTION_STATUS_ROLLBACK)

    def setError(self):
        self.__setState(TRANSACTION_STATUS_ERROR)

    def getTransactionDocument(self):
        matches = self.__collection.find(self.__match)
        if countObj(matches) == 0:
            raise DBTransactionError(
                "Unable to find transaction %d" % self.getTransactionNumber())
        return matches[0]

    def getState(self):
        return self.getTransactionDocument()[TRANSACTION_STATUS_KEY]


class DBInsertError(Exception):
    pass


class EmptyTransaction(Exception):
    pass


class DBInserter(object):
    """
    Store messages, then insert them into MongoDB in one transaction.
    Control transaction logic.  Not thread safe.  We don't care about
    the document content as long as it can be inserted into MongoDB.
    """
    def __init__(self, db, collectionName):
        self.__collectionName = collectionName
        self._docs = []
        self.__db = db
        self._collection = db[collectionName]

    def insert(self, m):
        # Make a copy in case the original document is modified after the fact
        self._docs.append(copy.deepcopy(m))

    def docCount(self):
        return len(self._docs)

    def collectionName(self):
        return self.__collectionName

    def getTransactionData(self, transaction):
        # Need to place the single transaction document into a list
        # list(doc) only creates a list from the keys
        transactionDocList = []
        transactionDocList.append(transaction.getTransactionDocument())
        doc = {TRANSACTION_COLLECTION_NAME: transactionDocList}
        match = {TRANSACTION_KEY: transaction.getTransactionNumber()}
        doc[self.__collectionName] = list(self._collection.find(match))
        return doc

    def hasTransaction(self, transaction):
        match = {TRANSACTION_KEY: transaction.getTransactionNumber()}
        return len(list(self._collection.find(match, limit=1))) > 0

    def rollback(self, transaction):
        match = {TRANSACTION_KEY: transaction.getTransactionNumber()}
        self._collection.delete_many(match)
        transaction.setRollback()

    def getDocumentCount(self, transaction):
        """
        Just get the number of documents committed in the given transaction
        """
        match = {TRANSACTION_KEY: transaction.getTransactionNumber()}
        return countObj(self._collection.find(match))

    def doCommit(self):
        """
        Commit the list of messages to MongoDB
        """
        # If we don't have any new data to commit, don't create a transaction
        if len(self._docs) == 0:
            raise EmptyTransaction("No documents to insert")
        transaction = None
        try:
            # Obtain a transaction number lock
            transaction = DBTransaction(self.__db)
            transactionNumber = transaction.getTransactionNumber()
            # Insert the documents.
            # Set the message transaction number
            for m in self._docs:
                m[TRANSACTION_KEY] = transactionNumber
            # Use write_concern=1, because we need to immediately update
            result = self._collection.insert_many(self._docs)
            # All should have been inserted.
            assert len(self._docs) == len(result.inserted_ids)
        except Exception as e:
            # Oops.  Remove any documents we inserted
            if transaction != None:
                self.rollback(transaction)
                transaction.setError()
            raise DBInsertError("Batch mongoDB insert failed: %s" % str(e))
        # Now we update the transaction, making the transaction valid
        transaction.setCommitted()
        return transaction

    def commit(self):
        """
        Wrap __commit() such that internal message list is always cleared
        """
        try:
            return self.doCommit()
        except:
            raise
        finally:
            self._docs = []


def getName(doc):
    return doc[OBJECT_KEY][G.DataObject.OBJECT_NAME_KEY]


def getNameType(doc):
    return (doc[OBJECT_KEY][G.DataObject.OBJECT_NAME_KEY],
            doc[OBJECT_KEY][G.DataObject.OBJECT_TYPE_KEY])


def getUniqueDocumentMap(docs, getKey):
    """
    Return a map of iterable docs, using getKey(doc) as the document key.
    Ensure docs do not contain items that create identical keys.
    """
    docsByKey = {}
    for d in docs:
        key = getKey(d)
        if key in docsByKey:
            raise DBInsertError("Insert: found duplicate "
                                "objects: %s" % str(key))
        docsByKey[key] = d
    return docsByKey


class GeoDBInserter(DBInserter):
    def __init__(self, db):
        super(GeoDBInserter, self).__init__(db, GEO_COLLECTION_NAME)

    def insert(self, geoObject):
        data = {}
        data[REVISION_KEY] = 0
        data[OBJECT_KEY] = geoObject.getdict()
        DBInserter.insert(self, data)

    def doCommit(self):
        """
        Ensure there are no objects with the same object name
        and revision number before committing.
        """
        # Get map of documents with name as key
        docMap = getUniqueDocumentMap(self._docs, getName)
        for r in self._collection.find(
                          {OBJECT_NAME_KEY: {"$in": list(docMap.keys())}}):
            dbMsgName = r[OBJECT_KEY][G.DataObject.OBJECT_NAME_KEY]
            # Increase the revision number by 1
            docMap[dbMsgName][REVISION_KEY] = r[REVISION_KEY] + 1
        return DBInserter.doCommit(self)  


class CalDBInserter(DBInserter):
    def __init__(self, db, runValid):
        super(CalDBInserter, self).__init__(db, CAL_COLLECTION_NAME)
        self.__runValid = int(runValid)

    def insert(self, calObject):
        data = {}
        data[RUN_VALID_KEY] = self.__runValid
        data[REVISION_KEY] = 0
        data[OBJECT_KEY] = calObject.getdict()
        DBInserter.insert(self, data)

    def doCommit(self):
        """
        Need to support updating, not overwriting, data for a given
        runValid entry.  Use a hidden revision number to ensure we
        always get the most up-to-date data.  We need to hand-roll
        matching instead of using mongo's nice $in operation because
        we need to match two fields
        """
        # Get map of documents with (name, type) tuple as key. Both of
        # these properties are needed to identify a calibration object.
        docMap = getUniqueDocumentMap(self._docs, getNameType)
        for r in self._collection.find({RUN_VALID_KEY: self.__runValid}):
            dbKey = (r[OBJECT_KEY][G.DataObject.OBJECT_NAME_KEY],
                     r[OBJECT_KEY][G.DataObject.OBJECT_TYPE_KEY])
            if dbKey in docMap:
                # Increase the revision number by 1
                docMap[dbKey][REVISION_KEY] = r[REVISION_KEY] + 1
        return DBInserter.doCommit(self) 


class StatusDBInserter(DBInserter):
    def __init__(self, db):
        super(StatusDBInserter, self).__init__(db, STATUS_COLLECTION_NAME)

    def insert(self, statusObject):
        data = {}
        data[OBJECT_KEY] = statusObject.getdict()
        DBInserter.insert(self, data)

    def doCommit(self):
        """
        Need to check if objects already exist, and if they do, make
        sure the existing record and the new record are identical
        """
        # Get map of documents with name as key
        docMap = getUniqueDocumentMap(self._docs, getName)
        # Now check the database for duplicates
        for r in self._collection.find(
                        {OBJECT_NAME_KEY: {"$in": list(docMap.keys())}}):
            dbMsgData = r[OBJECT_KEY]
            dbMsgName = dbMsgData[G.DataObject.OBJECT_NAME_KEY]
            if docMap[dbMsgName][OBJECT_KEY] != dbMsgData:
                raise DBInsertError("Found configuration object with duplicate"
                                    " name but different data: %s" % dbMsgName)
            else:
                # Data is identical.  We don't need to save it.
                docMap.pop(dbMsgName)

        # Insert non-duplicates
        self._docs = docMap.values()
        return DBInserter.doCommit(self)


class QuickInserter(object):

    def __init__(self, db):
        self.__db = db

    def insert(self, collectionName, documents):
        if collectionName not in COLLECTION_NAMES:
            raise DBInsertError("Error: Bad collection: %s" % collectionName)
        collection = self.__db[collectionName]
        try:
            collection.insert_many(documents)
        except Exception as e:
            raise DBInsertError("Error inserting documents: %s" % str(e))
