import os
import sys

from icecube import icetray
from icecube.icetray import I3Module
from icecube.production_histograms.histograms.histogram import Histogram
from icecube.production_histograms.histogram_modules.histogram_module import HistogramModule
from icecube.production_histograms.db import create_simprod_db_client

if sys.version_info[0] >= 3:
    import pickle
else:
    import cPickle as pickle

def instance_of(h):
    '''
    This function simply returns an instance of the object
    if it's a Histogram or HistogramModule.
    '''
    classes = (Histogram, HistogramModule)
    try:
        if isinstance(h, classes):
            return h
        elif issubclass(h, classes):
            return h()
        else:
            icetray.logging.log_warn("%s is neither a Histogram or HistogramModule.  Ignoring." % str(h))
    except TypeError:
        print("Bad classinfo")
        print(classes)

def parameter_conversion(container):
    '''
    ProductionHistogramModule takes 'Histograms' as a parameter.
    This can contain both Histogram and HistogramModule object
    object and class instances...as well as iterables of each.

    This function converts class instances to object instances, 
    returning a flat list of Histogram or HistogramModule object 
    instances.    
    '''
    instances = list()
    for obj in container:
        if hasattr(obj, '__iter__'):
            if isinstance(obj, dict):
                for k,v in obj.items():
                    instances.append(instance_of(v))
            else:
                instances.extend(map(instance_of, obj))
        else:
            instances.append(instance_of(obj))
    return instances
    
