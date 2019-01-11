

def _is_range(s):
    '''
    This checks whether s is of the form X-Y
    where X and Y are type int.
    '''
    if s.count('-') != 1:
        return False

    x = s.split('-')[0]
    y = s.split('-')[1]

    try:
        int(x)
        int(y)
    except:
        return False
    return True

def categorize(path):
    '''        
    Given a path with /path/to/files/X-Y/subrun/filename.i3.gz
    strip out the /X-Y/ and the trailing filename and return
    path:to:files:subrun

    Each of these should correspond to, what I'm calling,
    a dataset.
    '''
    rval = ""
    for d in path.split('/')[3:]:
        if not _is_range(d):
            rval += d + ":"
    return rval.rstrip(":")



