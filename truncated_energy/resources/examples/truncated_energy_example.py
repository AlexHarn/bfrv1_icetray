import sys
from I3Tray import *
from icecube import dataio, tableio, phys_services
from icecube import icetray, dataclasses, simclasses
#from icecube import truncated_energy

load( "libtruncated_energy" )

#   Input files
gcdfile = ["/cvmfs/icecube.opensciencegrid.org/standard/RHEL_6_x86_64/i3ports/test-data/sim/GeoCalibDetectorStatus_2013.56429_V1.i3.gz"]
infile  = ["/cvmfs/icecube.opensciencegrid.org/standard/RHEL_6_x86_64/i3ports/test-data/sim/Level2_IC86.2011_corsika.010281.001664.00.i3.bz2"]
 
files = gcdfile + infile

tray = I3Tray( )

tray.AddModule( "I3Reader", "reader", FileNameList = files ) 

### Truncated Energy ###

# Register a tray segment configuration function with icetray.
# The segment can then be added to a tray using I3Tray.AddSegment().
@icetray.traysegment

def Truncated( tray, Name, Pulses = "", Seed = "", Suffix = "",
    If = lambda f: True, PhotonicsService = "", Model = "" ):
    # Base name to put into frame
    TruncatedName = Seed + "TruncatedEnergy" + Suffix + Model 

    tray.AddModule( "I3TruncatedEnergy", 
        #Name of pulses to grab from frame.
        RecoPulsesName          = Pulses,           
        #Name of the reconstructed particle to use.
        RecoParticleName        = Seed,
        #Name of the result particle to put in frame.
        ResultParticleName      = TruncatedName,   
        #Photonics service to use for energy estimator.
        I3PhotonicsServiceName  = PhotonicsService, 
        #Calibration info for Relative DOM Efficiency (RDE).  For HQE DOMs.
        UseRDE                  = True,             
        If                      = If )


# Service that I3TruncatedEnergy needs to talk to for energy estimation.
# This service creates a link for tables.
tray.AddService( "I3PhotonicsServiceFactory", "PhotonicsServiceMu",
    PhotonicsTopLevelDirectory  ="/cvmfs/icecube.opensciencegrid.org/data/photon-tables/SPICEMie/",
    DriverFileDirectory         ="/cvmfs/icecube.opensciencegrid.org/data/photon-tables/SPICEMie/driverfiles",
    PhotonicsLevel2DriverFile   = "mu_photorec.list", 
    PhotonicsTableSelection     = 2,
    ServiceName                 = "PhotonicsServiceMu" )


# Add I3TruncatedEnergy.
# For Seed Trine used S0MSPEFit4.  I am going to use what the level 3 
# muon filter reconstruction used, which is: SplineMPE
tray.AddSegment( Truncated, Pulses = "TWSRTOfflinePulses", Seed = "SPEFit2", 
    Suffix = "Test", PhotonicsService = "PhotonicsServiceMu",
    Model = "_SPICEMie" )  


# Output to save.
keep_keys = [   "SPEFit2TruncatedEnergyTest_SPICEMie_AllBINS_MuEres",
                "SPEFit2TruncatedEnergyTest_SPICEMie_AllBINS_Muon",
                "SPEFit2TruncatedEnergyTest_SPICEMie_AllBINS_Neutrino",
                "SPEFit2TruncatedEnergyTest_SPICEMie_AllBINS_dEdX",
                "SPEFit2TruncatedEnergyTest_SPICEMie_AllDOMS_MuEres",
                "SPEFit2TruncatedEnergyTest_SPICEMie_AllDOMS_Muon",
                "SPEFit2TruncatedEnergyTest_SPICEMie_AllDOMS_Neutrino",
                "SPEFit2TruncatedEnergyTest_SPICEMie_AllDOMS_dEdX",
                "SPEFit2TruncatedEnergyTest_SPICEMie_BINS_MuEres",
                "SPEFit2TruncatedEnergyTest_SPICEMie_BINS_Muon",
                "SPEFit2TruncatedEnergyTest_SPICEMie_BINS_Neutrino",
                "SPEFit2TruncatedEnergyTest_SPICEMie_BINS_dEdX",
		"SPEFit2TruncatedEnergyTest_SPICEMie_BINS_dEdxVector",
                "SPEFit2TruncatedEnergyTest_SPICEMie_DOMS_MuEres",
                "SPEFit2TruncatedEnergyTest_SPICEMie_DOMS_Muon",
                "SPEFit2TruncatedEnergyTest_SPICEMie_DOMS_Neutrino",
                "SPEFit2TruncatedEnergyTest_SPICEMie_DOMS_dEdX",
                "SPEFit2TruncatedEnergyTest_SPICEMie_ORIG_Muon",
                "SPEFit2TruncatedEnergyTest_SPICEMie_ORIG_Neutrino",
                "SPEFit2TruncatedEnergyTest_SPICEMie_ORIG_dEdX" ]

tray.AddModule( "Keep", "KeepKeys", keys = keep_keys )
tray.AddModule( "I3Writer", "Writer", FileName = "truncatedE_test.i3.bz2" )


tray.Execute( 100 )

