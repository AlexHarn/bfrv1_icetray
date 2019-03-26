import os
import sys
import glob
from icecube import icetray, dataio, dataclasses, simclasses, recclasses

from icecube.production_histograms.find_gcd_file import find_gcd_file
from icecube.production_histograms.db import create_simprod_db_client
from icecube.production_histograms.generate_collection_name import generate_collection_name


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
    histogram_collection_name = generate_collection_name(filename)
    icetray.logging.log_debug("    %s" % histogram_collection_name)
    
    client = create_simprod_db_client()
    collection = client.simprod_filecatalog.filelist
    document = collection.find_one({'category': histogram_collection_name})

    if not document:
        collection.insert_one({'filelist': list(),
                               'corrupt_filelist': [filename]})
    else:    
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
    Given either a single path (string) or a list of paths, 
    peek at the I3Files, loop over all frames, and figure 
    out which histograms need to be loaded.
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

def _generate_i3filelist(path, exclude_GCD = True):
    '''
    Get all I3Files with the option to exclude GCD files.
    '''
    filelist = list()
    for root, dirs, files in os.walk(path):
        for fn in files:
            
            if exclude_GCD and \
               ('GeoCalibDetectorStatus' in fn or \
                'GCD' in fn):
                continue

            if fn.endswith('.i3') or \
               fn.endswith('.i3.bz2') or \
               fn.endswith('.i3.gz') or \
               fn.endswith('.i3.zst'):
                
                result.append(os.path.join(root, fn))
    return filelist
    
def update_filecatalog(path):

    client = create_simprod_db_client()
    collection = client.simprod_filecatalog.filelist
    histogram_collection_name = generate_collection_name(filename)
    document = collection.find_one({'category': histogram_collection_name})

    if not document:
        collection.insert_one({'filelist': filelist})
    else:
        old_filelist = document['filelist']
        document['filelist'] = list(set(old_filelist + filelist)) # don't want duplicates
        result = collection.update({'_id': document['_id']}, document)
        print(result)

    
def generate_filelist(path):
    '''
    This function first finds the appropriate GCD file for this dataset, which
    becomes the first entry in the list.

    For the filelist, it'll look in the filecatalog first.  If there's no entry
    '''

    gcdfile = find_gcd_file(path)
    if not gcdfile:
    	config_year = path.split("/")
    	gcddir = '/data/sim/sim-new/downloads/GCD/'
    	keyword = config_year[4]
    	for filename in glob.glob(os.path.join(gcddir, '*.i3.gz')):
	    if keyword in filename:
		if len(filename.split("_")) < 4:
		    gcdfile = filename
    if not gcdfile:
        print("Could not find GCD file in %s" % path)
        sys.exit(1)
        
    filelist = [gcdfile]
    filelist.extend(_generate_i3filelist(path))
    return filelist

    
