#!/usr/bin/env python

from threading import Thread
from datetime import datetime, timedelta
import time
import os
import sys
import socket
import copy

from contexts import test_db_context, config_file_context
from TestData import GEOMETRY_1_61, GEOMETRY_TANK_1A, DOM_DROOP_1_61
from icecube.gcdserver.I3MS import geoDBInserter, calDBInserter
from icecube.gcdserver.MongoDB import fillBlobDB
import icecube.gcdserver.MongoDB as MongoDB
import icecube.gcdserver.ConfigImport as ConfigImport
import icecube.gcdserver.Geometry as G
import icecube.gcdserver.Calibration as C
import icecube.gcdserver.DetectorStatus as D


def assertRaises(func, type=Exception):
    try:
        func()
        raise Exception("The expected exception was not raised")
    except type:
        pass


def getGeometry_1_61():
    return G.GeometryObject.wrapdict(copy.deepcopy(GEOMETRY_1_61))


def getGeometry_Tank_1_A():
    return G.GeometryObject.wrapdict(copy.deepcopy(GEOMETRY_TANK_1A))


def getDomDroop_1_61():
    return G.DataObject.wrapdict(copy.deepcopy(DOM_DROOP_1_61))


def test_geometry_revision():
    """
    Ensure that newer geometry data supersedes the old
    """
    with test_db_context() as db:
        # Import geometry data for DOM 61.  Add a property we can check.
        with geoDBInserter(db) as geoInserter:
            o = getGeometry_1_61()
            o.data["type"] = "old"
            geoInserter.insert(o)
            geoInserter.commit()
            # This should create a new revision of DOM 61 geometry document
            o.data["type"] = "new"
            geoInserter.insert(o)
            geoInserter.commit()
        geoDB = fillBlobDB(db, run=None)
        # Should have one geometry object of "new" type
        docs = [doc for doc in geoDB.geometryDocuments()]
        assert len(docs) == 1
        assert docs[0].data["type"] == "new"
        # Ensure object from DB is the same as original
        assert o.getdict() == docs[0].getdict()


def test_geometry_duplicates():
    """
    Ensure we cannot commit geometry data with duplicate names
    """
    with test_db_context() as db:
    # Import geometry data for DOM 61
        with geoDBInserter(db) as geoInserter:
            o = getGeometry_1_61()
            geoInserter.insert(o)
            geoInserter.insert(o)
            assertRaises(lambda: geoInserter.commit())


def test_calibration_run_valid_revision():
    """
    Ensure we get the appropriate data given document runValid entries
    and the requested run
    """
    with test_db_context() as db:
        o = getDomDroop_1_61()
        with calDBInserter(db, 1) as calInserter:
            o.data["type"] = "run1"
            calInserter.insert(o)
            calInserter.commit()
        with calDBInserter(db, 2) as calInserter:
            o.data["type"] = "run2"
            calInserter.insert(o)
            calInserter.commit()
        # Should get no calibration data
        geoDB = fillBlobDB(db, run=0)
        assert len([x for x in geoDB.calibrationDocuments()]) == 0
        # Should get the run=1 document
        geoDB = fillBlobDB(db, run=1)
        docs = [x for x in geoDB.calibrationDocuments()]
        assert len(docs) == 1
        assert docs[0].data["type"] == "run1"
        # Should get the run=2 document
        geoDB = fillBlobDB(db, run=2)
        docs = [x for x in geoDB.calibrationDocuments()]
        assert len(docs) == 1
        assert docs[0].data["type"] == "run2"
        # Ensure object from DB is the same as original
        assert o.getdict() == docs[0].getdict()
        geoDB = fillBlobDB(db, run=3)
        docs = [x for x in geoDB.calibrationDocuments()]
        assert len(docs) == 1
        assert docs[0].data["type"] == "run2"
        # Revise the run=1 document
        with calDBInserter(db, 1) as calInserter:
            o.data["type"] = "run1_revised"
            calInserter.insert(o)
            calInserter.commit()
        geoDB = fillBlobDB(db, run=0)
        assert len([x for x in geoDB.calibrationDocuments()]) == 0
        geoDB = fillBlobDB(db, run=1)
        docs = [x for x in geoDB.calibrationDocuments()]
        assert len(docs) == 1
        assert docs[0].data["type"] == "run1_revised"
        geoDB = fillBlobDB(db, run=2)
        docs = [x for x in geoDB.calibrationDocuments()]
        assert len(docs) == 1
        assert docs[0].data["type"] == "run2"


def test_calibration_duplicates():
    """
    Ensure we cannot commit calibration data with duplicate
    name / data type
    """
    with test_db_context() as db:
        o = getDomDroop_1_61()
        with calDBInserter(db, 1) as calInserter:
            calInserter.insert(o)
            o.data["new"] = "Some new data"
            calInserter.insert(o)
            # This is a duplicate and should be caught
            assertRaises(lambda: calInserter.commit())
    with test_db_context() as db:
        o = getDomDroop_1_61()
        with calDBInserter(db, 1) as calInserter:
            calInserter.insert(o)
            o.type = "My New Type"
            calInserter.insert(o)
            # This is OK
            calInserter.commit()
    with test_db_context() as db:
        o = getDomDroop_1_61()
        with calDBInserter(db, 1) as calInserter:
            calInserter.insert(o)
            o.name = "Not a DOM Droop"
            calInserter.insert(o)
            # This is OK
            calInserter.commit()


