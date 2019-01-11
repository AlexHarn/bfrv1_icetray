#!/usr/bin/env python

"""
Compares two GCD files. The result is printed and returned as exit code:

* Exit code 0: The files are the same
* Exit code 1: A technical problem occurred
* Exit code 2: The files are not the same. For more details check stdout
"""

import math
import contextlib
import argparse

from icecube import icetray, dataio, dataclasses
import icecube.gcdserver.Calibration as C

 
fileNames = [None, None]
attrstack = []
ignore_start_end_times = False


@contextlib.contextmanager
def appendAttr(attr):
    attrstack.append(attr)
    try:
        yield
    finally:
        attrstack.pop()


def openI3File(fileName):
    try:
        return dataio.I3File(fileName, 'r')
    except:
        print "Unable to open file: %s" % fileName
        exit(1)


def getDetectorStatusFrame(fileName):
    f = openI3File(fileName)
    while True:
        fr = f.pop_frame()
        if fr is None:
            print "File %s: Missing D Frame" % fileName
            exit(1)
        if fr.Stop == icetray.I3Frame.DetectorStatus:
            return fr


def equalityCompare(obj1, obj2):
    return obj1 == obj2


def floatingPointCompare(obj1, obj2):
    if obj1 == 0.:
        return obj2 == 0.
    if math.isnan(obj1):
        return math.isnan(obj2)
    return abs(obj2 - obj1) / obj1 < 1e-6


def typeCompare(obj1, obj2):
    if type(obj1) == type(obj2):
        if isinstance(obj1, float):
            return floatingPointCompare(obj1, obj2)
        if isinstance(obj1, int):
            return equalityCompare(obj1, obj2)
        if isinstance(obj1, str):
            return equalityCompare(obj1, obj2)
        if isinstance(obj1, dataclasses.I3VectorOMKey):
            return sorted(obj1) == sorted(obj2)
    raise Exception("Cannot compare objects: %s, %s" % 
                                     (type(obj1), type(obj2)))


def printAttrStack():
    str = ""
    for s in attrstack[:-1]:
        str += "%s." % s
    if len(attrstack) > 0:
        str += "%s:" % attrstack[-1]
    print "  in '%s'" % str


def printAttrMismatch(values):
    printAttrStack()
    print ("ERROR: Attribute mismatch:\nfile %s: %s\nfile %s: %s" %
                       (fileNames[0], values[0], fileNames[1], values[1]))


def checkValues(values, compare=typeCompare):
    if not compare(values[0], values[1]):
        printAttrMismatch(values)
        return False

    return True


def checkAttr(objs, attr, compare=typeCompare):
    same = True

    if type(attr) is dict:
        key = attr.keys()[0]
        with appendAttr(key):
            newObjs = [x.__getattribute__(key) for x in objs]
            for newAttr in attr[key]:
                if not checkAttr(newObjs, newAttr):
                    same = False
    else:
        with appendAttr(attr):
            if not (ignore_start_end_times and (attr == 'start_time' or attr == 'end_time')):
                if not checkValues([x.__getattribute__(attr) for x in objs], compare):
                    same = False

    return same


def printExtraKeys(fileName, keySet):
    if len(keySet) > 0:
        print "ERROR: Extra keys in file %s: %s" % (fileName, list(keySet))


def checkAndIntersectKeys(maps, error):
    keys = [set(x.keys()) for x in maps]
    if keys[0] != keys[1]:
        error[0] = True
        printAttrStack()
        print "ERROR: Keys differ.  Summary:"
        printExtraKeys(fileNames[0], keys[0] - keys[1])
        printExtraKeys(fileNames[1], keys[1] - keys[0])
    return keys[0] & keys[1]


def checkAndIntersectKeysDirect(keys, error):
        missing1 = [k for k in keys[0] if k not in keys[1]]
        missing2 = [k for k in keys[1] if k not in keys[0]]
        if len(missing1) > 0 or len(missing2) > 0:
            error[0] = True
            printAttrStack()
            print "ERROR: Keys differ.  Summary:"
            printExtraKeys(fileNames[0], missing1)
            printExtraKeys(fileNames[1], missing2)
        # Take the key intersection and run from there
        for key in [k for k in keys[0] if k in keys[1]]:
            yield key


def checkKeysAndAttrs(objs, mapAttr, attrList):
    same = True

    with appendAttr(mapAttr):
        error = [False]

        maps = [x.__getattribute__(mapAttr) for x in objs]
        for key in checkAndIntersectKeys(maps, error):
            with appendAttr(key):
                values = [x[key] for x in maps]
                for attr in attrList:
                    if not checkAttr(values, attr):
                        same = False

        if error[0]:
            same = False

    return same


