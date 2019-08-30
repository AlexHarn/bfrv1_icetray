from icecube import icetray, dataclasses

@icetray.traysegment
def simpleSplineLengthReco(tray, name,
                           inputPulses, inputReco,
                           cylinderRadius     = 200,
                           doSimpleOnly       = True,
                           geometry           = 'IC86',
                           AmplitudeTable     = '',
                           MaxRadius          = float('inf'),
                           TiltTableDir       = '',
                           TimingSigma        = 0.0,
                           TimingTable        = '',
                           If                 = lambda frame: True
                       ):

    """
    Simple length reconstruction. The reconstructed vertex and
    stopping point are found by simply projecting back from the first
    and last hit of *inputPulses* on the track *inputReco*. Only hits
    within a cylinder with radius *cylinderRadius* around that track
    are considered. Note that the input pulse map *inputPulses*
    should in most cases have a soft hit cleaning, e.g. classicRT.

    This reconstruction puts three objects into the frame (note that the 
    names always contain the name *name* of the tray segment, to allow
    multiple instances of the segment being run in the same script):

    - *inputReco_Contained_name*: An I3Particle, which has the direction of
      *inputReco*, but the newly reconstructed length. The position is set
      to the reconstructed start point.

    - *inputReco_Contained_name_FiniteCuts*: Some cut values related to the
      length reconstruction.

    - *inputReco_Contained_name_StartStopParams*: The likelihood values for
      a starting, stopping and infinite track. These values are
      supposed to be used as likelihood ratios, i.e. as
      *LLHInf*-*LLHStart* and *LLHInf*-*LLHStop*. All other uses on
      your own risk!
    """

    tray.AddModule('I3StartStopPoint', name+'_VertexReco',
                   Name            = inputReco,
                   InputRecoPulses = inputPulses,
                   ExpectedShape   = 70,            #contained track
                   CylinderRadius  = cylinderRadius,
                   If              = If
                   )

    ic79 = [ 2, 3, 4, 5, 6,
             8, 9, 10, 11, 12, 13,
             15, 16, 17, 18, 19, 20, 21,
             23, 24, 25, 26, 27, 28, 29, 30,
             32, 33, 34, 35, 36, 37, 38, 39, 40,
             41, 42, 43, 44, 45, 46, 47, 48, 49, 50,
             51, 52, 53, 54, 55, 56, 57, 58, 59,
             60, 61, 62, 63, 64, 65, 66, 67,
             68, 69, 70, 71, 72, 73, 74,
             75, 76, 77, 78,
             81, 82, 83, 84, 85, 86 ]

    ic86 = [ 1, 2, 3, 4, 5, 6,
             7, 8, 9, 10, 11, 12, 13,
             14, 15, 16, 17, 18, 19, 20, 21,
             22, 23, 24, 25, 26, 27, 28, 29, 30,
             31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
             41, 42, 43, 44, 45, 46, 47, 48, 49, 50,
             51, 52, 53, 54, 55, 56, 57, 58, 59,
             60, 61, 62, 63, 64, 65, 66, 67,
             68, 69, 70, 71, 72, 73, 74,
             75, 76, 77, 78,
             81, 82, 83, 84, 85, 86,
             79, 80 ]

    tray.AddService('I3PhotoSplineServiceFactory', name+'_PhotoSplineService',
                    AmplitudeTable     = AmplitudeTable,
                    MaxRadius          = MaxRadius,
                    TiltTableDir       = TiltTableDir,
                    TimingSigma        = TimingSigma,
                    TimingTable        = TimingTable,
                    )

    if geometry=='IC86':
        tray.AddService('I3GulliverFinitePhPnhFactory', name+'_finitePhPnh', 
                        InputReadout  = inputPulses,
                        NoiseRate     = 700.0e-9,#GHz
                        SelectStrings = ic86,
                        RCylinder     = cylinderRadius,
                        ProbName      = 'PhPnhPhotorec',
                        PhotorecName  = name+'_PhotoSplineService',
                        )

    elif geometry=='IC79':
        tray.AddService('I3GulliverFinitePhPnhFactory', name+'_finitePhPnh', 
                        InputReadout  = inputPulses,
                        NoiseRate     = 700.0e-9,#GHz
                        SelectStrings = ic79,
                        RCylinder     = cylinderRadius,
                        ProbName      = 'PhPnhPhotorec',
                        PhotorecName  = name+'_PhotoSplineService',
                        )

    if doSimpleOnly:
        tray.AddModule('I3StartStopLProb', name+'_'+inputReco+'_FiniteFit',  # gets + '_StartStopParams'
                       Name        = inputReco+'_Finite',
                       ServiceName = name+'_finitePhPnh',
                       If          = If
                       )

        tray.AddModule('Delete', name+'_deleteFiniteRecoKeys',
                       Keys = [name+'_'+inputReco+'_FiniteFit_rusage',
                               name+'_VertexReco_rusage']
                       )

        tray.AddModule('Rename', name+'_renameFiniteRecoKeys',
                       Keys = [inputReco+'_Finite', inputReco+'_Contained_'+name,
                               inputReco+'_FiniteCuts', inputReco+'_Contained_'+name+'_FiniteCuts',
                               name+'_'+inputReco+'_FiniteFit_StartStopParams', inputReco+'_Contained_'+name+'_StartStopParams']
                       )




