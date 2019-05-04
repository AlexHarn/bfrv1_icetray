import os
from os import environ
from os.path import join
from os.path import exists
from icecube import icetray
    
def create_simprod_db_client(database_url = 'mongodb-simprod.icecube.wisc.edu',
                             dbuser = 'DBadmin',
                             password_path = os.path.expandvars('$HOME/.mongo')):
    
    try:
        from pymongo import MongoClient
    except ImportError:
        icetray.logging.log_fatal("PyMongo not installed.")

    if not exists(password_path):
        icetray.logging.log_fatal("Password file '%s' not found." % password_path)
        
    f = open(password_path)
    uri = "mongodb://%s:%s@%s" % (dbuser, f.readline().strip(), database_url)
    f.close()
    
    client = MongoClient(uri)

    return client

class SimProdHistogramDB:
    def __init__(self, collection_name = None):

        try:
            from pymongo import MongoClient
        except ImportError:
            logging.critical("PyMongo not installed.")

        database_url = 'mongodb-simprod.icecube.wisc.edu',
        uri = "mongodb://icecube:skua@%s" % (database_url)
        client = MongoClient(uri)
        self.db = client['simprod_histograms']

        if collection_name:
            self.set_collection(collection_name)

    def collection_names(self):
        return self.db.list_collection_names()
        
    def set_collection(self, collection_name):
        self.collection_name = collection_name
        self.collection = self.db[self.collection_name]
            
    def histograms(self):
        return [h['name'] for h in self.collection.find({'name' : {'$ne':'filelist'}})] 

    def get_histogram(self, collection_name, histogram_name):
        return self.collection.find_one({'name' : histogram_name})

    def get_filelist(self, collection_name):
        return self.collection.find_one({'name':'filelist'})['files'] 
        
