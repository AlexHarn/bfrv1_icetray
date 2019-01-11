from icecube import icetray
from icecube import dataclasses

class I3ParticleForgeModule(icetray.I3ConditionalModule):
    '''
    Simple replacement for the old I3ParticleForgeModule.
    This module simply creates an I3Particle with properties
    from various frame objects.

    Note : MCTree and MCMethod are no longer supported since it
    didn't look like they were being used anymore.
    '''
    def __init__(self, context):

        icetray.I3ConditionalModule.__init__(self, context)

        self.AddOutBox("OutBox")

        self.AddParameter("Direction",
                          "Use the direction from this I3Particle for resulting I3Particle",
                          None)
        self.AddParameter("Energy",
                          "Use the energy from this I3Particle for resulting I3Particle",
                          None)
        self.AddParameter("Position",
                          "Use the position from this I3Particle for resulting I3Particle",
                          None)
        self.AddParameter("Time",
                          "Use the time from this I3Particle for resulting I3Particle",
                          None)
        self.AddParameter("Shape",
                          "Use this shape for the resulting I3Particle",
                          None)
        self.AddParameter("Type",
                          "Use this type for resulting I3Particle",
                          None)
        self.AddParameter("Status",
                          "This is the fit status.",
                          None )
        self.AddParameter("Output",
                          "This is the name of the resulting I3Particle frame key.",
                          "ParticleForgeResult")
        
    def Configure(self):
        self.direction_key = self.GetParameter("Direction")
        self.energy_key = self.GetParameter("Energy")
        self.output_key = self.GetParameter("Output")
        self.position_key = self.GetParameter("Position")
        self.time_key = self.GetParameter("Time")
        self.shape = self.GetParameter("Shape")
        self.type = self.GetParameter("Type")
        self.fit_status = self.GetParameter("Status")

    def Physics(self, frame):
        particle = dataclasses.I3Particle()

        if self.direction_key:
            particle.dir = frame[self.direction_key].dir

        if self.energy_key:
            particle.energy = frame[self.energy_key].energy

        if self.position_key:
            particle.pos = frame[self.position_key].pos

        if self.time_key:
            particle.time = frame[self.time_key].time

        if self.type != None:
            particle.type = self.type
        
        if self.shape != None :
            particle.shape = self.shape

        if self.fit_status != None :
            particle.fit_status = self.fit_status

        frame[self.output_key] = particle

        self.PushFrame(frame)
