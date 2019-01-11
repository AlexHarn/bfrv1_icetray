
from I3Tray import I3Units
from icecube import dataclasses
from icecube.daq_decode import I3DAQDecoderUtil
from icecube.icetray import logging

import icecube.gcdserver.Geometry as G
import icecube.gcdserver.Calibration as C
import icecube.gcdserver.DetectorStatus as D
import icecube.gcdserver.util as util

import math


def getSubdetector(rConf):
    id = int(rConf["readoutType"])
    sub = dataclasses.I3TriggerStatus.NOT_SPECIFIED
    if id == 0:
        sub = dataclasses.I3TriggerStatus.ALL
    elif id == 1:
        sub = dataclasses.I3TriggerStatus.ICETOP
    elif id == 2:
        sub = dataclasses.I3TriggerStatus.INICE
    else:
        logging.log_warn("I3DetectorStatus: "
                         "Found bad subdetector ID: %d" % id)
    return sub


def buildTriggerStatus(trigConf):
    ts = dataclasses.I3TriggerStatus()
    ts.trigger_name = str(trigConf["triggerName"])
    # if one parameterConfig element is present, data will not be a list
    parameterConfigs = trigConf.get("parameterConfig", [])
    if type(parameterConfigs) is not list:
        parameterConfigs = [parameterConfigs]
    for pConf in parameterConfigs:
        ts.trigger_settings[str(pConf["parameterName"])] = \
                                         str(pConf["parameterValue"])
    readoutConfigs = trigConf.get("readoutConfig", [])
    if type(readoutConfigs) is not list:
        readoutConfigs = [readoutConfigs]
    for rConf in readoutConfigs:
        readoutConfig = dataclasses.I3TriggerStatus.I3TriggerReadoutConfig()
        readoutConfig.readout_time_offset = int(rConf["timeOffset"])
        readoutConfig.readout_time_minus = int(rConf["timeMinus"])
        readoutConfig.readout_time_plus = int(rConf["timePlus"])
        ts.readout_settings[getSubdetector(rConf)] = readoutConfig
    return ts


def getTrigMode(domConf):
    mode = dataclasses.I3DOMStatus.UnknownTrigMode
    modeStr = domConf["triggerMode"]
    if modeStr is not None:
        if modeStr == "testpattern":
            mode = dataclasses.I3DOMStatus.TestPattern
        elif modeStr == "cpu":
            mode = dataclasses.I3DOMStatus.CPU
        elif modeStr == "spe":
            mode = dataclasses.I3DOMStatus.SPE
        elif modeStr == "flasher":
            mode = dataclasses.I3DOMStatus.Flasher
        elif modeStr == "mpe":
            mode = dataclasses.I3DOMStatus.MPE
        else:
            logging.log_warn("I3DetectorStatus: Unsupported "
                             "trigger mode: %s" % modeStr)
    return mode


def getSubvalue(domConf, key1, key2, default=None):
    ret = default
    outer = domConf.get(key1)
    if outer is not None:
        value = outer.get(key2)
        if value is not None:
            ret = value
    return ret


def getLCValue(domConf, key, default=None):
    return getSubvalue(domConf, "localCoincidence", key, default)


def getEngineering(domConf, key, default=None):
    return getSubvalue(domConf, "engineeringFormat", key, default)


def getLCMode(domConf):
    mode = dataclasses.I3DOMStatus.UnknownLCMode
    modeStr = getLCValue(domConf, "mode")
    if modeStr is not None:
        if modeStr == "none":
            mode = dataclasses.I3DOMStatus.LCOff
        elif modeStr == "up-or-down":
            mode = dataclasses.I3DOMStatus.UpOrDown
        elif modeStr == "up":
            mode = dataclasses.I3DOMStatus.Up
        elif modeStr == "down":
            mode = dataclasses.I3DOMStatus.Down
        elif modeStr == "up-and-down":
            mode = dataclasses.I3DOMStatus.UpAndDown
        elif modeStr == "headers-only":
            mode = dataclasses.I3DOMStatus.SoftLC
        else:
            logging.log_warn("I3DetectorStatus: "
                             "Unsupported LC mode: %s" % modeStr)
    return mode


