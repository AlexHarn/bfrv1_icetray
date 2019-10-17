from __future__ import print_function
from os.path import expandvars
from icecube import icetray
import math
import warnings

"""
Convenience functions for configuring CLSim components.
"""

def configureOpenCLDevices(UseGPUs=True, UseCPUs=False, OverrideApproximateNumberOfWorkItems=None, DoNotParallelize=True, UseOnlyDeviceNumber=None):
    # get OpenCL devices
    from icecube.clsim import I3CLSimOpenCLDevice
    from icecube.icetray import logging
    import string
    
    openCLDevicesRaw = [device for device in I3CLSimOpenCLDevice.GetAllDevices() if (device.gpu and UseGPUs) or (device.cpu and UseCPUs)]
    openCLDevices = []
    
    # (auto-)configure OpenCL devices
    for i, device in enumerate(openCLDevicesRaw):
        if UseOnlyDeviceNumber is not None and i != UseOnlyDeviceNumber:
            # skip all devices except for the selected one (if there is a selection)
            continue
            
        if OverrideApproximateNumberOfWorkItems is not None:
            device.approximateNumberOfWorkItems=OverrideApproximateNumberOfWorkItems

        # OpenCL native math functions can be faster than their standard
        # equivalents, but have implementation-defined precision. This tends to
        # be sufficient on GPUs whose microarchitectures date from less than a
        # decade ago. X86 single-precision operations, on the other hand, are
        # full of >= 20 years of legacy cruft and can have precision as low as
        # 11 bits, which is enough to miss a DOM at a distance of 20 m. Do not
        # use.
        device.useNativeMath = not device.cpu

        if DoNotParallelize and device.cpu:
            # check if we can split this device into individual cores
            try:
                if device.platform == "Intel(R) OpenCL":
                    # device fission seems to cause serious segfaults in the Intel OpenCL driver.
                    # do not use it.
                    subDevices = []
                else:
                    subDevices = device.SplitDevice()
                
                if len(subDevices) > 0:
                    if OverrideApproximateNumberOfWorkItems is not None:
                        subDevices[0].approximateNumberOfWorkItems=OverrideApproximateNumberOfWorkItems
                    openCLDevices.append(subDevices[0])
                else:
                    logging.log_warn("failed to split CPU device into individual cores %s %s [using full device with minimal number of work-items to (hopefully) disable parallelization]" % (device.platform, device.device), unit="clsim")
                    device.approximateNumberOfWorkItems=1
                    openCLDevices.append(device)
            except:
                logging.log_error("failed to split CPU device into individual cores %s %s [using full device with minimal number of work-items to (hopefully) disable parallelization]" % (device.platform, device.device), unit="clsim")
                device.approximateNumberOfWorkItems=1
                openCLDevices.append(device)
            
        else:
            openCLDevices.append(device)

    if not openCLDevices:
        icetray.logging.log_fatal("No matching OpenCL devices. Devices: {} Selection: UseCPUs={}, UseGPUs={}, UseOnlyDeviceNumber={}. ".format(', '.join([d.device for d in I3CLSimOpenCLDevice.GetAllDevices()]), UseCPUs, UseGPUs, UseOnlyDeviceNumber))
    return openCLDevices

def parseIceModel(IceModelLocation, disableTilt=False):
    from os.path import exists, isdir, isfile, expandvars
    from icecube.clsim.MakeIceCubeMediumProperties import MakeIceCubeMediumProperties
    from icecube.clsim.MakeAntaresMediumProperties import MakeAntaresMediumProperties
    from icecube.clsim.MakeIceCubeMediumPropertiesPhotonics import MakeIceCubeMediumPropertiesPhotonics
    
    if IceModelLocation=="ANTARES":
        return MakeAntaresMediumProperties()
    
    if not exists(IceModelLocation):
        raise RuntimeError("The specified ice model path \"%s\" does not exist" % IceModelLocation)
    
    if isdir(IceModelLocation):
        # it's a PPC ice description directory
        mediumProperties = MakeIceCubeMediumProperties(iceDataDirectory=IceModelLocation, useTiltIfAvailable=not disableTilt)
    elif isfile(IceModelLocation):
        # it's a photonics ice description file
        mediumProperties = MakeIceCubeMediumPropertiesPhotonics(tableFile=IceModelLocation)
    else:
        raise RuntimeError("The specified ice model path \"%s\" is neither a directory nor a file." % IceModelLocation)
    
    return mediumProperties

