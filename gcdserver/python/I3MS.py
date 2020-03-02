
from contextlib import contextmanager
import zmq
import bson
import threading

from icecube.gcdserver.MongoDB import (GeoDBInserter, CalDBInserter,
                                       QuickInserter, StatusDBInserter,
                                       EmptyTransaction)


MSG_TOPIC = "gcdUpdate"
MSG_SERVICE = "gcdserver"
I3MS_REQ_PORT = 6015
I3MS_SUB_PORT = 6012
I3MS_DB_SERVER_PORT = 6018


class I3MSBase(object):
    def __init__(self, socketType, host, port, bind=False,
                 poll_msec=5000, linger_msec=1000):
        self.__context = zmq.Context()
        self._socket = self.__context.socket(socketType)
        if bind:
            self._socket.bind("tcp://*:%s" % port)
        else:
            self._socket.connect("tcp://%s:%s" % (host, port))
        self._poll_msec = poll_msec
        self._socket.linger = linger_msec
        self._poller = zmq.Poller()
        self._poller.register(self._socket, zmq.POLLIN | zmq.POLLOUT)

    def close(self):
        self._poller.unregister(self._socket)
        self._socket.close()
        self.__context.term()


class I3MSSender(I3MSBase):
    """
    ZMQ REQ client for sending data to I3MS
    """
    def __init__(self, host, port, **kwargs):
        super(I3MSSender, self).__init__(zmq.REQ, host, port, **kwargs)

    def send(self, msg):
        socks = dict(self._poller.poll(self._poll_msec))
        if socks.get(self._socket, 0) & zmq.POLLOUT:
            self._socket.send_json(msg)
            socks = dict(self._poller.poll(self._poll_msec))
            if socks.get(self._socket, 0) & zmq.POLLIN:
                return self._socket.recv_json()
            else:
                raise Exception("Timeout receiving reply from I3MS")
        else:
            raise Exception("ZMQ socket not available for sending")


@contextmanager
def i3ms_sender_context(host, port, **kwargs):
    s = I3MSSender(host, port, **kwargs)
    try:
        yield s
    finally:
        s.close()


def I3MSWrap(msg, topic):
    """
    Wrap up an I3MS transmission message with topic and service
    """
    packet = {'data': msg}
    packet['service'] = MSG_SERVICE
    packet['topic'] = topic
    return packet


class I3MSTransactionSender(object):
    """
    Format transaction data and send it to I3MS via I3MSSender.
    Set topic and check I3MS return.
    """
    def __init__(self, host, **kwargs):
        self._sender = I3MSSender(host, I3MS_REQ_PORT, **kwargs)

    def send(self, msg):
        ret = self._sender.send(I3MSWrap(msg, MSG_TOPIC))
        if ret != 'OK':
            raise Exception("Received bad return from I3MS: %s" % ret)

    def close(self):
        self._sender.close()


class I3MSSubscriber(I3MSBase):
    """
    ZMQ SUB client for receiving transaction data from I3MS
    """
    def __init__(self, handler, host, **kwargs):
        super(I3MSSubscriber, self).__init__(zmq.SUB, host,
                                             I3MS_SUB_PORT, **kwargs)
        self._socket.setsockopt(zmq.SUBSCRIBE, "".encode())
        self.__event = threading.Event()

        def task_function():
            while not self.__event.is_set():
                socks = dict(self._poller.poll(self._poll_msec))
                if socks.get(self._socket, 0) & zmq.POLLIN:
                    (msgTopics, msgData) = self._socket.recv_json()
                    if type(msgTopics) is not list:
                        msgTopics = [msgTopics]
                    if MSG_TOPIC in msgTopics:
                        try:
                            handler(msgData['data'])
                        except:
                            # Don't abort the process
                            pass

        self.__poll_thread = threading.Thread(target=task_function)
        self.__poll_thread.start()

    def close(self):
        self.__event.set()
        self.__poll_thread.join((self._poll_msec / 1000.) + 5.)
        I3MSBase.close(self)


