#!/usr/bin/env python

from I3Tray import *
from icecube import dataio,dataclasses
load("libtruncated_energy")

tabledir = '/cvmfs/icecube.opensciencegrid.org/data/photon-tables/SPICEMie/'
driverdir = "driverfiles"
driverfile = "mu_photorec.list"

if not os.path.exists(tabledir):
    print("Tables directory not found (no cvmfs)")
    sys.exit()

testdir = os.environ["I3_TESTDATA"]
files = ["GeoCalibDetectorStatus_2012.56063_V0.i3.gz",
         "Level3_nugen_numu_IC86.2012.011069.000000_20events.i3.bz2"]
filelist = [ os.path.join(testdir,"sim",f) for f in files]

driverdir = os.path.join(tabledir,driverdir)

tray = I3Tray()

tray.AddModule("I3Reader","reader",
               FileNameList  = filelist
               )

tray.AddService( "I3PhotonicsServiceFactory", "PhotonicsServiceMu_SpiceMie",
                 PhotonicsTopLevelDirectory=tabledir,
                 DriverFileDirectory=driverdir,
                 PhotonicsLevel2DriverFile=driverfile,
                 PhotonicsTableSelection=2,
                 ServiceName="PhotonicsServiceMu_SpiceMie")

tray.AddModule("I3TruncatedEnergy",
               RecoPulsesName = 'TWSRTHVInIcePulses',
               RecoParticleName = "SplineMPE",
               ResultParticleName = "SplineMPETruncatedEnergy2_SPICEMie",
               I3PhotonicsServiceName = "PhotonicsServiceMu_SpiceMie",
               UseRDE = True,
               )

def test(frame):
    for key in frame.keys():
        if key.startswith("SplineMPETruncatedEnergy_"):
            f1= frame[key]
            f2=frame[key.replace("SplineMPETruncatedEnergy_","SplineMPETruncatedEnergy2_")]
            attr = 'energy' if isinstance(f1, dataclasses.I3Particle) else 'value'
            energy1 = getattr(f1, attr)
            energy2 = getattr(f2, attr)
            d = abs(energy1 - energy2)
            nd = d/(energy1 + energy2) if (energy1 + energy2) > 0 else 0.
            print("%s difference = %f normalized difference = %0.10f" % (key, d, nd))
            assert(nd < 1e-6)

tray.AddModule(test)

tray.Execute()