def test_status_duplicates():
    """
    The detector status database has no revisions and duplicate inserts
    are not an error, as long as the duplicates are not simultaneous.
    """
    with test_db_context() as db:
        statusDB = db[MongoDB.STATUS_COLLECTION_NAME]
        with config_file_context("config1", "trig1", "domConf1") as configFile:
            ConfigImport.doInsert(db, [configFile], None)
        # Insert should have been successful
        geoDB = fillBlobDB(db, run=1, configuration="config1")
        triggers = [x for x in geoDB.statusDocuments(
                        objectType=D.ObjectType.TRIGGER_CONFIG_LIST)]
        assert len(triggers) == 1
        assert triggers[0].name == "trig1"
        doms = [x for x in geoDB.statusDocuments(
                        objectType=D.ObjectType.DOM_CONFIG_LIST)]
        assert len(doms) == 1
        assert doms[0].name == "domConf1"
        assert MongoDB.countObj(statusDB) == 3
        # Inserting the same configuration shouldn't raise an exception
        with config_file_context("config1", "trig1", "domConf1") as configFile:
            ConfigImport.doInsert(db, [configFile], None)
        geoDB = fillBlobDB(db, run=1, configuration="config1")
        triggers = [x for x in geoDB.statusDocuments(
                        objectType=D.ObjectType.TRIGGER_CONFIG_LIST)]
        assert len(triggers) == 1
        assert triggers[0].name == "trig1"
        doms = [x for x in geoDB.statusDocuments(
                        objectType=D.ObjectType.DOM_CONFIG_LIST)]
        assert len(doms) == 1
        assert doms[0].name == "domConf1"
        assert MongoDB.countObj(statusDB) == 3
        # Re-inserting a configuration with different data
        # but the same name is an error
        with config_file_context("config1", "trig2", "domConf1") as configFile:
            assertRaises(lambda: ConfigImport.doInsert(db, [configFile], None))
        # Inserting a new configuration with a new trigger file
        # shouldn't add/update the dom config file
        with config_file_context("config2", "trig2", "domConf1") as configFile:
            ConfigImport.doInsert(db, [configFile], None)
        geoDB = fillBlobDB(db, run=1, configuration="config2")
        triggers = [x for x in geoDB.statusDocuments(
                        objectType=D.ObjectType.TRIGGER_CONFIG_LIST)]
        assert len(triggers) == 1
        assert triggers[0].name == "trig2"
        doms = [x for x in geoDB.statusDocuments(
                        objectType=D.ObjectType.DOM_CONFIG_LIST)]
        assert len(doms) == 1
        assert doms[0].name == "domConf1"
        assert MongoDB.countObj(statusDB) == 5
        # Inserting a configuration with a new name but old data should
        # only update the configuration
        with config_file_context("config3", "trig2", "domConf1") as configFile:
            ConfigImport.doInsert(db, [configFile], None)
        assert MongoDB.countObj(statusDB) == 6


def test_transaction():
    
    # Test race condition on transaction number
    with test_db_context() as db:
        transactions = []
        threads = []
        for _ in range(10):
            thread = Thread(target=
                   lambda: transactions.append(MongoDB.DBTransaction(db)))
            thread.start()
            threads.append(thread)

        for thread in threads:
            thread.join(2)

        assert not any(thread.is_alive() for thread in threads)

        transactionNumbers = set(t.getTransactionNumber()
                                             for t in transactions)
        assert len(transactionNumbers) == 10
        assert min(transactionNumbers) == 0
        assert max(transactionNumbers) == 9

        # Now test transaction document validity and state control
        trans = transactions[0]
        doc = trans.getTransactionDocument()
        assert (doc[MongoDB.TRANSACTION_STATUS_KEY] ==
                                 MongoDB.TRANSACTION_STATUS_NEW)
        assert doc[MongoDB.TRANSACTION_CWD_KEY] == os.getcwd()
        assert doc[MongoDB.TRANSACTION_CMDLINE_KEY] == " ".join(sys.argv)
        assert doc[MongoDB.TRANSACTION_HOSTNAME_KEY] == socket.gethostname()
        insertTime = datetime.strptime(doc[MongoDB.TRANSACTION_DATE_KEY],
                                       "%Y-%m-%d %H:%M:%S")
        assert datetime.utcnow() - insertTime < timedelta(seconds=30)

        trans.setCommitted()
        assert trans.getState() == MongoDB.TRANSACTION_STATUS_COMMITTED
        trans.setRollback()
        assert trans.getState() == MongoDB.TRANSACTION_STATUS_ROLLBACK
        trans.setError()
        assert trans.getState() == MongoDB.TRANSACTION_STATUS_ERROR    


