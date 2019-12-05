#!/usr/bin/env python                                                                                                                                                         
from icecube import icetray,dataio, clsim , dataclasses,phys_services
import sys, os
from I3Tray import *
from icecube.simprod.util import *
import icecube.simclasses
from icecube.clsim.shadow import *
from icecube.simclasses import I3CylinderMap
from os.path import expandvars

cable_map = I3CylinderMap()

seed = 10

randomService = phys_services.I3SPRNGRandomService(
    seed = 1,
    nstreams = 100000,
    streamnum = seed)


gcd_file = os.getenv('I3_TESTDATA') + '/GCD/GeoCalibDetectorStatus_IC86.55697_corrected_V2.i3.gz'

class generateEvent(icetray.I3Module):
    def __init__(self, context):
        icetray.I3Module.__init__(self, context)
        self.AddParameter("I3RandomService", "the service", None)
        self.AddParameter("Type", "", dataclasses.I3Particle.ParticleType.EMinus)
        self.AddParameter("Energy", "", 10.*I3Units.TeV)
        self.AddParameter("NEvents", "", 1)
        self.AddParameter("XCoord", "", 0.)
        self.AddParameter("YCoord", "", 0.)
        self.AddParameter("ZCoord", "", 0.)
        self.AddParameter("Primary_direction","",dataclasses.I3Direction(0.,0.,-1.))
        self.AddParameter("Daughter_direction","",dataclasses.I3Direction(0.,0.,-1.))
        self.AddOutBox("OutBox")

    def Configure(self):
        self.rs = self.GetParameter("I3RandomService")
        self.particleType = self.GetParameter("Type")
        self.energy = self.GetParameter("Energy")
        self.nEvents = self.GetParameter("NEvents")
        self.xCoord = self.GetParameter("XCoord")
        self.yCoord = self.GetParameter("YCoord")
        self.zCoord = self.GetParameter("ZCoord")
        self.primary_direction = self.GetParameter("Primary_direction")
        self.daughter_direction = self.GetParameter("Daughter_direction")

        self.eventCounter = 0

    def DAQ(self, frame):
        daughter = dataclasses.I3Particle()
        daughter.type = self.particleType
        daughter.energy = self.energy
        daughter.pos = dataclasses.I3Position(self.xCoord,self.yCoord,self.zCoord)
        daughter.dir = self.daughter_direction
        daughter.time = 0.
        daughter.location_type = dataclasses.I3Particle.LocationType.InIce

        primary = dataclasses.I3Particle()
        primary.type = dataclasses.I3Particle.ParticleType.NuE
        primary.energy = self.energy
        primary.pos = dataclasses.I3Position(self.xCoord,self.yCoord,self.zCoord)
        primary.dir = self.primary_direction
        primary.time = 0.
        primary.location_type = dataclasses.I3Particle.LocationType.Anywhere

        mctree = dataclasses.I3MCTree()
        mctree.add_primary(primary)
        mctree.append_child(primary,daughter)

        frame["I3MCTree"] = mctree

        self.PushFrame(frame)

        self.eventCounter += 1
        if self.eventCounter==self.nEvents:
            self.RequestSuspension()

if __name__ == "__main__":

    tray = I3Tray()

    tray.AddModule("I3InfiniteSource" , "streams" , Prefix=expandvars(gcd_file),Stream = icetray.I3Frame.DAQ)

    tray.Add(AddCylinders , Cable_map = cable_map ,Length_of_cylinder = 17.0, Radius_of_cylinder = 0.023)

    tray.Add("Dump")

    tray.AddModule("I3MCEventHeaderGenerator","gen_header",
                   Year = 2009,
                   DAQTime=158100000000000000,
                   RunNumber=1,
                   EventID=1,
                   IncrementEventID=True)

    tray.AddModule(generateEvent , "generateEvent",
                   Type = dataclasses.I3Particle.ParticleType.MuMinus,
                   NEvents = 1,
                   XCoord = -256.14,
                   YCoord = -521.08,
                   ZCoord = 496.03,
                   Primary_direction = dataclasses.I3Direction(0 , 0 ,-1),
                   Daughter_direction = dataclasses.I3Direction(0 , 0 , -1),
                   I3RandomService = randomService,
                   Energy = 100.0*I3Units.TeV )

    photonSeriesName = "Photons"
    MCTreeName = "I3MCTree"


    tray.AddSegment(clsim.I3CLSimMakePhotons,"MakePhotons",
                    UseGPUs = False,
                    UseCPUs = True,
                    PhotonSeriesName = photonSeriesName,
                    MCTreeName = MCTreeName,
                    RandomService = randomService,
                    IceModelLocation = expandvars("$I3_BUILD/ice-models/resources/models/spice_lea"),
                    GCDFile = gcd_file)


    tray.AddModule("I3ShadowedPhotonRemoverModule",
                   "PhotonRemover",
                   InputPhotonSeriesMapName = "Photons",
                   OutputPhotonSeriesMapName = "ShadowedPhotons",
                   Cable_Map = "CableMap",
                   Distance = 125.0)


    tray.AddModule("I3NullSplitter","physics")

    import unittest
    class SanityCheck(unittest.TestCase):
        photons = 'Photons'
        shadowed_photons = 'ShadowedPhotons'


        def testKeys(self):
            self.assert_(self.shadowed_photons in self.frame, "The shadowed_photons actually shows up in the frame.")

        def testEnergy(self):
            non_shadowed = self.frame[self.photons]
            count_photons = 0

            for i in non_shadowed.values():
                count_photons += len(i)
            
            shadowed = self.frame[self.shadowed_photons]
            count_shadowed = 0
            for i in shadowed.values():
                count_shadowed+= len(i)
            print(count_photons-count_shadowed)
            self.assert_((count_photons - count_shadowed) > 0, "Photons removed")

    tray.AddModule('Dump', 'dump')
    tray.AddModule(icetray.I3TestModuleFactory(SanityCheck), 'testy')


    tray.Execute()