class ProductionHistogramModule(I3Module) :
    '''
    This I3Module passes frames to any histograms and histogram modules
    loaded in the user-defined configuration.  

    If OutputFilename is set then the output is a pickle file which contains 
    a dictionary (key = name, data = histogram).

    If DataSet is set then histograms for that dataset are updated, or created
    on the first instance.

    OutputFilename and DataSet are independent of each other, so it's possible
    to write pickle files *and* update the DB.

    It's also possible to use different prescales for different streams by passing
    a dictionary (key = stream, data = prescale).
    '''
    def __init__( self, context ):
        super(ProductionHistogramModule, self).__init__(context)

        self.AddParameter("Histograms", "List of histograms or modules to run.", list() )
        self.AddParameter("OutputFilename", "Name of output pickle file.", None )
        self.AddParameter("CollectionName", "Name of the MongoDB collection.", None )
        self.AddParameter("FilenameList", "List of I3Files passed to I3Reader", None) 
        self.AddParameter("PasswordPath", "Path to plain text password file for the DB.", None) 
        self.AddParameter("Prescales", "Dictionary of frame type to prescale",  
                          {"Geometry" : 1,
                           "Calibration": 1,
                           "DetectorStatus": 1,
                           "DAQ": 1,
                           "Physics": 1,
                           })

        self._frame_counter = 0

    def Configure(self):
        self.outfilename = self.GetParameter("OutputFilename")
        self.collection_name = self.GetParameter("CollectionName")
        self.filenamelist = self.GetParameter("FilenameList")
        self.prescales = self.GetParameter("Prescales")        
        self.histograms = parameter_conversion(self.GetParameter("Histograms"))
        
        _password_path = self.GetParameter("PasswordPath")
        self.password_path = _password_path \
                             if _password_path \
                                else os.path.join(os.environ["HOME"], '.mongo')
        
        # initialize the frame counters
        self._frame_counters = dict()
        for key in self.prescales.keys():
            self._frame_counters[key] = 0

        if self.collection_name and not self.filenamelist:
            icetray.logging.error('FilenameList is empty and CollectionName is set.')
            icetray.logging.error('When writing to the DB you have to pass the '
                                  'list of filenames used, since the histograms are '
                                  'summed on the fly.\n  This protects against double-counting.')
            icetray.logging.fatal('Please set FilenameList to the same value passed to I3Reader.')

            
        # If both are configured then the assumption is
        # that a set of files are being read and written
        # to the DB.  Need to guard against double counting.
        if self.filenamelist and self.collection_name:
            configured_filenamelist_len = len(self.filenamelist)
            print "EMILY", self.password_path
            client = create_simprod_db_client(self.password_path)
            db = client.simprod_histograms

            collection = db[str(self.collection_name)]
            if collection:
                # check to see if these I3Files have been histogrammed
                # already and fail to avoid double counting.            
                filelist = collection.find_one({'name': 'filelist'})

                if filelist:
                    for fn in filelist['files']:
                        if fn in self.filenamelist:
                            icetray.logging.log_warn("%s has been histogrammed already")
                            self.filenamelist.remove(fn)

            if not self.filenamelist:
                icetray.logging.log_error("FilenameList is now empty.")
                icetray.logging.log_error("Configured length = %d" % configured_filenamelist_len)
                if configured_filenamelist_len > 0:
                    icetray.logging.log_fatal("All of these files have likely been histogrammed already.")
                else:
                    icetray.logging.log_fatal("Configured FilenameList is empty.")
                
    def _Process(self, frame):
        '''
        This method is called by all IceTray methods and only need one, 
        and I don't want to repeat a bunch of code.  DRY.
        '''
        stream_key = str(frame.Stop)
        self._frame_counters[stream_key] += 1
        if self._frame_counters[stream_key] % self.prescales[stream_key] != 0 :
            self.PushFrame(frame)
            return        

        for histogram in self.histograms :
            getattr(histogram, str(frame.Stop))(frame)

        self.PushFrame(frame)

    def Geometry(self, frame):
        self._Process(frame)

    def Calibration(self, frame):
        self._Process(frame)

    def DetectorStatus(self, frame):
        self._Process(frame)
        
    def Physics(self, frame):
        self._Process(frame)
        
    def DAQ(self, frame):
        self._Process(frame)

    def __write_to_file(self, histograms):
        # Serialize the state dictionary instead of the object
        # so it doesn't remember where it came from.  There's
        # no good reason to require icetray to read and view histograms.
        # This also plays nicer with code that would want to read from
        # the DB or pickle files.

        state = {h.name: h.__getstate__() for h in histograms}
        state['filelist'] = {'files': self.filenamelist}
        
        pickle.dump(state,
                    open( self.outfilename, "wb"),
                    pickle.HIGHEST_PROTOCOL)

    def __write_to_database(self, histograms):

        client = create_simprod_db_client(self.password_path)
        if not client:
            return
        
        db = client.simprod_histograms
        collection = db[str(self.collection_name)]

        icetray.logging.log_info('Loading %d histograms into the DB' % len(histograms))
        for histogram in histograms:            
            h = collection.find_one({'name': histogram.name})
            if h:
                # update the histogram by add bin contents
                bin_values = [b1 + b2 for b1, b2 in zip(h['bin_values'], histogram.bin_values)]
                h['bin_values'] = bin_values
                h['overflow'] += histogram.overflow
                h['underflow'] += histogram.underflow
                h['nan_count'] += histogram.nan_count                
                result = collection.replace_one({'_id': h['_id']}, h)
            else:
                # this is the first histogram...add it
                result = collection.insert_one(histogram.__getstate__())
            icetray.logging.log_debug(str(result))

        # log which files make up these histograms
        # multiples are fine...it'll allow us to catch double counting, 
        # mismatched GCD files, etc...just log what was done
        # and don't try to get too fancy here.
        filelist = collection.find_one({'name': 'filelist'})
        if not filelist:
            filelist = {'name': 'filelist',
                        'files': self.filenamelist}
            result = collection.insert_one(filelist)
        else:
            filelist['files'].extend(filelist)
            result = collection.replace_one({'_id': filelist['_id']}, filelist)

        
    def Finish(self):
        '''
        Write the histograms to an output pickle file or DB.
        The output is a dictionary of histograms where
        the key is the name of the histogram.
        '''
        histograms = list()
        print("There were %d histograms generated by this module." % len(self.histograms))
        for histogram in self.histograms :
            if isinstance(histogram, Histogram):
                histograms.append(histogram)
            if isinstance(histogram, HistogramModule):
                histograms.extend([h for n,h in histogram.histograms.iteritems()])

        all_empty = not any([any(h.bin_values) for h in histograms])
        if all_empty:
            icetray.logging.log_error("All the histograms are empty.")
                
        if self.outfilename:
            self.__write_to_file(histograms)

        if self.collection_name:            
            self.__write_to_database(histograms)


