from os import environ
from os.path import join
from os.path import exists
    
def create_simprod_db_client(database_url = 'mongodb-simprod.icecube.wisc.edu',
                             dbuser = 'DBadmin',
                             password_path = join(environ["HOME"], '.mongo')):
    
    from icecube import icetray
    try:
        from pymongo import MongoClient
    except ImportError:
        icetray.logging.log_error("PyMongo not installed.")
        return

    if not exists(password_path):
        icetray.logging.log_error("Credentials not found.")
        return
        
    f = open(password_path)
    uri = "mongodb://%s:%s@%s" % (dbuser, f.readline().strip(), database_url)
    f.close()
    
    client = MongoClient(uri)

    return client


