#!/usr/bin/python

import os
import sys
import re
import datetime
import MySQLdb

#-------------------------------------------------
# Some convenience functions for database queries
#-------------------------------------------------

def getNextCalDate(cursor, date):
    sqlCmd="SELECT ValidityStartDate FROM CalibrationDetail WHERE TypeId=51 AND ValidityStartDate>'"
    sqlCmd=sqlCmd + date.strftime("%Y-%m-%d %H:%M:%S")
    sqlCmd=sqlCmd + "' ORDER by CaId"
    
    cursor.execute(sqlCmd)
    row = cursor.fetchone()
    if(row==None):
        return None
    else:
        return row[0]


def getFirstRunAfter(cursor, date):
    sqlCmd="SELECT run FROM run_summary WHERE run_mode='PhysicsTrig' AND start>'"
    sqlCmd=sqlCmd + date.strftime("%Y-%m-%d %H:%M:%S")
    sqlCmd=sqlCmd + "' ORDER by run"
    
    cursor.execute(sqlCmd)
    row = cursor.fetchone()
    if(row==None):
        return None
    else:
        return row[0]


def getStartTime(cursor, runID):
    sqlCmd="SELECT start FROM run_summary WHERE run=%d" % runID
    
    cursor.execute(sqlCmd)
    row = cursor.fetchone()
    if(row==None):
        return None
    else:
        return row[0]
