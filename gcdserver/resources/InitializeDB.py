#!/usr/bin/env python

import icecube.gcdserver.MongoDB as MongoDB
from icecube.gcdserver.OptionParser import GCDOptionParser


"""
Create the gcdserver MongoDB database, including indexes.
Optionally delete existing data.  Use with caution.
"""
def main():
    parser = GCDOptionParser()
    parser.add_option("-d", "--drop", dest="drop", action="store_true",
                      help="Drop old data", default=False)
    (options, args) = parser.parse_args()
    db = MongoDB.getDB(options.dbhost, options.dbuser, options.dbpass)
    if options.drop:
        print "Dropping existing tables"
        MongoDB.destroyDB(db)
    MongoDB.initDB(db)


if __name__ == "__main__":
    main()