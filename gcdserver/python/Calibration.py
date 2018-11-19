
import Geometry as G

"""
file: Calibration.py
Contains all of the string keys for calibration documents along with
simple routines and classes to access the data.  This file is
designed to internalize all knowledge of calibration database documents.
"""


class ATWDConstants(object):
    """
    Basic constants for DOM ATWD digitization
    """
    N_ATWDS = 2
    N_CHANNELS = 3
    N_BINS = 128

    @classmethod
    def checkValid(cls, atwd=0, channel=0, bin=0):
        assert atwd < ATWDConstants.N_ATWDS
        assert channel < ATWDConstants.N_CHANNELS
        assert bin < ATWDConstants.N_BINS

    @classmethod
    def atwdKey(cls, atwd):
        return "atwd%d" % atwd

    @classmethod
    def channelKey(cls, channel):
        return "channel%d" % channel


class ObjectType(object):
    """
    Enumeration of calibration DB object type strings
    """
    SNOW_HEIGHT = "Snow Height"
    NOISE_RATE = "Noise Rate"
    TOROID_DROOP = "Toroid Droop"
    VEMCAL = "IceTop Muon Calibration"
    BEACON_BASELINES = "Beacon Baseline Averages"
    GAIN_CAL = "GainVsHV"
    FADC_BASELINE = "FADC Baseline"
    ATWD_CAL = "ATWD Calibration"
    FADC_DELTA_T_CAL = "FADC Delta-T Calibration"
    FADC_GAIN_CAL = "FADC Gain Calibration"
    PMT_TRANSIT_TIME_CAL = "PMT Transit Time Calibration"
    PMT_DISC_CAL = "PMT Discriminator Calibration"
    ATWD_FREQ_CAL = "ATWD Frequency Calibration"
    ATWD_DELTA_T_CAL = "ATWD Delta-T Calibration"
    AMP_CAL = "Amplifier Calibration"
    FRONT_END_IMPEDANCE = "Front End Impedance"
    SPE_DISC_CAL = "SPE Discriminator Calibration"
    MPE_DISC_CAL = "MPE Discriminator Calibration"
    ATWD_SPE_CORR = "ATWD SPE Corrections"
    FADC_SPE_CORR = "FADC SPE Corrections"
    JOINT_SPE_CORR = "Joint ATWD/FADC SPE Corrections"


class Keys(object):

    # Snow height objects
    SNOW_HEIGHT = "Snow Height"
    DATE = "date"

    # Noise rate objects
    NOISE_RATE = "NoiseRate"

    # DOM toriod droop objects
    TAU0 = "tau0"
    TAU1 = "tau1"
    TAU2 = "tau2"
    TAU3 = "tau3"
    TAU4 = "tau4"
    TAU5 = "tau5"
    TAU_FRACTION = "tauFraction"
    ATWD_SIGMA = "ATWD Sigma"
    FADC_SIGMA = "FADC Sigma"

    # VEM Calibration
    PE_PER_VEM ="pePerVEM"
    MUON_PEAK_WIDTH = "muPeakWidth"
    HG_LG_CROSSOVER = "hglgCrossOver"
    CORR_FACTOR = "corrFactor"

    # Beacon baselines
    FADC_BASELINE = "fadcBaseline"

    # DOMCal base keys
    VERSION = "version"
    TEMPERATURE = "temperature"
    GAIN = "gain"

    # DOMCal linear fit
    SLOPE = "slope"
    INTERCEPT = "intercept"
    REG_COEFF = "regressionCoeff"

    # DOMCal delta-T calibrations
    DELTA_T = "deltaT"
    
    # Front-end impedance
    FE_IMPEDANCE = "frontEndImpedance"

    # SPE Corrections
    CHI_2 = "Chi2"
    ERROR = "Error"
    EXP_NORM = "Exp Normalization"
    EXP2_NORM = "Exp2 Normalization"
    EXP_SCALE = "Exp Scale"
    EXP2_SCALE = "Exp2 Scale"
    GAUSS_MEAN = "Gaussian Mean"
    GAUSS_NORM = "Gaussian Norm"
    GAUSS_STDDEV = "Gaussian Standard Dev"
    SPE_COMP_FACTOR = "SPE Compensation Factor"
    NDF = "NDF"
    N_ENTRIES = "Number of Entries"
    VALID = "Valid"
    X_MAX_F = "X_Max_F"


class CalibrationData(G.DictionaryBacked):
    """
    Base calibration data class providing __getitem__ and __setitem__
    access on the backing dictionary
    """
    def __init__(self):
        super(CalibrationData, self).__init__()

    def __getitem__(self, key):
        return self._data[key]

    def __setitem__(self, key, value):
        self._data[key] = value


