#!/usr/bin/env python
"""
Script to convert RAW i3 files into SDST i3 files.
"""

def run(gcdfile, infiles, outfile, num=-1):
    """
    Convert raw input files into a SDST output file.

    Note: Can concatenate an entire run of PFRaw files here but do NOT combine
    multiple runs!

    Args:
        gcdfile (str): filename for GCD file
        infiles (list): list of input filenames
        outfile (str): filename to write out to
        num (int): number of frames to process (default: all)
    """
    from icecube import icetray, dataclasses, dataio, WaveCalibrator, wavedeform, payload_parsing
    from I3Tray import I3Tray
    from icecube.icetray import OMKey

    from icecube.filterscripts.baseproc_daqtrimmer import DAQTrimmer
    from icecube.filterscripts.baseproc_superdst import SuperDST, MaskMaker
    from icecube.filterscripts.baseproc_onlinecalibration import OnlineCalibration, DOMCleaning
    from icecube.filterscripts import filter_globals

    tray = I3Tray()

    files = [gcdfile]+infiles
    tray.Add(dataio.I3Reader, "reader", filenamelist=files)

    # shim the D frame with IceTop Bad Doms.
    def shim_bad_icetopdoms(frame):
        if filter_globals.IcetopBadTanks not in frame:
            frame[filter_globals.IcetopBadTanks] = dataclasses.I3VectorTankKey()
        if filter_globals.IceTopBadDoms not in frame:
            frame[filter_globals.IceTopBadDoms] = dataclasses.I3VectorOMKey()

    tray.AddModule(shim_bad_icetopdoms, 'Base_shim_icetopbads',
                  Streams=[icetray.I3Frame.DetectorStatus])

    tray.AddModule('QConverter', WritePFrame=False)

    tray.AddSegment(
        payload_parsing.I3DOMLaunchExtractor, "decode",
        MinBiasID = 'MinBias',
        FlasherDataID = 'Flasher',
        CPUDataID = "BeaconHits",
        SpecialDataID = "SpecialHits",
        SpecialDataOMs = [OMKey(0,1),
                          OMKey(12,65),
                          OMKey(12,66),
                          OMKey(62,65),
                          OMKey(62,66)]
    )

    tray.AddSegment(DOMCleaning, "_DOMCleaning")

    tray.AddSegment(OnlineCalibration, 'Calibration', 
                    simulation=False, WavedeformSPECorrections=True,
                    Harvesting=filter_globals.do_harvesting)
    tray.AddSegment(SuperDST, 'SuperDST',
        InIcePulses=filter_globals.UncleanedInIcePulses, 
        IceTopPulses='IceTopPulses',
        Output = filter_globals.DSTPulses
    )

    # Set up masks for normal access
    tray.AddModule(MaskMaker, '_superdst_aliases',
                   Output=filter_globals.DSTPulses,
                   Streams=[icetray.I3Frame.DAQ]
    )

    tray.AddSegment(DAQTrimmer, 'DAQTrimmer')

    tray.AddModule('Rename', Keys=['UncleanedInIcePulsesTimeRange', 'I3SuperDSTTimeRange'])

    tray.AddModule('Delete', Keys=[
        'CalibratedWaveforms',
        'CalibratedWaveformRange',
        'DrivingTime',
        'I3DAQData',
        'I3EventHeader_orig',
        'OfflinePulses',
        'I3TriggerHierarchy',
        'DAQTrimmer_CleanRawData',
        'DAQTrimmer_I3SuperDST_CalibratedWaveforms_Chi',
        'DAQTrimmer_HighCharge',
        'DAQTrimmer_TrivialLaunches',
        'DAQTrimmer_TrivialLaunchesKeys',
        'CalibratedIceTopATWD_HLC',
        'CalibratedIceTopATWD_SLC',
        'CalibratedIceTopFADC_HLC',
        'CleanIceTopRawData',
        'CleanInIceRawData',
        'DAQTrimmer_I3SuperDST_CalibratedWaveforms_Borked',
        'IceTopCalibratedWaveforms',
        'IceTopDSTPulses',
        'IceTopHLCPulseInfo',
        'IceTopHLCVEMPulses',
        'IceTopPulses',
        'IceTopPulses_HLC',
        'IceTopPulses_SLC',
        'IceTopSLCVEMPulses',
        'InIceDSTPulses',
        'JEBEventInfo',
        'RawDSTPulses',
        'UncleanedInIcePulses',
    ])

    # Needed features: IceTop data, seatbelts
    def FailedEHE(frame):
        return not ("QFilterMask" in frame and "EHEFilter_12" in frame["QFilterMask"]
                    and frame["QFilterMask"]["EHEFilter_12"].condition_passed) 
    tray.AddModule("Delete", Keys=["InIceRawData"], If=FailedEHE)

    tray.AddModule('I3Writer', Filename=outfile,
        Streams=[icetray.I3Frame.TrayInfo, icetray.I3Frame.DAQ, icetray.I3Frame.Physics]
    )

    # make it go
    if num >= 0:
        tray.Execute(num)
    else:
        tray.Execute()

    print("Done")

def main():
    import argparse
    parser = argparse.ArgumentParser(description='RAW -> SDST processing')
    parser.add_argument('-g', '--gcdfile', type=str, required=True,
                        help='GCD filename')
    parser.add_argument('-i', '--infiles', type=str, action='append', required=True,
                        help='Input i3 filenames')
    parser.add_argument('-o', '--outfile', type=str, required=True,
                        help='Output i3 filename')
    parser.add_argument('-n','--num', default=-1, type=int,
                        help='Number of frames to process (default: all)')
    args = parser.parse_args()

    run(**vars(args))

if __name__ == "__main__":
    main()
