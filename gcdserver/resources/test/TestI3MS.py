#!/usr/bin/env python

from contextlib import contextmanager
from datetime import datetime
import zmq
import time
import threading

from icecube.gcdserver.I3MS import (I3MSBase, DBInsertHandler, geoDBInserter,
                                    db_receiver_context, I3MS_REQ_PORT,
                                    I3MS_SUB_PORT)
import icecube.gcdserver.Geometry as G
import icecube.gcdserver.MongoDB as MongoDB

from contexts import test_db_context
from TestData import GEOMETRY_1_61


class I3MSReceiver(I3MSBase):

    def __init__(self, handler, host, **kwargs):
        super(I3MSReceiver, self).__init__(zmq.REP, host, I3MS_REQ_PORT,
                                           bind=True, **kwargs)
        self.__handler = handler
        self.__event = threading.Event()

        def task_function():
            while not self.__event.is_set():
                socks = dict(self._poller.poll(self._poll_msec))
                if socks.get(self._socket, 0) & zmq.POLLIN:
                    ret = self.__handler(self._socket.recv_json())
                    socks = dict(self._poller.poll(self._poll_msec))
                    if socks.get(self._socket, 0) & zmq.POLLOUT:
                        self._socket.send_json(ret)
                    else:
                        raise Exception("Timeout transmitting data from I3MS")

        self.__poll_thread = threading.Thread(target=task_function)
        self.__poll_thread.start()

    def close(self):
        self.__event.set()
        self.__poll_thread.join((self._poll_msec / 1000.) + 5.)
        I3MSBase.close(self)


class I3MSPublisher(I3MSBase):

    def __init__(self, host, **kwargs):
        super(I3MSPublisher, self).__init__(zmq.PUB, host, I3MS_SUB_PORT,
                                            bind=True, **kwargs)

    def send(self, topics, data):
        """
        Publish data and topics as a pair.  The idea is that subscribers
        can filter on topic to reduce traffic.  Note that ZMQ provides
        its own topic/filter system, but in some cases (e.g. synchronization)
        we want access to the raw message stream.
        """
        if topics is None:
            topics = [""]
        if type(topics) is not list:
            topics = [topics]
        self._socket.send_json((topics, data))


class SimI3MS(object):
    def __init__(self):
        self.received = []
        self.pub = I3MSPublisher('localhost')

        # Remove all but "data" and "topic"
        def transport(msgs):
            # Accept single messages or lists, same as I3MS
            if type(msgs) is not list:
                msgs = [msgs]
            for msg in msgs:
                new_msg = {}
                new_msg["topic"] = msg["topic"]
                new_msg["data"] = msg["data"]
                self.pub.send(new_msg["topic"], new_msg)
                new_msg["t"] = datetime.utcnow()
                self.received.append(new_msg)
            return "OK"

        self.transporter = I3MSReceiver(transport, 'localhost', poll_msec=100)
        # Ensure 0MQ components have time to start
        time.sleep(0.1)

    def close(self):
        self.pub.close()
        self.transporter.close()


@contextmanager
def i3ms_context():

    simI3MS = SimI3MS()
    # Include database construction in this context
    with test_db_context() as db:
        # We need a second database to insert the message from I3MS
        with test_db_context("test-omdb2") as db2:
            try:
                yield (db, db2)
            finally:
                simI3MS.close()


def test_i3ms():
    with i3ms_context() as (db, db2):
        with db_receiver_context(db2, 'localhost'):
            # Wait for PUB/SUB connection to establish
            time.sleep(0.1)
            with geoDBInserter(db, 'localhost') as geoInserter:
                o = G.GeometryObject.wrapdict(GEOMETRY_1_61)
                geoInserter.insert(o)
                geoInserter.commit()
            for i in range(5000):
                time.sleep(0.1)
                if (MongoDB.countObj(db2[MongoDB.GEO_COLLECTION_NAME]) == 1 and
                    MongoDB.countObj(
                        db2[MongoDB.TRANSACTION_COLLECTION_NAME]) == 1):
                    break
                if i > 50:
                    raise Exception("Never received DB entry from I3MS")
        dbGeo = [d for d in db[MongoDB.GEO_COLLECTION_NAME].find()]
        db2Geo = [d for d in db2[MongoDB.GEO_COLLECTION_NAME].find()]
        assert dbGeo == db2Geo
        dbTrans = [d for d in db[MongoDB.TRANSACTION_COLLECTION_NAME].find()]
        db2Trans = [d for d in db2[MongoDB.TRANSACTION_COLLECTION_NAME].find()]
        assert dbTrans == db2Trans


if __name__ == "__main__":
    test_i3ms()
