import os
import sys
    
def create_simprod_db_client():

    try:
        from pymongo import MongoClient
    except ImportError:
        icetray.logging.log_error("PyMongo not installed.")
        return
        
    try:
        path = os.path.join(os.environ["HOME"], '.mongo')
    except IOError:
        icetray.logging.log_error("Credentials not found. Histograms won't be uploaded.")
        return
        
    f = open(path)        
    client = MongoClient("mongodb://DBadmin:%s@mongodb-simprod.icecube.wisc.edu" % 
                         f.readline().strip())

    return client


