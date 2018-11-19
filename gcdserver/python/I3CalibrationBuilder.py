
from I3Tray import I3Units
from icecube import dataclasses
from icecube.icetray import logging

import icecube.gcdserver.Geometry as G
import icecube.gcdserver.Calibration as C
import icecube.gcdserver.CalibrationDefaults as CalibrationDefaults
import icecube.gcdserver.util as util


class DuplicateDataError(Exception):
    pass


class InsufficientDataError(Exception):
    pass


def loadRecord(db, deviceName, type):
    """
    Load a single calibration record from the database
    @db: Access to set of valid GeometryObject and CalibrationObject instances
    @deviceName: Name of device we're building an I3DOMCalibration for
    @cls: Calibration data class desired
    """
    record = None
    for rec in db.calibrationDocuments(type, deviceName):
        if record is not None:
            raise DuplicateDataError(type)
        record = rec
    if record is None:
        raise InsufficientDataError(type)
    return record


def getDOMCalibration(db, omkey, device):
    """
    Build an I3DOMCalibration object for deployed device 'deviceName'
    using data in db
    @db: Access to set of valid GeometryObject and CalibrationObject instances
    @deviceName: Name of device we're building an I3DOMCalibration for
    @return: I3DOMCalibration instance.
    """
    cal = dataclasses.I3DOMCalibration()
    deviceName = device.name

    # Toroid droop parameters: first load defaults
    if device.data[G.Keys.TOROID_TYPE] == G.ToroidType.OLD:
        cal.tau_parameters = CalibrationDefaults.oldToroidTauParam
    else:
        cal.tau_parameters = CalibrationDefaults.newToroidTauParam
    try:
        rec = loadRecord(db, deviceName, C.ObjectType.TOROID_DROOP)
        tau = dataclasses.TauParam()
        tau.p0 = rec.data[C.Keys.TAU0]
        tau.p1 = rec.data[C.Keys.TAU1]
        tau.p2 = rec.data[C.Keys.TAU2]
        tau.p3 = rec.data[C.Keys.TAU3]
        tau.p4 = rec.data[C.Keys.TAU4]
        tau.p5 = rec.data[C.Keys.TAU5]
        tau.tau_frac = rec.data[C.Keys.TAU_FRACTION]
        cal.tau_parameters = tau
    except InsufficientDataError:
        # Not an error: Use defaults
        pass

    # Load PMT data
    cal.dom_noise_rate = 0.
    try:
        rec = loadRecord(db, deviceName, C.ObjectType.NOISE_RATE)
        cal.dom_noise_rate = rec.data[C.Keys.NOISE_RATE] * I3Units.hertz
    except InsufficientDataError:
        logging.log_warn("I3Calibration: Device %s: has OMKey %s-%s but no "
                         "noise rate" % (device.name, omkey.string, omkey.om))
    cal.relative_dom_eff = 1.
    try:
        # Increase relative efficiency for HQE DOMs
        if device.data[G.Keys.PMT_TYPE] == G.PMTType.R7081_100:
            cal.relative_dom_eff = CalibrationDefaults.R7081_100_Efficiency
    except KeyError:
        pass

    # Next, mandatory DOMCal data
    rec = loadRecord(db, deviceName, C.ObjectType.FRONT_END_IMPEDANCE)
    cal.front_end_impedance = rec.data[C.Keys.FE_IMPEDANCE] * I3Units.ohm

    rec = loadRecord(db, deviceName, C.ObjectType.ATWD_CAL)
    o = C.ATWDCalibration.wrapdict(rec.data)
    for atwd in range(C.ATWDConstants.N_ATWDS):
        for channel in range(C.ATWDConstants.N_CHANNELS):
            for bin in range(C.ATWDConstants.N_BINS):
                slope = o.getSlope(atwd, channel, bin) * I3Units.V
                cal.atwd_bin_calib_slope[(atwd, channel, bin)] = slope
    # NB: Take temperature and version from ATWD calibration
    cal.temperature = o[C.Keys.TEMPERATURE] * I3Units.kelvin
    cal.dom_cal_version = str(o[C.Keys.VERSION])

    rec = loadRecord(db, deviceName, C.ObjectType.ATWD_FREQ_CAL)
    o = C.ATWDFrequencyCal.wrapdict(rec.data)
    for atwd in range(C.ATWDConstants.N_ATWDS):
        data = o.getFit(atwd)
        fit = dataclasses.QuadraticFit()
        fit.quad_fit_a = data[0]
        fit.quad_fit_b = data[1]
        fit.quad_fit_c = data[2]
        cal.atwd_freq_fit[atwd] = fit

    rec = loadRecord(db, deviceName, C.ObjectType.AMP_CAL)
    o = C.AmplifierCal.wrapdict(rec.data)
    for channel in range(C.ATWDConstants.N_CHANNELS):
        cal.atwd_gain[channel] = o.getGain(channel)

    rec = loadRecord(db, deviceName, C.ObjectType.FADC_GAIN_CAL)
    cal.fadc_gain = rec.data[C.Keys.GAIN] * I3Units.V

    rec = loadRecord(db, deviceName, C.ObjectType.FADC_DELTA_T_CAL)
    cal.fadc_delta_t = rec.data[C.Keys.DELTA_T] * I3Units.ns

    rec = loadRecord(db, deviceName, C.ObjectType.ATWD_DELTA_T_CAL)
    o = C.ATWDDeltaTCal.wrapdict(rec.data)
    for atwd in range(C.ATWDConstants.N_ATWDS):
        cal.atwd_delta_t[atwd] = o.getDeltaT(atwd) * I3Units.ns

    rec = loadRecord(db, deviceName, C.ObjectType.FADC_BASELINE)
    cal.fadc_baseline_fit = util.getDOMCalFit(rec)

    rec = loadRecord(db, deviceName, C.ObjectType.SPE_DISC_CAL)
    cal.spe_disc_calib = util.getDOMCalFit(rec)

    rec = loadRecord(db, deviceName, C.ObjectType.MPE_DISC_CAL)
    cal.mpe_disc_calib = util.getDOMCalFit(rec)

    # Load the baseline if available
    try:
        rec = loadRecord(db, deviceName, C.ObjectType.BEACON_BASELINES)
        o = C.BeaconBaseline.wrapdict(rec.data)
        for atwd in range(C.ATWDConstants.N_ATWDS):
            for channel in range(C.ATWDConstants.N_CHANNELS):
                baseline = o.getATWDBaseline(atwd, channel)
                cal.atwd_beacon_baseline[(atwd, channel)] = baseline
        cal.fadc_beacon_baseline = o[C.Keys.FADC_BASELINE]
    except InsufficientDataError:
        logging.log_warn("I3Calibration: Device %s: has OMKey %s-%s but no"
                         " baseline" % (device.name, omkey.string, omkey.om))

    # Optional parts of the DOMCal calibration
    try:
        rec = loadRecord(db, deviceName, C.ObjectType.PMT_TRANSIT_TIME_CAL)
        cal.transit_time = util.getDOMCalFit(rec)
    except InsufficientDataError:
        logging.log_warn("I3Calibration: Device %s: has OMKey %s-%s but "
                         "no PMT transit time calibration" %
                         (device.name, omkey.string, omkey.om))

    try:
        rec = loadRecord(db, deviceName, C.ObjectType.GAIN_CAL)
        cal.hv_gain_fit = util.getDOMCalFit(rec)
    except InsufficientDataError:
        logging.log_warn("I3Calibration: Device %s: has OMKey %s-%s but "
                         "no gain calibration" %
                         (device.name, omkey.string, omkey.om))

    try:
        rec = loadRecord(db, deviceName, C.ObjectType.PMT_DISC_CAL)
        cal.pmt_disc_calib = util.getDOMCalFit(rec)
        # NB: This behavior is copied from I3DB with no justification
        if rec.data[C.Keys.SLOPE] == 0 or rec.data[C.Keys.INTERCEPT] == 0:
            fit = dataclasses.LinearFit()
            fit.slope = float('NaN')
            fit.intercept = float('NaN')
            cal.pmt_disc_calib = fit
    except InsufficientDataError:
        logging.log_warn("I3Calibration: Device %s: has OMKey %s-%s but "
                         "no PMT discriminator calibration" %
                         (device.name, omkey.string, omkey.om))

    # Load the SPE corrections if we have them. No IceTop corrections,
    # so skip the warnings here
    cal.mean_atwd_charge = float('NaN')
    try:
        rec = loadRecord(db, deviceName, C.ObjectType.ATWD_SPE_CORR)
        cal.mean_atwd_charge = rec.data[C.Keys.GAUSS_MEAN]
    except InsufficientDataError:
        pass

    cal.mean_fadc_charge = float('NaN')
    try:
        rec = loadRecord(db, deviceName, C.ObjectType.FADC_SPE_CORR)
        cal.mean_fadc_charge = rec.data[C.Keys.GAUSS_MEAN]
    except InsufficientDataError:
        pass

    try:
        rec = loadRecord(db, deviceName, C.ObjectType.JOINT_SPE_CORR)
        spe_charge_dist = dataclasses.SPEChargeDistribution()
        spe_charge_dist.exp1_amp = rec.data[C.Keys.EXP_NORM]
        spe_charge_dist.exp1_width = rec.data[C.Keys.EXP_SCALE]
        spe_charge_dist.gaus_amp = rec.data[C.Keys.GAUSS_NORM]
        spe_charge_dist.gaus_mean = rec.data[C.Keys.GAUSS_MEAN]
        spe_charge_dist.gaus_width = rec.data[C.Keys.GAUSS_STDDEV]
        try:
            # Use data from Spencer's SPE fits if we have it
            spe_charge_dist.exp2_amp = rec.data[C.Keys.EXP2_NORM]
            spe_charge_dist.exp2_width = rec.data[C.Keys.EXP2_SCALE]
            spe_charge_dist.compensation_factor = rec.data[
                                                    C.Keys.SPE_COMP_FACTOR]
            # Need to go back to the FADC data for the SLC Gaussian Mean
            # This is not ideal.
            rec = loadRecord(db, deviceName, C.ObjectType.FADC_SPE_CORR)
            spe_charge_dist.slc_gaus_mean = rec.data[C.Keys.GAUSS_MEAN]
            
        except KeyError:
            pass
        cal.combined_spe_charge_distribution = spe_charge_dist
    except InsufficientDataError:
        pass

    return cal


