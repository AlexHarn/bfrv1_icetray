#
#   Usage: python credoL2Example.py
#
#   Description:  credo is a photonics based reconstruction implemented in the 
#       Gulliver framework.  It incorporates an ice model timing information 
#       in order to attempt to reconstruct the event direction.  This icetray 
#       script make an attempt to demonstrate how the loglikelihood is
#       calculated for a real event where a hypothetical event is constructed
#       and used as a template to compare the real event against.

from I3Tray import *
from icecube import icetray, dataclasses, dataio, credo, gulliver, photonics_service, lilliput, DomTools, gulliver_modules, cscd_llh, phys_services
from icecube.credo import particle_forge

#   Input files
gcdfile = ["/data/sim/IceCube/2012/filtered/level2/neutrino-generator/11068/00000-00999/GeoCalibDetectorStatus_2012.56063_V1.i3.gz"]
infile  = ["/data/sim/IceCube/2012/filtered/level2/neutrino-generator/11068/00000-00999/Level2_nugen_nue_IC86.2012.011068.000972.i3.bz2"]

files = gcdfile + infile 

#   Create a tray
tray = I3Tray( )

#   Read files
tray.AddModule( "I3Reader", "reader", FileNameList = files ) 

#   Check the filter mask object to see if event passes the Cascade Filter
#   Every time a module returns a 0 (false) to the tray, the event is skipped
#   over in following modules.
def cascadeFrame( frame ):
    if frame.Stop == icetray.I3Frame.Physics and frame.Has('FilterMask'):
        if frame['FilterMask'].get('CascadeFilter_12').condition_passed:
            return 1
        else:
            return 0
    else:
        return 0

tray.AddModule( cascadeFrame, "checkframe" )

#   Photonics service factory should be made available for some modules and
#   other services's needs.
tray.AddService( "I3PhotonicsServiceFactory", "I3PhotonicsService",
    PhotonicsTopLevelDirectory =
        "/data/sim/sim-new/PhotonTablesProduction/AHA07v1ice/",
    DriverFileDirectory =
        "/data/sim/sim-new/PhotonTablesProduction/AHA07v1ice/driverfiles/",
    PhotonicsLevel2DriverFile = "cscd_photorec.list",
    UseDummyService = False,
    PhotonicsTableSelection = 2 )

#   This sets up loglikelihood factory with photonics service as PDF.
tray.AddService( "I3PoissonGaussLogLikelihoodFactory", "likelihood",
    InputPulses             = "InIcePulses",
    NoiseRate               = 480. * I3Units.hertz,
    PDF                     = "I3PhotonicsService",
    EventLength             = -1,
    GaussianErrorConstant   = 1000.,
    UseBaseContributions    = False,
    MinChargeFraction       = 0.,
    SaturationLimitIceCube  = 10000.,
    PhotonicsSaturation     = True,
    BadDOMListInFrame       = "",
    BadDOMList              = [],
    ATWDOnly                = False,
    UseEmptyPulses          = False )

#   This is one of the standard cascade event vertex fitters.
tray.AddModule( "I3CscdLlhModule", "CscdL3_CascadeLlhVertexFit",
    InputType       = "RecoPulse",
    RecoSeries      = "InIcePulses",
    FirstLE         = True,
    SeedWithOrigin  = False,
    SeedKey         = "CascadeLlhVertexFit_L2",
    MinHits         = 3,
    AmpWeightPower  = 0.,
    ResultName      = "CscdL3_CascadeLlhVertexFit",
    Minimizer       = "Powell",
    PDF             = "UPandel",
    ParamT          = "1., 0., 0., false",
    ParamX          = "1., 0., 0., false",
    ParamY          = "1., 0., 0., false",
    ParamZ          = "1., 0., 0., false" )

#   This module creates a "franken-I3Particle".  Every particle object must
#   have direction, vertex position, energy, etc.  This particle is set to have
#   cascade shape, the time and position are copied from the 
#   CascdL3_CascadeLlhVertexFit module above, direction from SPEFit2, etc.
#   Note: The particle status must be set to OK in order for the subsequent 
#   fitters to use the particle when specified.
tray.AddModule( particle_forge.I3ParticleForgeModule, "mcforge1",
    Shape       = dataclasses.I3Particle.ParticleShape.Cascade,
    Time        = "CscdL3_CascadeLlhVertexFit",
    Position    = "CscdL3_CascadeLlhVertexFit",
    Direction   = "SPEFit2",
    Energy      = "AtmCscdEnergyReco_L2",
    Type        = dataclasses.I3Particle.ParticleType.unknown,
    Status      = dataclasses.I3Particle.OK,
    Output      = "CascadeSeed" )


