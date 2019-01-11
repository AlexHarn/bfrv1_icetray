#!/usr/bin/env python

import pymongo
import sys

import icecube.gcdserver.MongoDB as MongoDB
from icecube.gcdserver.OptionParser import GCDOptionParser
from icecube.gcdserver.I3MS import getSender

from signal import signal, SIGPIPE, SIG_DFL
signal(SIGPIPE,SIG_DFL)


def getInserter(db, transaction):
    inserters = [MongoDB.GeoDBInserter(db),
                 MongoDB.CalDBInserter(db, 0),
                 MongoDB.StatusDBInserter(db)]
    for inserter in inserters:
        # Only one collection may have any given transaction number
        if inserter.hasTransaction(transaction):
            return inserter
    print ("Error: No documents matching transaction %s" %
                            transaction.getTransactionNumber())
    sys.exit(1)


def rollback(db, transaction):
    getInserter(db, transaction).rollback(transaction)


def replay(db, i3msHost, transaction):
    data = getInserter(db, transaction).getTransactionData(transaction)
    # document is dictionary with collection name as key and
    # a list of DB documents as each value.  Convert the '_id'
    # field to string type
    for docs in data.itervalues():
        for m in docs:
            m['_id'] = str(m['_id'])
    try:
        getSender(i3msHost).send(data)
    except Exception as e:
        print "Unable to forward data: %s" % str(e)


def printTransaction(t):
    transaction = t.pop(MongoDB.TRANSACTION_KEY)
    print "\n%s: %s" % (MongoDB.TRANSACTION_KEY, transaction)
    print "-" * 50
    for k in t:
        print "%s: %s" % (str(k), str(t[k])[:200])


def listTransactions(db, count):
    collection = db[MongoDB.TRANSACTION_COLLECTION_NAME]
    for trans in reversed(list(collection.find(limit=count,
                    sort=[(MongoDB.TRANSACTION_KEY, pymongo.DESCENDING)]))):
        printTransaction(trans)


if __name__ == "__main__":
    parser = GCDOptionParser()
    parser.add_option("--list", dest="list",
                      help="Print details for the most recent n transactions")
    parser.add_option("--replay", dest="replay",
                      help="Resend the given transaction number via I3MS")
    parser.add_option("--rollback", dest="rollback",
                      help="Roll back the given transaction number")
    (options, args) = parser.parse_args()
    if (options.list == None and
        options.replay == None and
        options.rollback == None):
        print "No command specified"
        parser.print_help()
        sys.exit(1)

    db = MongoDB.getDB(options.dbhost, options.dbuser, options.dbpass)
    if options.replay:
        transaction = MongoDB.DBTransaction(db, int(options.replay))
        if options.i3mshost is None:
            print "I3MS host name required for replay"
            parser.print_help()
            sys.exit(1)
        replay(db, options.i3mshost, transaction)

    if options.rollback:
        transaction = MongoDB.DBTransaction(db, int(options.rollback))
        rollback(db, transaction)

    if options.list:
        listTransactions(db, int(options.list))