#!/usr/bin/env python

from contexts import (test_db_context, file_context,
                      tempdir_context, config_file_context)
from TestData import (DOMCAL_XML_1_61, DOMCAL_XML_1_62, GEOMETRY_1_61,
                      GEOMETRY_1_62)
import icecube.gcdserver.DOMCalImport as DOMCalImport
import icecube.gcdserver.ConfigImport as ConfigImport
from icecube.gcdserver.I3MS import geoDBInserter
from icecube.gcdserver.MongoDB import fillBlobDB
from icecube.gcdserver.I3DetectorStatusBuilder import buildI3DetectorStatus
from icecube.gcdserver.RunData import RunData
import icecube.gcdserver.Geometry as G
import icecube.gcdserver.util as util

from icecube.icetray import OMKey, I3Units
from icecube import dataclasses
from icecube.dataclasses import I3TriggerStatus
from icecube.daq_decode import I3DAQDecoderUtil
import os
from contextlib import contextmanager


RUN_VALID = 1
CONFIG_NAME = "config"
START_TIME = "2016-12-16 21:45:20"
STOP_TIME = "2016-12-16 21:45:30"
START_TIME_FRAC = 5989746668
STOP_TIME_FRAC = 1837394365


THROUGHPUT_TRIG_KEY = I3DAQDecoderUtil().create_trigger_key(3, -1, 6000)
FIXED_RATE_TRIG_KEY = I3DAQDecoderUtil().create_trigger_key(23, 23050, 4000)
SMT_TRIG_KEY = I3DAQDecoderUtil().create_trigger_key(0, 1006, 4000)


def checkTriggerStatus(key, status):
    if key == THROUGHPUT_TRIG_KEY:
        assert status.trigger_name == "ThroughputTrigger"
        assert len(status.readout_settings) == 0
        assert len(status.trigger_settings) == 0
    elif key == FIXED_RATE_TRIG_KEY:
        assert status.trigger_name == "FixedRateTrigger"
        assert len(status.readout_settings) == 1
        rc = dataclasses.I3TriggerStatus.I3TriggerReadoutConfig()
        rc.readout_time_minus = 5000000
        rc.readout_time_plus = 5000000
        rc.readout_time_offset = 0
        assert status.readout_settings[I3TriggerStatus.ALL] == rc
        assert len(status.trigger_settings) == 1
        assert status.trigger_settings["interval"] == str(300000000000)
    else:
        assert status.trigger_name == "SimpleMajorityTrigger"
        assert len(status.readout_settings) == 2
        rc = dataclasses.I3TriggerStatus.I3TriggerReadoutConfig()
        rc.readout_time_minus = 10000
        rc.readout_time_plus = 10000
        rc.readout_time_offset = 0
        assert status.readout_settings[I3TriggerStatus.ICETOP] == rc
        rc.readout_time_minus = 4000
        rc.readout_time_plus = 6000
        assert status.readout_settings[I3TriggerStatus.INICE] == rc
        assert len(status.trigger_settings) == 2
        assert status.trigger_settings["threshold"] == str(8)
        assert status.trigger_settings["timeWindow"] == str(5000)


def checkDOMStatus(key, status):
    if key == OMKey(1, 62):
        assert status.dom_gain_type == dataclasses.I3DOMStatus.Low
        assert status.dac_trigger_bias_0 == 814
        assert status.dac_trigger_bias_1 == 819
        assert status.mpe_threshold == 550
        assert status.spe_threshold == 620
        assert status.pmt_hv == 700.5 * I3Units.V
        assert status.tx_mode == dataclasses.I3DOMStatus.LCOff
        assert status.trig_mode == dataclasses.I3DOMStatus.SPE
    else:
        assert status.dom_gain_type == dataclasses.I3DOMStatus.High
        assert status.dac_trigger_bias_0 == 829
        assert status.dac_trigger_bias_1 == 802
        assert status.mpe_threshold == 575
        assert status.spe_threshold == 600
        assert status.pmt_hv == 1261.5 * I3Units.V
        assert status.tx_mode == dataclasses.I3DOMStatus.UpAndDown
        assert status.trig_mode == dataclasses.I3DOMStatus.MPE
    assert status.cable_type == dataclasses.I3DOMStatus.Terminated
    assert status.dac_fadc_ref == 800
    assert status.delta_compress == dataclasses.I3DOMStatus.On
    assert status.fe_pedestal == 2130
    assert status.lc_mode == dataclasses.I3DOMStatus.UpOrDown
    assert status.lc_span == 1
    assert status.lc_window_post == 2000 * I3Units.ns
    assert status.lc_window_pre == 1000 * I3Units.ns
    assert status.n_bins_atwd_0 == 0
    assert status.n_bins_atwd_1 == 0
    assert status.n_bins_atwd_2 == 0
    assert status.n_bins_atwd_3 == 0
    assert status.n_bins_fadc == 0
    assert status.slc_active == True
    assert status.status_atwd_a == dataclasses.I3DOMStatus.On
    assert status.status_atwd_b == dataclasses.I3DOMStatus.On
    assert status.status_fadc == dataclasses.I3DOMStatus.Off


def test_detector_status():
    with test_db_context() as db:
        # Import geometry data for string 1, DOMs 61 and 62
        with geoDBInserter(db) as geoInserter:
            geoInserter.insert(G.GeometryObject.wrapdict(GEOMETRY_1_61))
            geoInserter.insert(G.GeometryObject.wrapdict(GEOMETRY_1_62))
            geoInserter.commit()
        # Need DOMCal data to determine whether IceTop DOMs are high/low gain
        with file_context(DOMCAL_XML_1_61) as inputFile:
            DOMCalImport.doInsert(db, RUN_VALID, None, [inputFile])
        with file_context(DOMCAL_XML_1_62) as inputFile:
            DOMCalImport.doInsert(db, RUN_VALID, None, [inputFile])
        with config_file_context(CONFIG_NAME) as configFile:
            ConfigImport.doInsert(db, [configFile], None)
        # Get the BlobDB instance
        geoDB = fillBlobDB(db, run=1, configuration=CONFIG_NAME)
        # Build I3DetectorStatus
        rd = RunData(configName=CONFIG_NAME,
                     startTime=START_TIME,
                     startFrac=START_TIME_FRAC,
                     stopTime=STOP_TIME,
                     stopFrac=STOP_TIME_FRAC)
        ds = buildI3DetectorStatus(geoDB, rd)
        time = dataclasses.I3Time()
        util.setI3Time(time, START_TIME, START_TIME_FRAC)
        assert time == ds.start_time
        assert time.get_utc_string("%Y-%m-%d %H:%M:%S") == START_TIME
        assert time.utc_nano_sec == START_TIME_FRAC * 1e-1
        util.setI3Time(time, STOP_TIME, STOP_TIME_FRAC)
        assert time == ds.end_time
        assert time.get_utc_string("%Y-%m-%d %H:%M:%S") == STOP_TIME
        assert time.utc_nano_sec == STOP_TIME_FRAC * 1e-1
        assert ds.daq_configuration_name == CONFIG_NAME
        # Now check trigger and dom configs
        assert len(ds.trigger_status) == 3
        for (key, status) in ds.trigger_status:
            assert key in [THROUGHPUT_TRIG_KEY, FIXED_RATE_TRIG_KEY, SMT_TRIG_KEY]
            checkTriggerStatus(key, status)
        assert len(ds.dom_status) == 2
        for (key, status) in ds.dom_status:
            assert key in [OMKey(1, 61), OMKey(1, 62)]
            checkDOMStatus(key, status)


if __name__ == "__main__":
    test_detector_status()    