#   This parametrization factory is needed by Gulliver reconstruction.  It 
#   defines an event with the parameters below.  By setting the upper and lower
#   event values to by the same, the parametarization can be used for event
#   hypothesis.
tray.AddService("I3SimpleParametrizationFactory","SimpleCascade",
                    BoundsAzimuth                              = [0.0, 0.0], # Default                          
                    BoundsLinE                                 = [0.0, 0.0], # Default                          
                    BoundsLinL                                 = [0.0, 0.0], # Default                          
                    BoundsLogE                                 = [0.0, 0.0], # Default                          
                    BoundsLogL                                 = [0.0, 0.0], # Default                          
                    BoundsT                                    = [0.0, 0.0], # Default                          
                    BoundsX    =      [-1200.0*I3Units.m,+1200.0*I3Units.m],  # ! Change 
                    BoundsY    =      [-1200.0*I3Units.m,+1200.0*I3Units.m],  # ! Change 
                    BoundsZ    =      [-1200.0*I3Units.m,+1200.0*I3Units.m],  # ! Change 
                    BoundsZenith                               = [0.0, 0.0], # Default                            
                    ParticleTrace                                   = False, # Default                            
                    RelativeBoundsAzimuth                      = [0.0, 0.0], # Default                  
                    RelativeBoundsLinE                         = [0.0, 0.0], # Default                  
                    RelativeBoundsLinL                         = [0.0, 0.0], # Default                  
                    RelativeBoundsLogE                         = [0.0, 0.0], # Default                  
                    RelativeBoundsLogL                         = [0.0, 0.0], # Default                  
                    RelativeBoundsT                            = [0.0, 0.0], # Default                  
                    RelativeBoundsX                            = [0.0, 0.0], # Default                  
                    RelativeBoundsY                            = [0.0, 0.0], # Default                  
                    RelativeBoundsZ                            = [0.0, 0.0], # Default                  
                    RelativeBoundsZenith                       = [0.0, 0.0], # Default                  
                    StepAzimuth                      = 0.1 * I3Units.radian,  # ! Change                 
                    StepLinE                                          = 0.0, # Default                                
                    StepLinL                                          = 0.0, # Default                                
                    StepLogE                                          = 1.0,  # ! Change                               
                    StepLogL                                          = 0.0, # Default                                
                    StepT                               = 20.0 * I3Units.ns,  # ! Change                             
                    StepX                                = 40.0 * I3Units.m,  # ! Change                             
                    StepY                                = 40.0 * I3Units.m,  # ! Change                              
                    StepZ                                = 40.0 * I3Units.m,  # ! Change                             
                    StepZenith                       = 0.1 * I3Units.radian,  # ! Change                        
                    VertexMode                                         = '', # Default                                       
                    )

#   This sets up the seed service Gulliver needs to do a loglikelihood
#   calculation.  We will test how cascade like an event is.  To do so we need
#   to specify a cascade event's characteristics, which we did with the
#   I3ParticleForge.
tray.AddService( "I3BasicSeedServiceFactory", "SeedCascade",
                     AddAlternatives=            "None",
                     AltTimeShiftType=         "TFirst",
                     ChargeFraction=                0.9,
                     FirstGuess=                     "",
                     FirstGuesses=      ["CascadeSeed"],
                     FixedEnergy=                   NaN,
                     InputReadout=               "InIcePulses",
                     MaxMeanTimeResidual=        1000.0,
                     NChEnergyGuessPolynomial=       [],
                     OnlyAlternatives=            False,
                     PositionShiftType=              "",
                     SpeedPolice=                  True,
                     TimeShiftType=             "TNone" )

#   Set up minuit minimizer service.
tray.AddService( "I3GulliverMinuitFactory", "Minuit",
                     Algorithm= "SIMPLEX",                                  # Default
                     Tolerance= 0.01,                                       # ! change to 0.01
                     MaxIterations= 10000,                                  # Default
                     MinuitPrintLevel= -2,                                  # Default
                     MinuitStrategy= 2,                                     # Default
                     FlatnessCheck= True,                                   # Default
                     )

#   Use the Gulliver loglikelihood fitter to determine how cascade like an event
#   is compared to the franken-I3Particle cascade event we created and used 
#   as seed for Gulliver in I3BasicSeedServiceFactory
tray.AddModule( "I3SimpleFitter", 
    OutputName = "llhcalc",
    LogLikelihood   = "likelihood",
    Minimizer       = "Minuit",        
    NonStdName      = "Params",               
    Parametrization = "SimpleCascade",               
    SeedService     = "SeedCascade",            
    StoragePolicy   = "OnlyBestFit" )

#   Specify which objects to save to file.
keep_keys = ["llhcalc", "CscdL3_CascadeLlhVertexFit", "CascadeSeed", "LineFit"]

tray.AddModule( "Keep", "KeepKeys", keys = keep_keys )
tray.AddModule( "I3Writer", "Writer", FileName = "test.i3.bz2" )


tray.Execute( 10 )