def setupDetector(GCDFile,
                  SimulateFlashers=False,
                  IceModelLocation=expandvars("$I3_SRC/clsim/resources/ice/spice_mie"),
                  DisableTilt=False,
                  UnWeightedPhotons=False,
                  UnWeightedPhotonsScalingFactor=None,
                  UseI3PropagatorService=True,
                  UseGeant4=False,
                  CrossoverEnergyEM=None,
                  CrossoverEnergyHadron=None,
                  UseCascadeExtension=True,
                  StopDetectedPhotons=True,
                  DOMOversizeFactor=5.,
                  UnshadowedFraction=0.9,
                  HoleIceParameterization=expandvars("$I3_SRC/ice-models/resources/models/angsens/as.h2-50cm"),
                  WavelengthAcceptance=None,
                  DOMRadius=0.16510*icetray.I3Units.m, # 13" diameter
                  IgnoreSubdetectors=['IceTop']):
    """
    Set up data structures used in N different places in clsim
    """
    
    from icecube import clsim
    from icecube.icetray import logging
    from icecube.clsim import GetDefaultParameterizationList
    from icecube.clsim import GetFlasherParameterizationList
    from icecube.clsim import GetHybridParameterizationList
    import numpy
    
    def harvest_detector_parameters(GCDFile):
        
        from icecube import dataio, dataclasses
        from I3Tray import I3Tray
        
        tray = I3Tray()
        tray.Add(dataio.I3Reader, Filenamelist=[GCDFile])
        
        # make sure the geometry is updated to the new granular format
        tray.AddModule("I3GeometryDecomposer",
                       If=lambda frame: ("I3OMGeoMap" not in frame) and ("I3ModuleGeoMap" not in frame))
        
        def pluck_geo(frame):
            pluck_geo.frame = frame
        pluck_geo.frame = None
        tray.Add(pluck_geo, Streams=[icetray.I3Frame.Geometry])
        
        def pluck_calib(frame):
            pluck_calib.frame = frame
        pluck_calib.frame = None
        tray.Add(pluck_calib, Streams=[icetray.I3Frame.Calibration])
        
        tray.Execute()
        
        geometry = clsim.I3CLSimSimpleGeometryFromI3Geometry(
            DOMRadius, DOMOversizeFactor, pluck_geo.frame,
            ignoreSubdetectors=dataclasses.ListString(IgnoreSubdetectors),
            # NB: we trust advanced users to properly label subdetectors, and disable any
            # min/max string/om numbers and strings/oms to ignore
            ignoreStrings=dataclasses.ListInt(), ignoreDomIDs=dataclasses.ListUInt(),
            ignoreStringIDsSmallerThan=1, ignoreStringIDsLargerThan=numpy.iinfo(numpy.int32).max,
            ignoreDomIDsSmallerThan=1, ignoreDomIDsLargerThan=numpy.iinfo(numpy.uint32).max,
            splitIntoPartsAccordingToPosition=False, useHardcodedDeepCoreSubdetector=False
            )
        
        rde = dict()
        max_compensation_factor = -float('inf')
        for k, domcal in pluck_calib.frame['I3Calibration'].dom_cal.iteritems():
            rde[k] = domcal.relative_dom_eff
            max_compensation_factor = max(max_compensation_factor,
                domcal.combined_spe_charge_distribution.compensation_factor)
        if math.isinf(max_compensation_factor):
            logging.log_warn("No SPE charge distribution compensation factor found in {}. Assuming 1.".format(GCDFile), unit="clsim")
            max_compensation_factor = 1

        return geometry, rde, max_compensation_factor
    
    geometry, rde, max_compensation_factor = harvest_detector_parameters(GCDFile)
    
    # ice properties
    if isinstance(IceModelLocation, str):
        mediumProperties = parseIceModel(IceModelLocation, disableTilt=DisableTilt)
    else:
        # get ice model directly if not a string
        mediumProperties = IceModelLocation

    icemodel_efficiency_factor = mediumProperties.efficiency 

    # detector properties
    if WavelengthAcceptance is None:
        # the hole ice acceptance curve peaks at a value different than 1
        peak = numpy.loadtxt(HoleIceParameterization)[0] # The value at which the hole ice acceptance curve peaks
        domEfficiencyCorrection = UnshadowedFraction*peak*max_compensation_factor*icemodel_efficiency_factor
        assert domEfficiencyCorrection > 0
        
        acceptance = {
            'IceCube': clsim.GetIceCubeDOMAcceptance(domRadius = DOMRadius*DOMOversizeFactor, efficiency=domEfficiencyCorrection),
            'DeepCore': clsim.GetIceCubeDOMAcceptance(domRadius = DOMRadius*DOMOversizeFactor, efficiency=domEfficiencyCorrection, highQE=True),
        }
        
        domAcceptanceEnvelope = clsim.I3CLSimFunctionFromTable(acceptance['IceCube'].GetMinWlen(), acceptance['IceCube'].GetWavelengthStepping(), [max((f.GetEntryValue(i) for f in acceptance.values())) for i in range(acceptance['IceCube'].GetNumEntries())])


        domAcceptance = clsim.I3CLSimFunctionMap()
        #loop over every event in the geometry and determine what type of DOM it is
        #based on the relative efficiency in the detector status
        for string_id,dom_id in zip(geometry.stringIDs,geometry.domIDs):
            k = icetray.OMKey(string_id,dom_id,0)
            releff = rde.get(k,float('nan'))
            if releff == 1:
                domAcceptance[k] = acceptance['IceCube']
            elif round(releff, 6) == 1.35:
                domAcceptance[k] = acceptance['DeepCore']
            elif math.isnan(releff):
                #DOMs where rde is nan are bad DOMs which are unplugged
                #call them IceCube for now, hits generated by them will be
                #removed later by detector simulation 
                domAcceptance[k] = acceptance['IceCube']                            
            else:
                raise ValueError("Relative DOM efficiency is neither 1 nor 1.35. You probably need to add support for individual DOM efficiencies") 


    else:
        domAcceptance = WavelengthAcceptance
        domAcceptanceEnvelope = WavelengthAcceptance
    
    angularAcceptance = clsim.GetIceCubeDOMAngularSensitivity(holeIce=HoleIceParameterization)

    # photon generation wavelength bias
    if not UnWeightedPhotons:
        wavelengthGenerationBias = domAcceptanceEnvelope
        if UnWeightedPhotonsScalingFactor is not None:
            raise RuntimeError("UnWeightedPhotonsScalingFactor should not be set when UnWeightedPhotons is not set")
    else:
        if UnWeightedPhotonsScalingFactor is not None:
            logging.log_info("***** running unweighted simulation with a photon pre-scaling of {}".format(UnWeightedPhotonsScalingFactor), unit="clsim")
            wavelengthGenerationBias = clsim.I3CLSimFunctionConstant(UnWeightedPhotonsScalingFactor)
        else:
            wavelengthGenerationBias = None
    
    # create wavelength generators
    wavelengthGenerators = [clsim.makeCherenkovWavelengthGenerator(wavelengthGenerationBias, UnWeightedPhotons, mediumProperties)]
    
    # muon&cascade parameterizations
    ppcConverter = clsim.I3CLSimLightSourceToStepConverterPPC(photonsPerStep=200)
    ppcConverter.SetUseCascadeExtension(UseCascadeExtension)
    if not UseGeant4:
        particleParameterizations = GetDefaultParameterizationList(ppcConverter, muonOnly=False)
    else:
        if CrossoverEnergyEM>0 or CrossoverEnergyHadron>0:
            particleParameterizations = GetHybridParameterizationList(ppcConverter, CrossoverEnergyEM=CrossoverEnergyEM, CrossoverEnergyHadron=CrossoverEnergyHadron)
        else:
            # use no parameterizations except for muons with lengths assigned to them
            # (those are assumed to have been generated by PROPOSAL)
            particleParameterizations = GetDefaultParameterizationList(ppcConverter, muonOnly=True)

    # flasher parameterizations
    if SimulateFlashers:
        # this needs a spectrum table in order to pass spectra to OpenCL
        spectrumTable = clsim.I3CLSimSpectrumTable()
        particleParameterizations += clsim.GetFlasherParameterizationList(spectrumTable)
        for spectrum in spectrumTable:
            if spectrum:
                wavelengthGenerators.append(clsim.makeWavelengthGenerator(spectrum, wavelengthGenerationBias, mediumProperties))
        logging.log_info("number of spectra (1x Cherenkov + Nx flasher): {}".format(len(spectrumTable)))
    else:
        # no spectrum table is necessary when only using the Cherenkov spectrum
        spectrumTable = None
    
    return dict(Geometry=geometry,
                MediumProperties=mediumProperties,
                IceModelLocation=IceModelLocation,
                WavelengthGenerationBias=wavelengthGenerationBias,
                SpectrumTable=spectrumTable,
                GenerateCherenkovPhotonsWithoutDispersion=UnWeightedPhotons,
                WavelengthGenerators=wavelengthGenerators,
                DOMRadius=DOMRadius,
                DOMOversizeFactor=DOMOversizeFactor,
                DOMPancakeFactor=DOMOversizeFactor,
                UnshadowedFraction=UnshadowedFraction,
                AngularAcceptance=angularAcceptance,
                WavelengthAcceptance=domAcceptance,
                ParameterizationList=particleParameterizations,
                UseGeant4=UseGeant4,
                UseI3PropagatorService=UseI3PropagatorService,
                IgnoreSubdetectors=IgnoreSubdetectors,)

