import os

from icecube import icetray, dataio, dataclasses, simclasses, recclasses

from icecube.production_histograms.find_gcd_file import find_gcd_file
from icecube.production_histograms.categorize import categorize


from icecube.production_histograms.histogram_modules.simulation.mctree_primary import I3MCTreePrimaryModule
from icecube.production_histograms.histogram_modules.simulation.mctree import I3MCTreeModule
from icecube.production_histograms.histogram_modules.simulation.mcpe_module import I3MCPEModule
from icecube.production_histograms.histogram_modules.simulation.pmt_response import PMTResponseModule
from icecube.production_histograms.histogram_modules.simulation.dom_mainboard_response import InIceResponseModule
from icecube.production_histograms.histogram_modules.simulation.trigger import TriggerModule
from icecube.production_histograms.histograms.simulation.secondary_multiplicity import SecondaryMultiplicity
from icecube.production_histograms.histograms.simulation.noise_occupancy import NoiseOccupancy
from icecube.production_histograms.histograms.particle_histogram_generator import generate_particle_histograms

_type_to_histogram = {
    dataclasses.I3MCTree: [I3MCTreePrimaryModule, I3MCTreeModule, SecondaryMultiplicity],
    simclasses.I3MCPESeriesMap: [I3MCPEModule, NoiseOccupancy],
    simclasses.I3MCPulseSeriesMap: [PMTResponseModule],
    dataclasses.I3DOMLaunchSeriesMap: [InIceResponseModule],
    dataclasses.I3TriggerHierarchy: [TriggerModule],
    dataclasses.I3Particle: []
}

def _known_type(frame_object):
    for frame_type in _type_to_histogram.keys():
        if isinstance(frame_object, frame_type):
            return True
    return False

def _get_modules(frame_object):
    for frame_type in _type_to_histogram.keys():
        if isinstance( frame_object, frame_type):
            return _type_to_histogram[frame_type]    

def _log_corrupt_file(filename):
    icetray.logging.log_info("This file is corrupt: %s" % filename)
    category = categorize(filename)
    icetray.logging.log_debug("    %s" % category)
    
    from pymongo import MongoClient
    f = open('/home/olivas/.mongo')
    client = MongoClient("mongodb://DBadmin:%s@mongodb-simprod.icecube.wisc.edu" %
                         f.readline().strip())

    collection = client.simprod_filecatalog.filelist
    document = collection.find_one({'category': category})
                
    if 'corrupt_filelist' not in document:
        document['corrupt_filelist'] = list()

    if filename not in document['corrupt_filelist']:
        document['filelist'].remove(filename)
        document['corrupt_filelist'].append(filename)

        result = collection.update({'_id': document['_id']}, document)
        print(result)

def _configure_from_frame(frame, frame_key):
    '''
    Pull the frame object out of the frame.  Not all objects are
    going to be serializable, so we catch that and just skip over it.
    If we don't have the library loaded, we can't histogram it.
    '''
    histograms = list()
    try:
        frame_object = frame[frame_key]
    except KeyError:
        return
    
    if _known_type(frame_object):
        # this is something we want to histogram
        if isinstance(frame_object, dataclasses.I3Particle):
            histograms.extend(generate_particle_histograms(frame_key))
        else:
            for module in _get_modules(frame_object):
                m = module()
                m.frame_key = frame_key
                histograms.append(m)
    return histograms
                    
def _configure(filename, histograms):
    '''
    When we can't get a frame from an I3File, that's
    an indication that the file is corrupt, so we catch
    that and log the file in the database.
    '''
    try:
        i3file = dataio.I3File(filename)
        for frame in i3file:
            for frame_key in frame.keys():
                if frame_key not in histograms:
                    hlist = _configure_from_frame(frame, frame_key)
                    if hlist:
                        histograms[frame_key] = hlist

    except RuntimeError:
        _log_corrupt_file(filename)
        
    finally:
        print("configure: %d" % len(histograms))
        return histograms

def generate_histogram_configuration_list(i3files):
    '''
    Given a path, peek at the I3Files, loop over
    some frames, and figure out which histograms
    need to be loaded.
    '''
    histograms = dict()

    filelist = [i3files] if isinstance(i3files, str) else i3files
    
    file_counter = 0
    for filename in filelist:
        file_counter += 1
        print("Processing file (%d/%d): %s " % (file_counter, len(filelist), filename))
        _configure(filename, histograms)

    histogram_list = list()
    for frame_key, hl in histograms.iteritems():
        histogram_list.extend(hl) 
    return histogram_list

def generate_filelist(path):
    # first find the GCD file, because that needs
    # to be the first file in the list.
    gcdfile = find_gcd_file(path)
    if not gcdfile:
        print("Could not find GCD file in %s" % path)
        sys.exit()
    filelist = [gcdfile]

    from pymongo import MongoClient        
    f = open('/home/olivas/.mongo')
    client = MongoClient("mongodb://DBadmin:%s@mongodb-simprod.icecube.wisc.edu" %
                         f.readline().strip())
    collection = client.simprod_filecatalog.filelist
    
    category = categorize(path)
    document = collection.find_one({'category': category})

    # icetray doesn't like unicode strings and the purpose of
    # this function is to produce something that can be
    # passed directly to I3Reader.
    filelist.extend([str(fn) for fn in document['filelist']])
    return filelist
    