def test_inserter():
    with test_db_context() as db:
        inserter = MongoDB.DBInserter(db, MongoDB.GEO_COLLECTION_NAME)
        assert inserter.collectionName() == MongoDB.GEO_COLLECTION_NAME
        assert inserter.docCount() == 0
        inserter.insert({"doc": 1})
        assert inserter.docCount() == 1
        transaction = inserter.commit()
        assert inserter.hasTransaction(transaction)
        assert MongoDB.countObj(db[MongoDB.GEO_COLLECTION_NAME]) == 1
        docs = [doc for doc in db[MongoDB.GEO_COLLECTION_NAME].find()]
        assert docs[0]["doc"] == 1
        assert docs[0][MongoDB.TRANSACTION_KEY] == 0
        assert inserter.getDocumentCount(transaction) == 1
        assert MongoDB.countObj(db[MongoDB.TRANSACTION_COLLECTION_NAME]) == 1
        trans = [doc for doc in db[MongoDB.TRANSACTION_COLLECTION_NAME].find()]
        transData = inserter.getTransactionData(transaction)
        assert len(transData[MongoDB.GEO_COLLECTION_NAME]) == 1
        assert transData[MongoDB.GEO_COLLECTION_NAME][0] == docs[0]
        assert len(transData[MongoDB.TRANSACTION_COLLECTION_NAME]) == 1
        assert transData[MongoDB.TRANSACTION_COLLECTION_NAME][0] == trans[0]
        inserter.rollback(transaction)
        assert not inserter.hasTransaction(transaction)
        assert MongoDB.countObj(db[MongoDB.GEO_COLLECTION_NAME]) == 0
        assert MongoDB.countObj(db[MongoDB.TRANSACTION_COLLECTION_NAME]) == 1
        trans = [doc for doc in db[MongoDB.TRANSACTION_COLLECTION_NAME].find()]
        assert (trans[0][MongoDB.TRANSACTION_STATUS_KEY] == 
                                     MongoDB.TRANSACTION_STATUS_ROLLBACK)
        inserter.insert("bad document")
        assertRaises(lambda: inserter.commit(), MongoDB.DBInsertError)


def test_quick_inserter():
    with test_db_context() as db:
        q = MongoDB.QuickInserter(db)
        q.insert(MongoDB.GEO_COLLECTION_NAME, [{"doc": 1}, {"doc": 2}])
        assert MongoDB.countObj(db[MongoDB.GEO_COLLECTION_NAME]) == 2
        docs = [doc for doc in db[MongoDB.GEO_COLLECTION_NAME].find()]
        assert docs[0]["doc"] in [1, 2]
        assert docs[1]["doc"] in [1, 2]


def test_blob_db():
    with test_db_context() as db:
        # Test geometry routines
        dom_1_61 = getGeometry_1_61()
        tank_1_A = getGeometry_Tank_1_A()
        with geoDBInserter(db) as geoInserter:
            geoInserter.insert(dom_1_61)
            geoInserter.insert(tank_1_A)
            geoInserter.commit()

        geoDB = fillBlobDB(db, run=None)
        deployed = [x for x in geoDB.deployedDevices()]
        assert len(deployed) == 1
        ((string, om, pmt), device) = deployed[0]
        assert device.name == dom_1_61.name

        assert len([x for x in geoDB.geometryDocuments()]) == 2
        doms = [x for x in geoDB.geometryDocuments(
                            objectType=G.ObjectType.ICETOP_DOM)]
        assert len(doms) == 1
        assert doms[0].name == dom_1_61.name
        assert geoDB.mbidMap(dom_1_61.data[G.Keys.MBID]) == dom_1_61.name
        assert geoDB.omKeyMap(string, om, pmt) == dom_1_61.name
        assert geoDB.deployedNameMap(dom_1_61.name) == doms[0]
        assert geoDB.tankMap(tank_1_A.name) == (1, "A")
        
        # Test calibration
        droop = getDomDroop_1_61()
        with calDBInserter(db, 1) as calInserter:
            calInserter.insert(droop)
            calInserter.commit()

        geoDB = fillBlobDB(db, run=1)

        assert len([x for x in geoDB.calibrationDocuments()]) == 1
        assert len([x for x in geoDB.calibrationDocuments(
                         objectType=C.ObjectType.TOROID_DROOP)]) == 1
        assert len([x for x in geoDB.calibrationDocuments(
                         objectType="Not a calibration")]) == 0
        assert len([x for x in geoDB.calibrationDocuments(
                                    deviceName="Not a device")]) == 0
        assert len([x for x in geoDB.calibrationDocuments(
                                    deviceName=droop.name)]) == 1

        # Test detector status
        with config_file_context("config") as configFile:
            ConfigImport.doInsert(db, [configFile], None)

        geoDB = fillBlobDB(db, run=1, configuration="config")
        assert len([x for x in geoDB.statusDocuments()]) == 2
        assert len([x for x in geoDB.statusDocuments(
                        objectType=D.ObjectType.DOM_CONFIG_LIST)]) == 1


if __name__ == "__main__":
    test_geometry_revision()
    test_geometry_duplicates()
    test_calibration_run_valid_revision()
    test_calibration_duplicates()
    test_status_duplicates()
    test_transaction()
    test_inserter()
    test_quick_inserter()
    test_blob_db()
