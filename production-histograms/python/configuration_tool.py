import os
import sys
import glob
from icecube import icetray, dataio, dataclasses, simclasses, recclasses

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
    good_filelist = list()
    corrupt_filelist = list()
    try:
        i3file = dataio.I3File(filename)
        for frame in i3file:
            for frame_key in frame.keys():
                if frame_key not in histograms:
                    hlist = _configure_from_frame(frame, frame_key)
                    if hlist:
                        histograms[frame_key] = hlist
        good_filelist.append(filename)
    except RuntimeError:
        corrupt_filelist.append(filename)
        
    finally:
        print("configure: %d" % len(histograms))
        return good_filelist, corrupt_filelist

def generate_histogram_configuration_list(i3files):
    '''
    Given either a single path (string) or a list of paths, 
    peek at the I3Files, loop over all frames, and figure 
    out which histograms need to be loaded.
    '''
    histograms = dict()
    good_filelist = list()
    corrupt_filelist = list()
    
    filelist = [i3files] if isinstance(i3files, str) else i3files
    
    file_counter = 0
    for filename in filelist:
        file_counter += 1
        print("Processing file (%d/%d): %s " % (file_counter, len(filelist), filename))
        good_filelist, corrupt_filelist = _configure(filename, histograms)

    histogram_list = list()
    for frame_key, hl in histograms.iteritems():
        histogram_list.extend(hl) 
    return histogram_list, good_filelist, corrupt_filelist

def generate_i3filelist(path):
    '''
    Get all I3Files, excluding GCD files.
    '''
    filelist = list()
    for root, dirs, files in os.walk(path):
        for fn in files:
            
            if 'GeoCalibDetectorStatus' in fn or \
                'GCD' in fn:
                continue

            if fn.endswith('.i3') or \
               fn.endswith('.i3.bz2') or \
               fn.endswith('.i3.gz') or \
               fn.endswith('.i3.zst'):
                
                filelist.append(os.path.join(root, fn))
    return filelist
    

