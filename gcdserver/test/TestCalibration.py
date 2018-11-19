#!/usr/bin/env python

from contexts import test_db_context, file_context
from TestData import (BASELINE_XML, VEMCAL_XML, NOISE_RATE_JSON,
                      DOMCAL_XML_1_61, DOMCAL_XML_1_62,
                      GEOMETRY_1_61, GEOMETRY_1_62, DOM_DROOP_1_61,
                      SPE_FIT_JSON, SPENCER_FIT_JSON)
import icecube.gcdserver.DOMCalImport as DOMCalImport
import icecube.gcdserver.VEMCalImport as VEMCalImport
import icecube.gcdserver.SPEFitImport as SPEFitImport
import icecube.gcdserver.NoiseRateImport as NoiseRateImport
import icecube.gcdserver.BaselineImport as BaselineImport
from icecube.gcdserver.I3MS import geoDBInserter, calDBInserter
from icecube.gcdserver.MongoDB import fillBlobDB
from icecube.gcdserver.I3CalibrationBuilder import buildI3Calibration
import icecube.gcdserver.Geometry as G
import icecube.gcdserver.Calibration as C
import icecube.gcdserver.CalibrationDefaults as CalibrationDefaults

from icecube.icetray import OMKey, I3Units
from icecube import dataclasses


def compareVEMCal(key, val):
    if key == OMKey(1, 61):
        assert val.corr_factor == 1
        assert val.hglg_cross_over == 2361.66
        assert val.mu_peak_width == 20.3121
        assert val.pe_per_vem == 116.274
    if key == OMKey(1, 62):
        assert val.corr_factor == 1
        assert val.hglg_cross_over == -1
        assert val.mu_peak_width == 20.3121
        assert val.pe_per_vem == 134.187


TAU_PARAM_1_61 = dataclasses.TauParam()
TAU_PARAM_1_61.p0 = 222.97
TAU_PARAM_1_61.p1 = 4267.6001
TAU_PARAM_1_61.p2 = 21.552
TAU_PARAM_1_61.p3 = 167.227
TAU_PARAM_1_61.p4 = 3200.7
TAU_PARAM_1_61.p5 = 21.552
TAU_PARAM_1_61.tau_frac = -3.3


def getFit(slope, intercept):
    """
    Return a LinearFit object from a DOMCal linear fit
    """
    fit = dataclasses.LinearFit()
    fit.slope = slope
    fit.intercept = intercept
    return fit


def compareDOMCal(key, val):
    if key == OMKey(1, 61):
        assert val.atwd_beacon_baseline[(1, 1)] == 134.035253
        assert val.fadc_beacon_baseline == 137.185200
        assert val.dom_noise_rate == 1386.3499999999999 * I3Units.hertz
        assert val.relative_dom_eff == 1.
        assert val.tau_parameters == TAU_PARAM_1_61
        assert val.mean_atwd_charge == 1.0341419190237735
        assert val.mean_fadc_charge == 1.1160816676546705
        spe_charge_dist = val.combined_spe_charge_distribution
        assert spe_charge_dist.exp1_amp == 5630.9241121652794
        assert spe_charge_dist.exp1_width == 0.25054968276866763
        assert spe_charge_dist.gaus_amp == 6497.3366978454396
        assert spe_charge_dist.gaus_mean == 1.0325212232516821
        assert spe_charge_dist.gaus_width == 0.25579619731120745
    elif key == OMKey(1, 62):
        assert val.atwd_beacon_baseline[(1, 1)] == 136.098590
        assert val.fadc_beacon_baseline == 126.028301
        assert val.dom_noise_rate == 54.650000000000006 * I3Units.hertz
        assert val.relative_dom_eff == CalibrationDefaults.R7081_100_Efficiency
        assert val.tau_parameters == CalibrationDefaults.newToroidTauParam
        assert val.mean_atwd_charge == 1.0544911706904694
        assert val.mean_fadc_charge == 1.0715882684510842
        spe_charge_dist = val.combined_spe_charge_distribution
        assert spe_charge_dist.exp1_amp == 6728.183586839823
        assert spe_charge_dist.exp1_width == 0.19590329312561494
        assert spe_charge_dist.gaus_amp == 8260.8984581176519
        assert spe_charge_dist.gaus_mean == 1.0543760019040789
        assert spe_charge_dist.gaus_width == 0.24293675094300488

    # N.B. I3DOMCalibration::SetATWDBinCalibSlope reverses bin order
    assert val.atwd_bin_calib_slope[(0, 2, 127 - 5)] == -0.00195929 * I3Units.V
    assert val.atwd_delta_t[0] == 0. * I3Units.ns
    assert val.atwd_delta_t[1] == -0.602172 * I3Units.ns

    fit = dataclasses.QuadraticFit()
    fit.quad_fit_a = 36.3884
    fit.quad_fit_b = 0.349686
    fit.quad_fit_c = -3.01701e-05
    assert val.atwd_freq_fit[0] == fit

    fit.quad_fit_a = 37.9335
    fit.quad_fit_b = 0.35192
    fit.quad_fit_c = -2.98137e-05
    assert val.atwd_freq_fit[1] == fit

    assert val.atwd_gain[0] == -15.8162
    assert val.atwd_gain[1] == -1.80474
    assert val.atwd_gain[2] == -0.210016
    assert val.dom_cal_version == "7.6.0"
    assert val.fadc_baseline_fit == getFit(1.26364, -886.734)
    assert val.fadc_delta_t == -113.479 * I3Units.ns
    assert val.fadc_gain == 8.79808e-05 * I3Units.V
    assert val.front_end_impedance == 50.0 * I3Units.ohm
    assert val.hv_gain_fit == getFit(7.54566, -16.4696)
    assert val.is_mean_atwd_charge_valid == True
    assert val.is_mean_fadc_charge_valid == True
    assert val.mpe_disc_calib == getFit(0.126135, -67.6154)
    assert val.pmt_disc_calib == getFit(0.0155336, -8.34602)
    assert val.spe_disc_calib == getFit(0.012365, -6.69295)
    assert val.temperature == 244.5 * I3Units.kelvin
    assert val.toroid_type == dataclasses.I3DOMCalibration.NEW_TOROID
    assert val.transit_time == getFit(1978.53, 88.4719)


