#!/usr/bin/env python

from datetime import datetime, timedelta
import json
import sys
from icecube.gcdserver.MongoDB import getDB, getDocumentCounts
from icecube.gcdserver.I3MS import (i3ms_sender_context, I3MSWrap,
                                    I3MS_DB_SERVER_PORT, I3MS_REQ_PORT)
import icecube.gcdserver.Email as Email
from icecube.gcdserver.OptionParser import GCDOptionParser


DB_CONSISTENCY_TOPIC = "dbConsistency"
EMAIL_SUBJECT = "Database Consistency Check Error"


def fetchDBConsistency(lookbackSeconds, host, **kwargs):
    match = {"topic": DB_CONSISTENCY_TOPIC}
    startTime = (datetime.utcnow() - timedelta(seconds=lookbackSeconds))
    match["startTime"] = str(startTime)
    with i3ms_sender_context(host, I3MS_DB_SERVER_PORT, **kwargs) as sender:
        response = sender.send(match)
        return [m['data'] for m in response]


def sendDBConsistency(record, host, **kwargs):
    with i3ms_sender_context(host, I3MS_REQ_PORT, **kwargs) as sender:
        ret = sender.send(I3MSWrap(record, DB_CONSISTENCY_TOPIC))
        assert ret == 'OK'


def main():
    parser = GCDOptionParser()
    parser.add_option('--send', dest='send',
                      default=False, action='store_true',
                      help='Send consistency record over I3MS')
    parser.add_option('--check', dest='check',
                      default=False, action='store_true',
                      help='Check consistency against records in I3MS')
    parser.add_option("--recipient", dest="recipient",
                      help="Email recipient to receive error reports")
    parser.add_option("--lookbackSeconds", dest="lookbackSeconds",
                      default=300000,
                      help="Look back this many seconds for DB records")
    parser.add_option("--sender", dest="sender",
                      default="omdb-noreply@icecube.wisc.edu",
                      help="Email sender address")
    parser.add_option("--email_server", dest="email_server",
                      default="mail.icecube.wisc.edu",
                      help="Email server host name")
    (options, args) = parser.parse_args()
    if options.i3mshost is None:
        print "I3MS host not specified"
        parser.print_help()
        sys.exit(-1)
    db = getDB(options.dbhost, options.dbuser, options.dbpass)
    documentCounts = getDocumentCounts(db)
    if options.send:
        sendDBConsistency(documentCounts, options.i3mshost, poll_msec=60000)
    if options.check:
        i3msDocumentCounts = fetchDBConsistency(int(options.lookbackSeconds),
                                        options.i3mshost, poll_msec=60000)

        # We have an error if current document counts do not agree with
        # any of the latest two document counts received from the other side.
        if len(i3msDocumentCounts) > 1:
            if not any(m == documentCounts for m in i3msDocumentCounts[-2:]):
                # Error.  Send an email.
                body = ("Remote documents: %s\nLocal documents: %s" %
                                    (i3msDocumentCounts[-1], documentCounts))
                recipients = options.recipient.split(',')
                Email.sendMessage(recipients, options.sender,
                                  EMAIL_SUBJECT, options.email_server, body)


if __name__ == "__main__":
    main()