def buildI3Calibration(db):
    """
    Build an I3Calibration instance from GCD data.
    @db: Access to set of valid GeometryObject and CalibrationObject instances
    @return: dataclasses.I3Calibration instance.
    """
    cal = dataclasses.I3Calibration()

    # Discussion with Alex O.: the start/end times are only used by I3Muxer,
    # and it should be easy to switch to run-based validity.  Since this
    # feature is unused, set start/end times such that data is always valid.
    cal.start_time.set_daq_time(0, 0)
    cal.end_time.set_daq_time(9999, 0)

    # Build the VEMCal map
    for vemcal in db.calibrationDocuments(C.ObjectType.VEMCAL):
        try:
            vc = dataclasses.I3VEMCalibration()
            vc.pe_per_vem = vemcal.data[C.Keys.PE_PER_VEM]
            vc.mu_peak_width = vemcal.data[C.Keys.MUON_PEAK_WIDTH]
            vc.hglg_cross_over = vemcal.data[C.Keys.HG_LG_CROSSOVER]
            vc.corr_factor = vemcal.data[C.Keys.CORR_FACTOR]
            cal.vem_cal[util.getOMKey(db.deployedNameMap(vemcal.name))] = vc
        except KeyError:
            logging.log_warn("I3Calibration: Ignoring VEMCal for unknown "
                             "device: %s" % vemcal.name)

    # For each deployed device, attempt to fill an I3DOMCalibration object
    for (key, device) in db.deployedDevices():
        omkey = util.getOMKey(device)
        try:
            cal.dom_cal[omkey] = getDOMCalibration(db, omkey, device)
        except DuplicateDataError as e:
            logging.log_warn("I3Calibration: Skipping device with "
                             "duplicate calibration "
                             "data: %s: Duplicate: %s" % (device.name, str(e)))
        except InsufficientDataError as e:
            logging.log_warn("I3Calibration: Skipping device %s: has OMKey "
                             "%s-%s but insufficient calibration data: "
                             "Missing: %s" % (device.name, omkey.string,
                                              omkey.om, str(e)))
    return cal
