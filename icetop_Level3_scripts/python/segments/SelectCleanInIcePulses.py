#!/usr/bin/env python                                                                                                                                                                                     
from icecube import icetray
@icetray.traysegment
def SelectCleanInIcePulses(tray,name,
                           Detector,
                           CoincPulses='CoincPulses',
                           CleanCoincPulses='CleanCoincPulses',
                           IceTopTrack='Laputop',
                           isMC=False,
                           If=lambda fr: True
                           ):
    # This segment actually does some preparations for the reconstruction of the energy loss of the muon bundle with millipede. 
    # It cleans the inice pulses according to the inice track, and outputs NCh_+CleanCoincPulses, which is a boolean to see whether the cleaned pulses contain at least 8 DOMs.
    # It furthermore handles time windows and calibration errata.
    
    from icecube import icetop_Level3_scripts, dataclasses, WaveCalibrator, wavedeform
    # This part is a little bit tricky... since we can't do completely the same in data and MC.
    # The SaturationWindows were not saved in IC79 and IC86.2011, thus they need to be recreated.
    # This is what we will do.
    # In IC79 simulation however, we used DOMSimulator, which requires some corrections to wavecalibrator. This is however not possible with the new software.
    # Therefore, we'll approach this error as that all DOMs which would have saturated waveforms would be in CalibrationErrata, 
    # which would be the case most of the time since it's difficult to calibrate those. (Had some slack discussion with Jvs about this.)
    # Thus for IC79 MC we will do nothing here. 
    if isMC:
        if Detector=="IC86.2011":   
            tray.AddModule('I3WaveCalibrator', name + '_wavecal',
                            Launches='InIceRawData',
                            Waveforms='CalibratedWaveforms',
                            FADCSaturationMargin=1,
                            Errata='CalibrationErrata2')
            tray.AddModule('I3PMTSaturationFlagger', 'flagit',
                           Output='SaturationWindows', Waveforms='CalibratedWaveforms') #Current Threshold : default ( 50 mA)        
        
    else:
        if Detector=="IC79" or Detector=="IC86.2011":
            tray.AddModule('I3WaveCalibrator', name + '_wavecal',
                           Launches='InIceRawData',
                           Waveforms='CalibratedWaveforms',
                           FADCSaturationMargin=1,
                           Errata='CalibrationErrata2')
            
            tray.AddModule('I3PMTSaturationFlagger', 'flagit',
                           Output='SaturationWindows', Waveforms='CalibratedWaveforms') #Current Threshold : default ( 50 mA)        
            
        # In IC79 (and IC86.2011) times, the CalibrationErrate where a list of OMKeys, while in later years they are timewindows. 
        # Here, we will try to do something as similar as possible to the IC79 simulation (since this could be a big effect). 
        # We'll thus convert the timewindows to an OMKey list.
        else:
            tray.AddModule('Rename',name+'_calibErrName',
                           Keys = ['CalibrationErrata','CalibrationErrata_IC86'])
            
            def FixCalibErrata(frame):
                if 'CalibrationErrata_IC86' in frame:
                    new_calib_err = dataclasses.I3VectorOMKey()
                    calib_err = frame['CalibrationErrata_IC86']
                    for omk,tws in calib_err:
                        new_calib_err.append(omk)
                    frame['CalibrationErrata'] = new_calib_err
                return True
            
            tray.AddModule(FixCalibErrata,name+'_fixCalibErr',
                           Streams = [icetray.I3Frame.DAQ]
                           )
    
    # Module to further remove random coincidences, and noise. Here the track reconstructed by IceTop is used.
    tray.AddModule(icetop_Level3_scripts.modules.Early_cleaning_InIce,name+'_cleanit',
                   trackName=IceTopTrack,
                   inputpulseName=CoincPulses,
                   outputpulseName=CleanCoincPulses,
                   max_radius=500,  ## optimized and safe, see Radius_depth.C                                                                                                                               
                   min_time=-400,   ## optimized, see tres.eps
                   If=If
                   )
    
    #Always execute this module. Will put a false if pulses are not there.
    tray.AddModule(icetop_Level3_scripts.modules.SelectChannels_InIce,name+'_nchcut',
                   inputiipulses = CleanCoincPulses,
                   nchthreshold = 8,
                   outputboolname ='NCh_' +CleanCoincPulses,
                   removeEvents = False)

    # Find the balloon doms to put in an excluded list.
    tray.AddModule(icetop_Level3_scripts.modules.DOMselection_InIce,name+'_cutIt',
                   ICpulses = CleanCoincPulses,
                   OutputName = CleanCoincPulses+'_Balloon',
                   Ratio = 0.3,         # from data/MC disagreement (photonics issue)                                                                                                                       
                   If= lambda fr: "NCh_"+CleanCoincPulses in fr and fr['NCh_' + CleanCoincPulses].value)
    
    # Bugfix the readout window calc because a TW cleaning has been applied                                                                                                                
    # crucial for correct noise calculation in millipede, for both MC as all data!                                                                                                                          
    # MUST come after the NCh check.
    # Outputs 
    tray.AddModule(icetop_Level3_scripts.modules.AddReadoutTimeWindow, name+'_pulserange',
                   Pulses=CleanCoincPulses,
                   If=lambda fr: "NCh_"+CleanCoincPulses in fr and fr['NCh_' + CleanCoincPulses].value)
  
                   