def compareGeometry(objs):
    same_frame = True

    if not checkAttr(objs, 'start_time', compare=equalityCompare):
        same_frame = False

    if not checkAttr(objs, 'end_time', compare=equalityCompare):
        same_frame = False

    omgeoAttrs = [
                    'area',
                    {'direction': ['zenith', 'azimuth']},
                    'omtype',
                    {'orientation': ['x', 'y', 'z', 'up_x', 'up_y', 'up_z']},
                    {'position': ['x', 'y', 'z']}
                 ]

    if not checkKeysAndAttrs(objs, 'omgeo', omgeoAttrs):
        same_frame = False

    # Stationgeo is a map of station number to a list of (two) tanks
    with appendAttr('stationgeo'):
        # Needs to be a list in order to have a 'pass by reference' behavior
        error = [False]

        stationgeoattrs = [
                             'fillheight',
                             'omkey_list',
                             'orientation',
                             {'position': ['x', 'y', 'z']},
                             'snowheight',
                             'tankheight',
                             'tankradius',
                             'tanktype'
                          ]
        stationgeo = [x.__getattribute__('stationgeo') for x in objs]
        for key in checkAndIntersectKeys(stationgeo, error):
            with appendAttr(key):
                # Convert list to a map for easy comparison.  Assume
                # omkey_list is not empty and use the first omkey as a key
                tankMapList = [dict((sorted(y.omkey_list)[0], y)
                                        for y in x[key]) for x in stationgeo]
                for omkey in checkAndIntersectKeys(tankMapList, error):
                    with appendAttr(omkey):
                        values = [x[omkey] for x in tankMapList]
                        for attr in stationgeoattrs:
                            if not checkAttr(values, attr):
                                same_frame = False

        if error[0]:
            same_frame = False

    return same_frame

            
def compareCalibration(objs):
    same_frame = True

    if not checkAttr(objs, 'start_time', compare=equalityCompare):
        same_frame = False

    if not checkAttr(objs, 'end_time', compare=equalityCompare):
        same_frame = False

    vemCalAttrs = ['corr_factor', 'hglg_cross_over',
                   'mu_peak_width', 'pe_per_vem']

    if not checkKeysAndAttrs(objs, 'vem_cal', vemCalAttrs):
        same_frame = False

    domCalBasicAttrs = [
                          {'combined_spe_charge_distribution': ['exp_amp',
                                                                'exp_width',
                                                                'gaus_amp',
                                                                'gaus_mean',
                                                                'gaus_width',
                                                                'is_valid']},
                          'dom_cal_version',
                          'dom_noise_decay_rate',
                          'dom_noise_rate',
                          'dom_noise_scintillation_hits',
                          'dom_noise_scintillation_mean',
                          'dom_noise_scintillation_sigma',
                          'dom_noise_thermal_rate',
                          {'fadc_baseline_fit': ['intercept', 'slope']},
                          'fadc_beacon_baseline',
                          'fadc_delta_t',
                          'fadc_gain',
                          'front_end_impedance',
                          {'hv_gain_fit': ['intercept', 'slope']},
                          'is_mean_atwd_charge_valid',
                          'is_mean_fadc_charge_valid',
                          'mean_atwd_charge',
                          'mean_fadc_charge',
                          {'mpe_disc_calib': ['intercept', 'slope']},
                          {'pmt_disc_calib': ['intercept', 'slope']},
                          'relative_dom_eff',
                          {'spe_disc_calib': ['intercept', 'slope']},
                          {'tau_parameters':
                             ['p0', 'p1', 'p2', 'p3', 'p4', 'p5', 'tau_frac']},
                          'temperature',
                          'toroid_type',
                          {'transit_time': ['intercept', 'slope']}
                       ]

    if not checkKeysAndAttrs(objs, 'dom_cal', domCalBasicAttrs):
        same_frame = False

    with appendAttr('dom_cal'):
        # We've already checked the keys.  Just intersect them this time.
        domcals = [x.__getattribute__('dom_cal') for x in objs]
        keys = [set(x.keys()) for x in domcals]
        for key in keys[0] & keys[1]:
            freqFit = [x[key].atwd_freq_fit for x in domcals]
            baseline = [x[key].atwd_beacon_baseline for x in domcals]
            atwdCal = [x[key].atwd_bin_calib_slope for x in domcals]
            atwdDeltaT = [x[key].atwd_delta_t for x in domcals]
            atwdGain = [x[key].atwd_gain for x in domcals]
            for atwd in range(C.ATWDConstants.N_ATWDS):
                # ATWD frequency fit
                with appendAttr('%s.atwd_freq_fit.%s' % (key, str(atwd))):
                    attrs = ['quad_fit_a', 'quad_fit_b', 'quad_fit_c']
                    for attr in attrs:
                        if not checkAttr([x[atwd] for x in freqFit], attr):
                            same_frame = False
                # ATWD delta-T
                with appendAttr('%s.atwd_delta_t.%s' % (key, str(atwd))):
                    checkValues([x[atwd] for x in atwdDeltaT])
            for atwd in range(C.ATWDConstants.N_ATWDS):
                for ch in range(C.ATWDConstants.N_CHANNELS):
                    # ATWD baseline calibration
                    with appendAttr(
                          '%s.atwd_beacon_baseline.%s.%s' % (key, atwd, ch)):
                        if not checkValues([x[(atwd, ch)] for x in baseline]):
                            same_frame = False
                    # ATWD calibration
                    for bin in range(C.ATWDConstants.N_BINS):
                        with appendAttr('%s.atwd_bin_calib_slope.%s.%s.%s' %
                                                       (key, atwd, ch, bin)):
                            if not checkValues([x[(atwd, ch, bin)] for x in atwdCal]):
                                same_frame = False
            for ch in range(C.ATWDConstants.N_CHANNELS):
                # ATWD gain calibration
                with appendAttr('%s.atwd_gain.%s' % (key, ch)):
                    if not checkValues([x[ch] for x in atwdGain]):
                        same_frame = False
    return same_frame


