#!/usr/bin/env python

import unittest

from icecube import icetray
from icecube import dataclasses
from icecube import credo
from I3Tray import I3Tray
from I3Tray import I3Units

from icecube.credo.particle_forge import I3ParticleForgeModule
from icecube.icetray.test_module import I3TestModuleFactory

def Source(frame):
    '''
    Add several particles to the frame that I3ParticleForgeModule
    will use to create a franken-particle.
    '''
    p_direction = dataclasses.I3Particle()
    p_direction.dir = dataclasses.I3Direction(3.14, 1.618)
    frame["ParticleDirection"] = p_direction    
            
    p_energy = dataclasses.I3Particle()
    p_energy.energy = 100*I3Units.TeV
    frame["ParticleEnergy"] = p_energy
            
    p_position = dataclasses.I3Particle()
    p_position.pos = dataclasses.I3Position(10,10,10)
    frame["ParticlePosition"] = p_position
            
    p_time = dataclasses.I3Particle()
    p_time.time = 400*I3Units.ns
    frame["ParticleTime"] = p_time                

class I3ParticleForgeTest(unittest.TestCase):

    def test_direction(self):
        test_particle = self.frame['ParticleForgeResult']
        self.assertEqual(test_particle.dir,
                         dataclasses.I3Direction(3.14, 1.618),
                         "directions are not the same.")

    def test_energy(self):
        test_particle = self.frame['ParticleForgeResult']
        self.assertEqual(test_particle.energy,
                         100*I3Units.TeV,
                         "energies are not the same.")

    def test_position(self):
        test_particle = self.frame['ParticleForgeResult']
        self.assertEqual(test_particle.pos,
                         dataclasses.I3Position(10,10,10),
                         "positions are not the same.")

    def test_time(self):
        test_particle = self.frame['ParticleForgeResult']
        self.assertEqual(test_particle.time,
                         400*I3Units.ns,
                         "times are not the same.")

    def test_shape(self):
        test_particle = self.frame['ParticleForgeResult']
        self.assertEqual(test_particle.shape,
                         dataclasses.I3Particle.Primary,
                         "shapes are not the same.")

    def test_type(self):
        test_particle = self.frame['ParticleForgeResult']
        self.assertEqual(test_particle.type,
                         dataclasses.I3Particle.Monopole,
                         "types are not the same.")

    def test_fit_status(self):
        test_particle = self.frame['ParticleForgeResult']
        self.assertEqual(test_particle.fit_status,
                         dataclasses.I3Particle.OK,
                         "fit_status %s and dataclasses.I3Particle.OK"
                         " are not the same." %
                         (test_particle.fit_status))
                                     
tray = I3Tray()
tray.AddModule("BottomlessSource")
tray.AddModule(Source)
tray.AddModule(I3ParticleForgeModule,
               Direction = 'ParticleDirection',
               Energy = 'ParticleEnergy',
               Position = 'ParticlePosition',
               Time = 'ParticleTime',
               Shape = dataclasses.I3Particle.Primary,
               Type = dataclasses.I3Particle.Monopole,
               Status = dataclasses.I3Particle.OK
               )
tray.AddModule(icetray.I3TestModuleFactory(I3ParticleForgeTest))
tray.Execute(1)