@icetray.traysegment
def advancedSplineLengthReco(tray, name,
                             inputPulses, inputReco,
                             cylinderRadius     = 200,
                             geometry           = 'IC86',
                             AmplitudeTable     = '',
                             MaxRadius          = float('inf'),
                             TiltTableDir       = '',
                             TimingSigma        = 0.0,
                             TimingTable        = '',
                             If                 = lambda frame: True
                         ):
    
    """
    Advanced length reconstruction. The reconstructed vertex and
    stopping point are found by a likelihood minimization using
    gulliver. Only hits within a cylinder with radius *cylinderRadius*
    around *inputReco* are considered.  The simple length
    reconstruction is run first, and its output is used for seeding
    the advanced reconstruction. Note that the pulse map
    *inputPulses* should in most cases have a soft hit cleaning,
    e.g. classicRT.

    This reconstruction puts four objects into the frame (note that the 
    names always contain the name *name* of the tray segment, to allow
    multiple instances of the segment being run in the same script):

    - *inputReco_Contained_name*: An I3Particle, which has the direction of
      *inputReco*, but the newly reconstructed length. The position is set 
      to the reconstructed start point.

    - *inputReco_Contained_name_FiniteCuts*: Some cut values related to the
      length reconstruction.

    - *inputReco_Contained_name_StartStopParams*: The likelihood values for
      a starting, stopping and infinite track. These values are supposed to 
      be used as likelihood ratios, i.e. as *LLHInf*-*LLHStart* and 
      *LLHInf*-*LLHStop*. All other uses on your own risk!

    - *inputReco_Contained_name_FitParams*: The gulliver *FitParams* for
      the length reconstruction.
    """

    from icecube import lilliput
    from icecube.icetray import I3Units

    tray.AddSegment(simpleSplineLengthReco, name+'_simpleLengthReco',
                    inputPulses        = inputPulses,
                    inputReco          = inputReco,
                    cylinderRadius     = cylinderRadius,
                    doSimpleOnly       = False,
                    geometry           = geometry,
                    AmplitudeTable     = AmplitudeTable,
                    MaxRadius          = MaxRadius,
                    TiltTableDir       = TiltTableDir,
                    TimingSigma        = TimingSigma,
                    TimingTable        = TimingTable,
                    If                 = If
                    )


    tray.AddService('I3GSLSimplexFactory', name+'_FRsimplex',
                    MaxIterations = 1000,
                    Tolerance     = 0.01,
                    )

    # keep the MC direction and only vary the vertex
    # stop point reco : take start point and variate length
    tray.AddService('I3SimpleParametrizationFactory', name+'_simparStartVertex',
                    StepLinL   = 10.0*I3Units.m,
                    BoundsLinL = [0,2*I3Units.km],
                    VertexMode = 'Start'
                    )
    # seed service using the start position
    tray.AddService('I3BasicSeedServiceFactory', name+'_seedserveStartVertex',
                    FirstGuess    = inputReco+'_Finite',
                    InputReadout  = inputPulses,
                    TimeShiftType = 'TNone'
                    )

    # reconstruct stop point
    tray.AddModule('I3SimpleFitter', name+'_'+inputReco+'_FiniteStop',
                   Parametrization = name+'_simparStartVertex',
                   SeedService     = name+'_seedserveStartVertex',
                   LogLikelihood   = name+'_simpleLengthReco_finitePhPnh',
                   Minimizer       = name+'_FRsimplex',
                   StoragePolicy   = 'OnlyBestFit',
                   OutputName      = name+'_'+inputReco+'_FiniteStop',
                   If              = If
                   )

    # start point reco : take stop point and variate length - this requires setting the vertex mode of the parametrization to "Stop"
    tray.AddService('I3SimpleParametrizationFactory', name+'_simparStopVertex',
                    StepLinL   = 10.0*I3Units.m,
                    BoundsLinL = [0,2*I3Units.km],
                    VertexMode = 'Stop'
                    )
    # seed service using the stop position -> set option + USE RESULTS OF STOP RECO HERE
    tray.AddService('I3BasicSeedServiceFactory', name+'_seedserveStopVertex',
                    FirstGuess    = name+'_'+inputReco+'_FiniteStop',
                    InputReadout  = inputPulses,
                    TimeShiftType = 'TNone'
                    )
    # reconstruct start point
    tray.AddModule('I3SimpleFitter', name+'_'+inputReco+'_Contained',
                   Parametrization = name+'_simparStopVertex',
                   SeedService     = name+'_seedserveStopVertex',
                   LogLikelihood   = name+'_simpleLengthReco_finitePhPnh',
                   Minimizer       = name+'_FRsimplex',
                   StoragePolicy   = 'OnlyBestFit',
                   OutputName      = name+'_'+inputReco+'_Contained',
                   If              = If
                   )


    # now we have a reconstructed contained track, get the start stop params
    tray.AddModule('I3StartStopLProb', name+'_'+inputReco+'_ContainedFit',  # gets + '_StartStopParams'
                   Name        = name+'_'+inputReco+'_Contained', 
                   ServiceName = name+'_simpleLengthReco_finitePhPnh',
                   If          = If
                   )

    # clean up
    tray.AddModule('Delete', name +'deleteFiniteRecoKeys',
                   Keys = [inputReco+'_Finite', name+'_'+inputReco+'_FiniteStop', name+'_'+inputReco+'_FiniteStopFitParams', 
                           name+'_'+inputReco+'_ContainedFit_rusage', name+'_simpleLengthReco_VertexReco_rusage']
                   )

    tray.AddModule('Rename', name +'renameFiniteRecoKeys',
                   Keys = [name+'_'+inputReco+'_Contained', inputReco+'_Contained_'+name,
                           name+'_'+inputReco+'_ContainedFitParams', inputReco+'_Contained_'+name+'_FitParams',
                           name+'_'+inputReco+'_ContainedFit_StartStopParams', inputReco+'_Contained_'+name+'_StartStopParams',
                           inputReco+'_FiniteCuts', inputReco+'_Contained_'+name+'_FiniteCuts']
                   )