class DBReceiver():
    """
    Wrap a I3MSSubscriber instance with a routine to insert GCD
    data into the proper collection in the omdb Mongo database
    """
    def __init__(self, db, i3msHost):
        self.__inserter = QuickInserter(db)

        def handler(data):
            for (collection, docs) in iter(data.items()):
                for m in docs:
                    # Revert the ObjectID stringification
                    m['_id'] = bson.ObjectId(m['_id'])
                # Insert the documents
                self.__inserter.insert(collection, docs)

        self.__subscriber = I3MSSubscriber(handler, i3msHost, poll_msec=1000)

    def close(self):
        self.__subscriber.close()


@contextmanager
def db_receiver_context(db, i3msHost):
    rec = DBReceiver(db, i3msHost)
    try:
        yield
    finally:
        rec.close()


class NullSender(object):

    def __init__(self):
        pass

    def send(self, msgList):
        pass

    def close(self):
        pass


def getSender(i3msHost):
    if i3msHost is None:
        return NullSender()
    return I3MSTransactionSender(i3msHost, poll_msec=60000)    


class DBInsertHandler(object):
    """
    Wrap commit() method of inserter to both insert the data into the
    local database and forward the data to I3MS.  Roll back the local
    transaction if I3MS forwarding fails
    """
    def __init__(self, inserter, i3msHost, force=False):
        self.__sender = getSender(i3msHost)
        self.__inserter = inserter
        self.__force = force

    def insert(self, msg):
        self.__inserter.insert(msg)

    def __forwardTransactionData(self, transaction):
        # Get the messages back from the DB
        data = []
        try:
            data = self.__inserter.getTransactionData(transaction)
            # document is dictionary with collection name as key and
            # a list of DB documents as each value.  Convert the '_id'
            # field to string type
            for docs in data.values():
                for m in docs:
                    m['_id'] = str(m['_id'])
            # Forward the messages
            self.__sender.send(data)
        except Exception as e:
            raise Exception(
                    "Unable to prepare transaction for sending: %s" % str(e))

    def commit(self):
        if self.__inserter.docCount() == 0:
            print("No documents to commit")
            return
        # Insert the documents
        print ("Committing %d document(s) to %s collection." %
                                       (self.__inserter.docCount(),
                                        self.__inserter.collectionName()))
        transaction = None
        try:
            transaction = self.__inserter.commit()
        except EmptyTransaction:
            print("No new documents to commit")
            return
        newDocumentCount = self.__inserter.getDocumentCount(transaction)
        print("Committed %d new documents" % newDocumentCount)
        transactionNumber = transaction.getTransactionNumber()
        try:
            self.__forwardTransactionData(transaction)
            print("Transaction %d complete" % transactionNumber)
        except Exception as e:
            # Roll back the transaction
            print("Error forwarding transaction: %s" % str(e))
            if not self.__force:
                print("Rolling back transaction %d" % transactionNumber)
                self.__inserter.rollback(transaction)
                raise
            else:
                print("Transaction complete, but not forwarded")

    def close(self):
        self.__sender.close()


@contextmanager
def geoDBInserter(db, i3msHost=None, force=False):
    inserter = GeoDBInserter(db)
    handler = DBInsertHandler(inserter, i3msHost, force=force)
    try:
        yield handler
    finally:
        handler.close()


@contextmanager
def calDBInserter(db, runValid, i3msHost=None, force=False):
    inserter = CalDBInserter(db, runValid)
    handler = DBInsertHandler(inserter, i3msHost, force=force)
    try:
        yield handler
    finally:
        handler.close()


@contextmanager
def statusDBInserter(db, i3msHost=None, force=False):
    inserter = StatusDBInserter(db)
    handler = DBInsertHandler(inserter, i3msHost, force=force)
    try:
        yield handler
    finally:
        handler.close()