def setupPropagators(RandomService,
                     DetectorParams,
                     UseCPUs=False,
                     UseGPUs=True,
                     UseOnlyDeviceNumber=None,
                     DoNotParallelize=False,
                     OverrideApproximateNumberOfWorkItems=None,
                     ):
    """
    Create a collection of photon propagators suitable for use with I3CLSimServer
    """
    from icecube import clsim
    
    geometry = DetectorParams['Geometry']
    mediumProperties = DetectorParams['MediumProperties']
    wavelengthGenerationBias = DetectorParams['WavelengthGenerationBias']
    wavelengthGenerators = DetectorParams['WavelengthGenerators']
    
    openCLDevices = configureOpenCLDevices(
        UseGPUs=UseGPUs,
        UseCPUs=UseCPUs,
        OverrideApproximateNumberOfWorkItems=OverrideApproximateNumberOfWorkItems,
        DoNotParallelize=DoNotParallelize,
        UseOnlyDeviceNumber=UseOnlyDeviceNumber
    )
    
    def create_converter(device):
        return clsim.initializeOpenCL(device, RandomService, geometry,
            mediumProperties, wavelengthGenerationBias, clsim.I3CLSimRandomValuePtrSeries(wavelengthGenerators),
            pancakeFactor=DetectorParams['DOMPancakeFactor'],
            )
        
    return map(create_converter, openCLDevices)
