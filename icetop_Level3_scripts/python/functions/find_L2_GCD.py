import glob, re, os
try:
    from icecube import icetray
    with_icetray = True
except:
    with_icetray = False

def find_L2_GCD_from_date(runnumber, day, month, year, pass2a=False):
    """
    Locate the Level 2 GCD for data, based on the run number and date.
    """
    gcd_glob = ['/data/exp/IceCube/{year}/filtered/level2a/{month:02d}{day:02d}/Level2a_*_data_Run{run:08d}_*GCD.i3.bz2'.\
                    format(year=year, run=runnumber, month=month, day=day),
                '/data/exp/IceCube/{year}/filtered/level2/{month:02d}{day:02d}/Level2_*_data_Run{run:08d}_*GCD.i3.gz'.\
                    format(year=year, run=runnumber, month=month, day=day),
                '/data/exp/IceCube/{year}/filtered/level2/{month:02d}{day:02d}/Run{run:08d}/Level2_*_data_Run{run:08d}_*GCD.i3.gz'.\
                    format(year=year, run=runnumber, month=month, day=day),
                ## IC86.2017 requires these:
                '/data/exp/IceCube/{year}/filtered/level2/{month:02d}{day:02d}/Run{run:08d}/Level2_*_data_Run{run:08d}_*GCD.i3.zst'.\
                    format(year=year, run=runnumber, month=month, day=day)]
    if (pass2a):  # Look in a different list of places
        gcd_glob = ['/data/exp/IceCube/{year}/filtered/level2pass2a/{month:02d}{day:02d}/Run{run:08d}/Level2pass2_*_data_Run{run:08d}_*GCD.i3.zst'.\
                        format(year=year, run=runnumber, month=month, day=day),
                    '/data/exp/IceCube/{year}/filtered/level2pass2a/{month:02d}{day:02d}/Run{run:08d}/Level2pass2_*_data_Run{run:08d}_*GCD.i3.gz'.\
                        format(year=year, run=runnumber, month=month, day=day)]
 

    gcd = sum([glob.glob(g) for g in gcd_glob], [])
    gcd = [g for g in gcd if os.path.exists(g)]
    if len(gcd)==0:
        if with_icetray:
            icetray.logging.log_fatal('No suitable GCD file found (looking for {0})'.format(' '.join(gcd_glob)))
        else:
            print( 'No suitable GCD file found (looking for {0})'.format(' '.join([gcd_glob,gcd_glob_L2a])))
            return None
    verified_gcd = [g for g in gcd if re.search('VerifiedGCD', g)]
    # take the latest verified GCD, whatever that means...
    if verified_gcd: gcd = sorted(verified_gcd, key=lambda p: -os.path.getctime(p))
    return gcd[0]

def find_L2_GCD_from_filename(filename, pass2a=False):
    """
    Locate the Level 2 GCD for data, based on a level 2 filename.
    """
    import parse_l2_filename as parse
    data = parse.parse_l2_filename(filename)
    return find_L2_GCD_from_date(int(data['run']), int(data['day']), int(data['month']), int(data['year']), pass2a)



