#!/usr/bin/env python                                                                                                                                                         
from icecube import icetray,dataio, clsim , dataclasses,phys_services
import sys, os
from I3Tray import *
from icecube.simprod.util import *
import icecube.simclasses
from os.path import expandvars

randomService = phys_services.I3GSLRandomService(seed = 1)

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
    icetray.logging.set_level_for_unit("testCableShadow", "INFO")

    tray = I3Tray()

    tray.AddModule("I3InfiniteSource" , "streams" , Prefix=expandvars(gcd_file),Stream = icetray.I3Frame.DAQ)

    tray.AddModule("I3MCEventHeaderGenerator","gen_header",
                   Year = 2009,
                   DAQTime=158100000000000000,
                   RunNumber=1,
                   EventID=1,
                   IncrementEventID=True)

    tray.AddModule(generateEvent , "generateEvent",
                   Type = dataclasses.I3Particle.ParticleType.EMinus,
                   NEvents = 1,
                   XCoord = -256.14,
                   YCoord = -521.08,
                   ZCoord = 496.03,
                   Primary_direction = dataclasses.I3Direction(0 , 0 ,-1),
                   Daughter_direction = dataclasses.I3Direction(0 , 0 , -1),
                   I3RandomService = randomService,
                   Energy = 1.0*I3Units.TeV )

    photonSeriesName = "Photons"
    MCTreeName = "I3MCTree"


    # prefer GPUs
    usegpus = any([device.gpu for device in clsim.I3CLSimOpenCLDevice.GetAllDevices()])    
    tray.AddSegment(clsim.I3CLSimMakePhotons,"MakePhotons",
                    UseGPUs = usegpus,
                    UseOnlyDeviceNumber=0,
                    UseCPUs = not usegpus,                    
                    PhotonSeriesName = photonSeriesName,
                    MCPESeriesName = None,
                    MCTreeName = MCTreeName,
                    RandomService = randomService,
                    IceModelLocation = expandvars("$I3_BUILD/ice-models/resources/models/spice_lea"),
                    CableOrientation = None,
                    GCDFile = gcd_file)
    tray.AddSegment(clsim.I3CLSimMakePhotons,"MakePhotonsWithShadow",
                    UseGPUs = usegpus,
                    UseOnlyDeviceNumber=0,
                    UseCPUs = not usegpus,
                    PhotonSeriesName = photonSeriesName+"AfterShadow",
                    MCPESeriesName = None,
                    MCTreeName = MCTreeName,
                    RandomService = randomService,
                    IceModelLocation = expandvars("$I3_BUILD/ice-models/resources/models/spice_lea"),
                    CableOrientation = expandvars("$I3_BUILD/ice-models/resources/models/cable_position/orientation.led7.txt"),
                    GCDFile = gcd_file)

    tray.AddModule("I3NullSplitter","physics")

    import unittest
    class SanityCheck(unittest.TestCase):
        photons = photonSeriesName
        shadowed_photons = photonSeriesName+"AfterShadow"


        def testKeys(self):
            self.assertTrue(self.shadowed_photons in self.frame, "The shadowed_photons actually shows up in the frame.")

        def testEnergy(self):
            non_shadowed = self.frame[self.photons]
            count_photons = 0

            for i in non_shadowed.values():
                count_photons += len(i)
            
            shadowed = self.frame[self.shadowed_photons]
            count_shadowed = 0
            for i in shadowed.values():
                count_shadowed+= len(i)
            icetray.logging.log_info("photons before shadow: {} after: {}".format(count_photons, count_shadowed), unit="testCableShadow")
            print(count_photons, count_shadowed)
            self.assertGreater(count_photons, 0, "some photons made it to DOMs")
            self.assertGreater(count_photons, count_shadowed, "some photons removed")
            self.assertAlmostEqual(count_shadowed/float(count_photons), 0.9, delta=0.03, msg="approximately 10% photons removed")

    tray.AddModule(icetray.I3TestModuleFactory(SanityCheck), 'testy')


    tray.Execute()