def fillMissingATWDData(atwdCal):
    o = C.ATWDCalibration.wrapdict(atwdCal.data)
    for atwd in range(C.ATWDConstants.N_ATWDS):
        for channel in range(C.ATWDConstants.N_CHANNELS):
            for bin in range(C.ATWDConstants.N_BINS):
                if o.getSlope(atwd , channel, bin) is None:
                    o.setSlope(atwd , channel, bin, 0.)


RUN_VALID = 1


def getI3Calibration(db):
    # Get the BlobDB instance
    blobDB = fillBlobDB(db, run=RUN_VALID)
    # We cut ATWD calibration data to keep TestData small.  Fill it in.
    for atwdCal in blobDB.calibrationDocuments(C.ObjectType.ATWD_CAL):
        fillMissingATWDData(atwdCal)
        
    # Build I3Calibration
    return buildI3Calibration(blobDB)


def test_calibration():
    with test_db_context() as db:
        # Import geometry data for string 1, DOMs 61 and 62
        with geoDBInserter(db) as geoInserter:
            geoInserter.insert(G.GeometryObject.wrapdict(GEOMETRY_1_61))
            geoInserter.insert(G.GeometryObject.wrapdict(GEOMETRY_1_62))
            geoInserter.commit()
        # Import calibration data
        with file_context(BASELINE_XML) as inputFile:
            BaselineImport.doInsert(db, RUN_VALID, None, [inputFile])
        with file_context(VEMCAL_XML) as inputFile:
            VEMCalImport.doInsert(db, RUN_VALID, None, [inputFile])
        with file_context(NOISE_RATE_JSON) as inputFile:
            NoiseRateImport.doInsert(db, RUN_VALID, None, [inputFile])
        with file_context(DOMCAL_XML_1_61) as inputFile:
            DOMCalImport.doInsert(db, RUN_VALID, None, [inputFile])
        with file_context(DOMCAL_XML_1_62) as inputFile:
            DOMCalImport.doInsert(db, RUN_VALID, None, [inputFile])
        with file_context(SPE_FIT_JSON) as inputFile:
            SPEFitImport.doInsert(db, RUN_VALID, None, [inputFile])
        with calDBInserter(db, RUN_VALID) as calInserter:
            calInserter.insert(G.DataObject.wrapdict(DOM_DROOP_1_61))
            calInserter.commit()

        # Build I3Calibration
        cal = getI3Calibration(db)
        # Ensure the calibration frame was built correctly
        assert len(cal.vem_cal) == 2
        for (key, val) in cal.vem_cal:
            assert key in [OMKey(1, 61), OMKey(1, 62)]
            compareVEMCal(key, val)

        assert len(cal.dom_cal) == 2
        for (key, val) in cal.dom_cal:
            assert key in [OMKey(1, 61), OMKey(1, 62)]
            compareDOMCal(key, val)
        
        # Check that we can handle Spencer's new SPE fits
        with file_context(SPENCER_FIT_JSON) as inputFile:
            SPEFitImport.doInsert(db, RUN_VALID, None, [inputFile])
        cal = getI3Calibration(db)
        assert OMKey(1, 61) in cal.dom_cal
        for (key, val) in cal.dom_cal:
            if key == OMKey(1, 61):
                assert val.is_mean_atwd_charge_valid == True
                assert val.is_mean_fadc_charge_valid == True
                assert val.mean_atwd_charge == 1.0132495950223077
                assert val.mean_fadc_charge == 1.1605599339723862
                scd = val.combined_spe_charge_distribution
                assert scd.exp1_amp == 6.9
                assert scd.exp1_width == 0.032
                assert scd.exp2_amp == 0.5439097946821353
                assert scd.exp2_width == 0.43767916504836674
                assert scd.gaus_amp == 0.7453731951866129
                assert scd.gaus_mean == 1.0132495950223077
                assert scd.gaus_width == 0.28970067994005266
                assert scd.compensation_factor == 1.2535007845267907
                assert scd.slc_gaus_mean == 1.1605599339723862
                

if __name__ == "__main__":
    test_calibration()    