def getLCTXMode(domConf):
    mode = dataclasses.I3DOMStatus.UnknownLCMode
    modeStr = getLCValue(domConf, "txMode")
    if modeStr is not None:
        if modeStr == "none":
            mode = dataclasses.I3DOMStatus.LCOff
        elif modeStr == "both":
            mode = dataclasses.I3DOMStatus.UpAndDown
        elif modeStr == "up":
            mode = dataclasses.I3DOMStatus.Up
        elif modeStr == "down":
            mode = dataclasses.I3DOMStatus.Down
        else:
            logging.log_warn("I3DetectorStatus: "
                             "Unsupported LC TX mode: %s" % modeStr)
    return mode


def getLCType(domConf):
    # Just like I3DB, use "hard" by default
    type = "hard"
    typeStr = getLCValue(domConf, "type")
    if typeStr == "soft":
        type = "soft"
    return type


def getGainType(db, pmt_hv, domConf):
    deviceName = db.mbidMap(str(domConf["@mbid"]).lower())
    objectType = db.deployedNameMap(deviceName).type
    # Shamelessly copy from I3Db: All IceCube DOMs are high-gain.
    if objectType == G.ObjectType.ICECUBE_DOM:
        return dataclasses.I3DOMStatus.High
    # We don't know the meaning of gain type for non-DOM objects
    if objectType != G.ObjectType.ICETOP_DOM:
        return dataclasses.I3DOMStatus.UnknownGainType
    # We need to get the gain fit to make this determination.
    # This data should really be elsewhere.  We *know* which
    # DOMs are low-gain.  We shouldn't need to check the HV.
    gainType = dataclasses.I3DOMStatus.UnknownGainType
    gainCals = [x for x in db.calibrationDocuments(C.ObjectType.GAIN_CAL,
                                                                deviceName)]
    if pmt_hv > 0 and len(gainCals) == 1:
        log10HV = math.log10(pmt_hv / I3Units.V)
        log10Gain = (gainCals[0].data[C.Keys.INTERCEPT] +
                          log10HV * gainCals[0].data[C.Keys.SLOPE])
        gain = math.pow(10, log10Gain)
        gainType = dataclasses.I3DOMStatus.High
        if gain < 25e5:
            gainType = dataclasses.I3DOMStatus.Low
    return gainType


