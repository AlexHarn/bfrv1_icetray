

def parse_l2_filename(filename):
    """
    Parse a filename and returns a dictionary with keys:
    ('run', 'part', 'det', 'month', 'day', 'year').
    The date is in the path and not in the filename
    itself, so you need to pass an absolute path.
    """

    import re, glob, os
    data = {}
    basename = os.path.basename(filename)

    # run and subrun
    m = re.search('[R,r]un(?P<run>[0-9]+)_(Part|Subrun)(?P<part>[0-9]+)', basename)
    if m: data.update(m.groupdict())
    else: raise Exception(filename + ': Failed to guess run')

    # detector
    m = re.search('(?P<det>(?P<det_tag>IC[0-9]+)(.(?P<det_year>[0-9]+))?)', basename)
    if m: data.update(m.groupdict())
    else: raise Exception(filename + ': Failed to guess detector')

    # date
    month_day = re.findall('/[0-9]{4}/', filename)
    if len(month_day)==2:
        data['month'] = month_day[-1][1:3]
        data['day'] = month_day[-1][3:-1]
        data['year'] = month_day[0][1:-1]
    else: raise Exception(filename + ': Failed to guess date')

    return data
