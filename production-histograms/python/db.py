from os import environ
from os.path import join
from os.path import exists
from icecube import icetray
    
def create_simprod_db_client(database_url = 'mongodb-simprod.icecube.wisc.edu',
                             dbuser = 'DBadmin',
                             password_path = expandvars('$HOME/.mongo')):
    
    try:
        from pymongo import MongoClient
    except ImportError:
        icetray.logging.log_fatal("PyMongo not installed.")

    if not exists(password_path):
        icetray.logging.log_fatal("Credentials not found.")
        
    f = open(password_path)
    uri = "mongodb://%s:%s@%s" % (dbuser, f.readline().strip(), database_url)
    f.close()
    
    client = MongoClient(uri)

    return client


