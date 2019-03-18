import os
    
def find_gcd_file(path):
    '''
    Give a path to I3Files, this will return the name of the
    first GCD file found.

    Currently it does a quick search for I3Files that start 
    with GeoCalib (which should be the most common) or 
    GC (I've seen a few GCS files).

    If it can't find any with a standard naming convention
    above it will loop through every frame in every I3File
    in the path until it finds a frame with a Geometry,
    Calibration, and DetectorStatus frame.
    '''
    from icecube import dataio
    for fn in os.listdir(path):        
        if '.i3' in fn and\
            (fn.startswith('GeoCalib') or\
             fn.startswith('GC')):
            return os.path.join(path, fn)

    for fn in os.listdir(path):
        if '.i3' in fn:
            filename = os.path.join(path, fn)
            i3file = dataio.I3File(filename)
            for frame in i3file:
                if 'I3Geometry' in frame and\
                   'I3Calibration' in frame and\
                   'I3DetectorStatus' in frame:
                    return os.path.join(filename)