class BeaconBaseline(CalibrationData):
    """
    Explicit routines to get/set ATWD average baseline values
    """
    def __init__(self):
        super(BeaconBaseline, self).__init__()

    def setATWDBaseline(self, atwd, channel, value):
        ATWDConstants.checkValid(atwd, channel)
        atwdKey = ATWDConstants.atwdKey(atwd)
        if not atwdKey in self.getdict():
            self.getdict()[atwdKey] = {}
        chKey = ATWDConstants.channelKey(channel)
        self.getdict()[atwdKey][chKey] = value

    def getATWDBaseline(self, atwd, channel):
        ATWDConstants.checkValid(atwd, channel)
        atwdKey = ATWDConstants.atwdKey(atwd)
        chKey = ATWDConstants.channelKey(channel)
        return self.getdict()[atwdKey][chKey]


class ATWDCalibration(CalibrationData):
    """
    Voltage vs ATWD bin ADC value.  Keep only the slope.  Minimize
    document size by placing slopes for each bin into an array.
    """
    def __init__(self):
        super(ATWDCalibration, self).__init__()
        for atwd in range(ATWDConstants.N_ATWDS):
            atwdKey = ATWDConstants.atwdKey(atwd)
            self.getdict()[atwdKey] = {}
            for channel in range(ATWDConstants.N_CHANNELS):
                chKey = ATWDConstants.channelKey(channel)
                self.getdict()[atwdKey][chKey] = [None] * ATWDConstants.N_BINS

    def setSlope(self, atwd, channel, bin, value):
        ATWDConstants.checkValid(atwd, channel, bin)
        atwdKey = ATWDConstants.atwdKey(atwd)
        chKey = ATWDConstants.channelKey(channel)
        self.getdict()[atwdKey][chKey][bin] = value

    def getSlope(self, atwd, channel, bin):
        ATWDConstants.checkValid(atwd, channel, bin)
        atwdKey = ATWDConstants.atwdKey(atwd)
        chKey = ATWDConstants.channelKey(channel)
        return self.getdict()[atwdKey][chKey][bin]


class ATWDFrequencyCal(CalibrationData):

    C0 = "c0"
    C1 = "c1"
    C2 = "c2"
    COEFF = "regressionCoeff"

    dataNameString = "ATWD Frequency Calibration"
    def __init__(self):
        super(ATWDFrequencyCal, self).__init__()

    def setFit(self, atwd, c0, c1, c2, coeff):
        ATWDConstants.checkValid(atwd)
        atwdKey = ATWDConstants.atwdKey(atwd)
        if not atwdKey in self.getdict():
            self.getdict()[atwdKey] = {}
        self.getdict()[atwdKey][self.C0] = c0
        self.getdict()[atwdKey][self.C1] = c1
        self.getdict()[atwdKey][self.C2] = c2
        self.getdict()[atwdKey][self.COEFF] = coeff

    def getFit(self, atwd):
        ATWDConstants.checkValid(atwd)
        atwdKey = ATWDConstants.atwdKey(atwd)
        return (self.getdict()[atwdKey][self.C0],
                self.getdict()[atwdKey][self.C1],
                self.getdict()[atwdKey][self.C2])

    def getFitRegressionCoeff(self, atwd):
        ATWDConstants.checkValid(atwd)
        atwdKey = ATWDConstants.atwdKey(atwd)
        return self.getdict()[atwdKey][self.COEFF]


class ATWDDeltaTCal(CalibrationData):
    """
    Time offset for ATWD0 and ATWD1
    """
    def __init__(self):
        super(ATWDDeltaTCal, self).__init__()

    def setDeltaT(self, atwd, deltaT):
        ATWDConstants.checkValid(atwd)
        atwdKey = ATWDConstants.atwdKey(atwd)
        self.getdict()[atwdKey] = deltaT

    def getDeltaT(self, atwd):
        ATWDConstants.checkValid(atwd)
        atwdKey = ATWDConstants.atwdKey(atwd)
        return self.getdict()[atwdKey]


class AmplifierCal(CalibrationData):
    """
    Gains of the three front-end amplifiers
    """
    def __init__(self, name=None):
        super(AmplifierCal, self).__init__()

    def setGain(self, channel, gain):
        ATWDConstants.checkValid(channel=channel)
        chKey = ATWDConstants.channelKey(channel)
        self.getdict()[chKey] = gain

    def getGain(self, channel):
        ATWDConstants.checkValid(channel=channel)
        chKey = ATWDConstants.channelKey(channel)
        return self.getdict()[chKey]