def buildDOMStatus(db, omKey, domConf):
    ds = dataclasses.I3DOMStatus()
    ds.status_atwd_a = dataclasses.I3DOMStatus.On
    ds.status_atwd_b = dataclasses.I3DOMStatus.On
    chipSelect = domConf.get('atwdChipSelect')
    if chipSelect is not None:
        if chipSelect == "A":
            ds.status_atwd_b = dataclasses.I3DOMStatus.Off
        elif chipSelect == "B":
            ds.status_atwd_a = dataclasses.I3DOMStatus.Off
        else:
            logging.log_warn("I3DetectorStatus: "
                             "Found bad ATWD status: %s" % chipSelect)

    ds.trig_mode = getTrigMode(domConf)
    ds.cable_type = dataclasses.I3DOMStatus.Terminated
    # This is a hack from I3DB. We could probably put this in the
    # geometry database
    if omKey.om % 2 == 1 and omKey.om < 60:
        ds.cable_type = dataclasses.I3DOMStatus.Unterminated
    ds.lc_mode = getLCMode(domConf)
    ds.tx_mode = getLCTXMode(domConf)

    ds.slc_active = False
    if getLCType(domConf) == "soft":
        if (ds.lc_mode in [dataclasses.I3DOMStatus.UpOrDown,
                           dataclasses.I3DOMStatus.SoftLC,
                           dataclasses.I3DOMStatus.Up,
                           dataclasses.I3DOMStatus.Down]):
            ds.slc_active = True

    # NB: All defaults are zero.  Instead of defining specific defaults,
    # just assign zero, and add defaults if/when needed
    ds.lc_window_pre = float(
                        getLCValue(domConf, "preTrigger", 0)) * I3Units.ns
    ds.lc_window_post = float(
                        getLCValue(domConf, "postTrigger", 0)) * I3Units.ns
    ds.lc_span = int(getLCValue(domConf, "span", 0))

    ds.n_bins_fadc = int(getEngineering(domConf["format"], "fadcSamples", 0))
    atwdFormat = getEngineering(domConf["format"], "atwd", [])
    ds.n_bins_atwd_0 = 0
    ds.n_bins_atwd_1 = 0
    ds.n_bins_atwd_2 = 0
    ds.n_bins_atwd_3 = 0
    for atwd in atwdFormat:
        ch = int(atwd["ch"])
        bins = int(atwd["samples"])
        if ch == 0:
            ds.n_bins_atwd_0 = bins
        elif ch == 1:
            ds.n_bins_atwd_1 = bins
        elif ch == 2:
            ds.n_bins_atwd_2 = bins
        elif ch == 3:
            ds.n_bins_atwd_3 = bins
        else:
            logging.log_warn("I3DetectorStatus: "
                             "Unsupported ATWD channel: %s" % ch)

    ds.status_fadc = dataclasses.I3DOMStatus.Off
    if ds.n_bins_fadc > 0:
        ds.status_fadc = dataclasses.I3DOMStatus.On

    ds.delta_compress = dataclasses.I3DOMStatus.Off
    if domConf["format"].get('deltaCompressed') is not None:
        ds.delta_compress = dataclasses.I3DOMStatus.On

    ds.dac_fadc_ref = int(domConf.get("fastAdcRef", 0))
    # N.B. The PMT HV DAC is in units of 0.5 volt
    ds.pmt_hv = 0.5 * (int(domConf.get("pmtHighVoltage", 0))) * I3Units.V
    ds.fe_pedestal = int(domConf.get("frontEndPedestal", 0))
    ds.spe_threshold = int(domConf.get("speTriggerDiscriminator", 0))
    ds.mpe_threshold = int(domConf.get("mpeTriggerDiscriminator", 0))
    ds.dac_trigger_bias_0 = float(domConf.get("atwd0TriggerBias", 0))
    ds.dac_trigger_bias_1 = float(domConf.get("atwd1TriggerBias", 0))
    ds.dom_gain_type = getGainType(db, ds.pmt_hv, domConf)
    return ds


def buildI3DetectorStatus(db, runData):
    """
    Build an I3DetectorStatus instance from GCD data.
    @db: Access to set of valid geometry, calibration, and detector status data
    @return: dataclasses.I3DetectorStatus instance.
    """
    status = dataclasses.I3DetectorStatus()
    util.setI3Time(status.start_time, runData.startTime, runData.startFrac)
    if runData.stopTime is not None:
        util.setI3Time(status.end_time, runData.stopTime, runData.stopFrac)
    else:
        status.end_time.set_daq_time(9999, 0)
    status.daq_configuration_name = str(runData.configName)

    # Build the trigger status map
    for trigConfList in db.statusDocuments(D.ObjectType.TRIGGER_CONFIG_LIST):
        o = D.TriggerConfigList.wrapdict(trigConfList.data)
        for trigConf in o.getTriggerConfigs():
            key = I3DAQDecoderUtil().create_trigger_key(
                                           int(trigConf["triggerType"]),
                                           int(trigConf["triggerConfigId"]),
                                           int(trigConf["sourceId"]))
            status.trigger_status[key] = buildTriggerStatus(trigConf)

    # Build the DOM status map
    for domStatusList in db.statusDocuments(D.ObjectType.DOM_CONFIG_LIST):
        o = D.DOMConfigList.wrapdict(domStatusList.data)
        for domConf in o.getDOMConfigs():
            deviceName = db.mbidMap(str(domConf["@mbid"]).lower())
            omKey = util.getOMKey(db.deployedNameMap(deviceName))
            status.dom_status[omKey] = buildDOMStatus(db, omKey, domConf)

    return status