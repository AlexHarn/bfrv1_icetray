#!/usr/bin/env python

import os
import sys
from datetime import datetime
import xml.etree.cElementTree as ElementTree

import icecube.gcdserver.Geometry as G
import icecube.gcdserver.Calibration as C
from icecube.gcdserver.OptionParser import GCDOptionParser
from icecube.gcdserver.MongoDB import getDB, fillBlobDB
from icecube.gcdserver.I3MS import calDBInserter


class LinearFit(object):
    def __init__(self, slope=None, intercept=None, regressionCoeff=None):
        self.slope = slope
        self.intercept = intercept
        self.regressionCoeff = regressionCoeff

    @classmethod
    def fromElement(cls, element):
        fit = LinearFit()
        try:
            fit.regressionCoeff = float(
                                    (element.find('regression-coeff')).text)
            params = element.findall('param')
            assert len(params) == 2
            for param in element.findall('param'):
                if param.attrib['name'] == 'slope':
                    fit.slope = float(param.text)
                if param.attrib['name'] == 'intercept':
                    fit.intercept = float(param.text)
            assert fit.slope is not None and fit.intercept is not None
            return fit
        except:
            return None

class DOMCalImporter(object):

    def __init__(self, geoDB, inserter):
        self.__geoDB = geoDB
        self.__inserter = inserter

    def __createResult(self, root, type, data=None):
        mbid = ((root.find('domid')).text).lower()
        # Use name of corresponding Geometry object as the object name
        result = G.DataObject(self.__geoDB.mbidMap(mbid), type)
        if data is not None:
            result.data = data.getdict()
        return result

    def __saveResult(self, root, result):
        if result is None:
            return

        # Grab global DOMCal data from the XML
        result.data[C.Keys.VERSION] = str(root.attrib['version'])
        recordDate = (root.find('date')).text
        recordTime = (root.find('time')).text
        result.data[C.Keys.DATE] = str(datetime.strptime("%s %s" %
                           (recordDate, recordTime), "%m-%d-%Y %H:%M:%S"))
        result.data[C.Keys.TEMPERATURE] = float(
                                          (root.find('temperature')).text)
        self.__inserter.insert(result)

    def __copyFitData(self, result, fitElement):
        assert fitElement is not None
        fit = LinearFit.fromElement(fitElement)
        result.data[C.Keys.SLOPE] = fit.slope
        result.data[C.Keys.INTERCEPT] = fit.intercept
        result.data[C.Keys.REG_COEFF] = fit.regressionCoeff

    def __importGainVsHV(self, root):
        gainVsHV = root.find('hvGainCal')
        if gainVsHV is None:
            return
        result = self.__createResult(root, C.ObjectType.GAIN_CAL)
        self.__copyFitData(result, gainVsHV.find('fit'))
        self.__saveResult(root, result)

    def __importATWDCal(self, root):
        if len(root.findall('atwd')) == 0:
            return
        o = C.ATWDCalibration()
        for e in root.findall('atwd'):
            fit = LinearFit.fromElement(e.find('fit'))
            assert fit is not None
            o.setSlope(int(e.attrib['id']), int(e.attrib['channel']),
                       int(e.attrib['bin']), fit.slope)
        result = self.__createResult(root, C.ObjectType.ATWD_CAL, o)
        self.__saveResult(root, result)

    def __importFADCDeltaT(self, root):
        e = root.find('fadc_delta_t')
        if e is None:
            return
        result = self.__createResult(root, C.ObjectType.FADC_DELTA_T_CAL)
        result.data[C.Keys.DELTA_T] = float((e.find('delta_t')).text)
        self.__saveResult(root, result)

    def __importATWDDeltaT(self, root):
        if len(root.findall('atwd_delta_t')) == 0:
            return
        o = C.ATWDDeltaTCal()
        for e in root.findall('atwd_delta_t'):
            atwd = int(e.attrib['id'])
            o.setDeltaT(atwd, float((e.find('delta_t')).text))
        result = self.__createResult(root, C.ObjectType.ATWD_DELTA_T_CAL, o)
        self.__saveResult(root, result)

    def __importFADCGainCal(self, root):
        e = root.find('fadc_gain')
        if e is None:
            return
        result = self.__createResult(root, C.ObjectType.FADC_GAIN_CAL)
        result.data[C.Keys.GAIN] = float((e.find('gain')).text)
        self.__saveResult(root, result)

    def __importPMTTransitTimeCal(self, root):
        pmtTransitTime = root.find('pmtTransitTime')
        if pmtTransitTime is None:
            return
        result = self.__createResult(root, C.ObjectType.PMT_TRANSIT_TIME_CAL)
        self.__copyFitData(result, pmtTransitTime.find('fit'))
        self.__saveResult(root, result)

    def __importPMTDiscCal(self, root):
        pmtDiscCal = root.find('pmtDiscCal')
        if pmtDiscCal is None:
            return
        result = self.__createResult(root, C.ObjectType.PMT_DISC_CAL)
        self.__copyFitData(result, pmtDiscCal.find('fit'))
        self.__saveResult(root, result)

    def __importATWDFreqCal(self, root):
        if len(root.findall('atwdfreq')) == 0:
            return
        o = C.ATWDFrequencyCal()
        for e in root.findall('atwdfreq'):
            atwd = int(e.attrib['atwd'])
            fit = e.find('fit')
            coeff = float((fit.find('regression-coeff')).text)
            c0 = c1 = c2 = None
            for param in fit.findall('param'):
                if param.attrib['name'] == "c0":
                    c0 = float(param.text)
                elif param.attrib['name'] == "c1":
                    c1 = float(param.text)
                elif param.attrib['name'] == "c2":
                    c2 = float(param.text)
            assert not any(x == None for x in [c0, c1, c2])
            o.setFit(atwd, c0, c1, c2, coeff)
        result = self.__createResult(root, C.ObjectType.ATWD_FREQ_CAL, o)
        self.__saveResult(root, result)

    def __importAmplifierCal(self, root):
        if len(root.findall('amplifier')) == 0:
            return
        
        o = C.AmplifierCal()
        for e in root.findall('amplifier'):
            channel = int(e.attrib['channel'])
            o.setGain(channel, float((e.find('gain')).text))
        result = self.__createResult(root, C.ObjectType.AMP_CAL, o)
        self.__saveResult(root, result)

    def __importImpedance(self, root):
        impedance = root.find('frontEndImpedance')
        if impedance is None:
            return
        result = self.__createResult(root, C.ObjectType.FRONT_END_IMPEDANCE)
        result.data[C.Keys.FE_IMPEDANCE] = float(impedance.text)
        self.__saveResult(root, result)

    def __importDiscCal(self, root):
        for e in root.findall('discriminator'):
            assert e.attrib['id'] in ['spe', 'mpe']
            result = self.__createResult(root, C.ObjectType.SPE_DISC_CAL)
            if e.attrib['id'] == 'mpe':
                result = self.__createResult(root, C.ObjectType.MPE_DISC_CAL)
            self.__copyFitData(result, e.find('fit'))
            self.__saveResult(root, result)

    def __importFADCBaseline(self, root):
        for e in root.findall('fadc_baseline'):
            result = self.__createResult(root, C.ObjectType.FADC_BASELINE)
            self.__copyFitData(result, e.find('fit'))
            self.__saveResult(root, result)

    def importFile(self, inputFile):

        # Parse the XML tree
        tree = ElementTree.parse(inputFile)
        root = tree.getroot()
        if root.tag != "domcal":
            raise Exception("Unable to find domcal tag")

        # Import each part
        self.__importGainVsHV(root)
        self.__importATWDCal(root)
        self.__importATWDDeltaT(root)
        self.__importFADCDeltaT(root)
        self.__importFADCGainCal(root)
        self.__importPMTTransitTimeCal(root)
        self.__importPMTDiscCal(root)
        self.__importATWDFreqCal(root)
        self.__importAmplifierCal(root)
        self.__importImpedance(root)
        self.__importDiscCal(root)
        self.__importFADCBaseline(root)


def doInsert(db, runValid, i3msHost, files):

    # Get BlobDB instance loaded with geometry data
    geoDB = fillBlobDB(db, run=None)

    errCode = 0
    fileCnt = 0
    with calDBInserter(db, runValid, i3msHost) as inserter:
        importer = DOMCalImporter(geoDB, inserter)
        for file in files:
            try:
                importer.importFile(os.path.abspath(file))
                fileCnt += 1
            except Exception as e:
                errCode = -1
                print("Unable to import file %s: %s" % (file, e))
        inserter.commit()
    print("Imported %d DOMCal file(s)" % fileCnt)
    return errCode


if __name__ == "__main__":
    parser = GCDOptionParser()
    parser.add_option("-r", "--runValid", dest="runValid",
                      help="runValid entry for calibration quantities")
    (options, args) = parser.parse_args()
    if options.runValid == None:
        print("Calibration runValid not specified")
        parser.print_help()
        sys.exit(-1)
    if len(args) == 0:
        print("No DOMCal files specified")
        sys.exit(-1)
    errCode = doInsert(getDB(options.dbhost, options.dbuser, options.dbpass),
                       options.runValid, options.i3mshost, args)
    sys.exit(errCode)