def compareStatus(objs):
    same_frame = True

    if not checkAttr(objs, 'start_time', compare=equalityCompare):
        same_frame = False

    if not checkAttr(objs, 'end_time', compare=equalityCompare):
        same_frame = False

    if not checkAttr(objs, 'daq_configuration_name'):
        same_frame = False

    domStatusAttrs = (
        ['cable_type', 'dac_fadc_ref', 'dac_trigger_bias_0',
         'dac_trigger_bias_1', 'delta_compress', 'dom_gain_type',
         'fe_pedestal', 'lc_mode', 'lc_span', 'lc_window_post',
         'lc_window_pre', 'mpe_threshold', 'n_bins_atwd_0', 'n_bins_atwd_1',
         'n_bins_atwd_2', 'n_bins_atwd_3', 'n_bins_fadc', 'pmt_hv',
         'slc_active', 'spe_threshold', 'status_atwd_a', 'status_atwd_b',
         'status_fadc', 'trig_mode', 'tx_mode'])

    if not checkKeysAndAttrs(objs, 'dom_status', domStatusAttrs):
        same_frame = False

    # Check trigger status.  I3TriggerKey is not hashable, so we
    # must check it manually
    with appendAttr('trigger_status'):
        # Needs to be a list in order to have a 'pass by reference' behavior
        error = [False]

        keys = [x.trigger_status.keys() for x in objs]
        # Must check for missing keys using equality
        for key in checkAndIntersectKeysDirect(keys, error):
            with appendAttr(key):
                values = [x.trigger_status[key] for x in objs]
                checkAttr(values, 'trigger_name')
                readoutKeys = [x.readout_settings.keys() for x in values]
                with appendAttr('readout_settings'):
                    for key in checkAndIntersectKeysDirect(readoutKeys, error):
                        with appendAttr(str(key)):
                            rd = [x.readout_settings[key] for x in values]
                            for attr in ['readout_time_minus',
                                         'readout_time_offset',
                                         'readout_time_plus']:
                                if not checkAttr(rd, attr):
                                    same_frame = False

                triggerKeys = [x.trigger_settings.keys() for x in values]
                with appendAttr('trigger_settings'):
                    for key in checkAndIntersectKeysDirect(triggerKeys, error):
                        with appendAttr(str(key)):
                            ts = [x.trigger_settings[key] for x in values]
                            # All the settings are simple strings
                            if not checkValues(ts):
                                same_frame = False

        if error[0]:
            same_frame = False

    return same_frame


def compareGCD():
    same_files = True

    # Get the DetectorStatus frames.  These frames should have G/C mixed in
    ds = [getDetectorStatusFrame(x) for x in fileNames]
    sections = (
        [('I3Geometry', compareGeometry),
         ('I3Calibration', compareCalibration),
         ('I3DetectorStatus', compareStatus)])
    for (key, fxn) in sections:
        try:
            print "\nChecking %s" % key
            if not fxn([ds[0][key], ds[1][key]]):
                same_files = False
        except KeyError:
            same_files = False

            for i in range(2):
                if key not in ds[i]:
                    print "File %s: Missing key %s" % (fileNames[i], key)

    return same_files

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description = __doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument('gcdfile1', type = str, help = 'First GCD file to compare')
    parser.add_argument('gcdfile2', type = str, help = 'Second GCD file to compare')
    parser.add_argument('--ignore-start-end-times', action = 'store_true', default = False, help = "Ignore start/end time mismatches. In fact, mismatches of the keys 'start_time' and 'end_time' will be ignored.")
    args = parser.parse_args()

    fileNames = [args.gcdfile1, args.gcdfile2]
    ignore_start_end_times = args.ignore_start_end_times

    if not compareGCD():
        exit(2)

