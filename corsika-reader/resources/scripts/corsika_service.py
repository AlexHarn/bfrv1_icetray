import numpy as np
from icecube import icetray,dataclasses,clsim,corsika_reader,hdfwriter
from I3Tray import I3Tray

u=icetray.I3Units

corsika_service = corsika_reader.CorsikaService("/data/user/kmeagher/corsika-76900/run/corsika")

NEvents = 100000
#primary_type=dataclasses.I3Particle.PPlus
primary_type=dataclasses.I3Particle.He4Nucleus
#primary_type=dataclasses.I3Particle.N14Nucleus
#primary_type=dataclasses.I3Particle.Al27Nucleus
#primary_type=dataclasses.I3Particle.Fe56Nucleus
#primary_type=dataclasses.I3Particle.U238Nucleus
primary_energy = 100000
bias = .1

filename="CORSIKA_service.B{:02d}.{}.{}".format(
    int(1/bias),str(primary_type),primary_energy)

print(filename)

class Generator(icetray.I3Module):

    def __init__(self, context):
        icetray.I3Module.__init__(self, context)
        self.AddParameter("N","Number of frames to generate",10)
        
    def Configure(self):
        self.N=self.GetParameter("N")
        self.i=0

    def Process(self):
        self.i+=1
        if self.i >= self.N:
            self.RequestSuspension()

        frame=icetray.I3Frame('Q')
        i3pos=dataclasses.I3Position(0,0,0)
        i3dir=dataclasses.I3Direction(30*u.degree,0)    
        primary = dataclasses.I3Particle(i3pos,i3dir,0)
        primary.energy=primary_energy
        primary.type=primary_type
        primary.location_type=dataclasses.I3Particle.Anywhere
        frame["Primary"]=primary

        corsika_reader.add_bias(primary,bias,frame);
        
        self.PushFrame(frame)

class I3CorsikaServiceModule(icetray.I3Module):
    
    def __init__(self,context):
        icetray.I3Module.__init__(self,context)
        self.AddParameter("PrimaryName","Name of the frame object representing the primary particle","Primary")
        self.AddParameter("EventGenerator","Service to generate events",None)

    def Configure(self):
        self.primary_name=self.GetParameter("PrimaryName")
        self.generator = self.GetParameter("EventGenerator")


    def DAQ(self,frame):
        primary=frame[self.primary_name]
        self.generator.StartShower(primary,frame)
        tree=dataclasses.I3MCTree(primary)

        more = True
        while True:
            secondary = dataclasses.I3Particle()
            more=self.generator.NextParticle(secondary)
            if not more:
                break
            tree.append_child(primary,secondary)

        frame["I3MCTree"]=tree



        frame2=icetray.I3Frame(frame.Stop)
        self.generator.EndEvent(frame2)
        #frame.merge is broken so we have to do this the old fasioned way
        #for k in frame2.keys():
        #    frame[k]=frame2[k]
        frame.merge(frame2)

        self.PushFrame(frame)

tray = I3Tray()
tray.AddModule(Generator,N=NEvents)
tray.AddModule(I3CorsikaServiceModule,
               eventGenerator=corsika_service)
#tray.AddModule("Dump")
tray.AddModule("Delete", Keys=["ShowerBias"])
tray.AddModule("I3Writer",FileName=filename+'.i3.zst')
tray.Add(hdfwriter.I3SimHDFWriter,
         Keys=["I3MCTree","Primary","I3CorsikaWeight"],
         Output=filename+".hdf5",
         #SubEventStreams=['SimHDFWriter',],
)




tray.Execute()

"""
for i in range(3):

    i3pos=dataclasses.I3Position(0,0,0)
    i3dir=dataclasses.I3Direction(30*u.degree,0)
    
    primary = dataclasses.I3Particle(i3pos,i3dir,0)
    primary.energy=100_000
    primary.type=dataclasses.I3Particle.PPlus
    primary.location_type=dataclasses.I3Particle.Anywhere

    frame=icetray.I3Frame()
    cs.StartShower(primary,frame)

    tree=dataclasses.I3MCTree(primary)

    more = True
    while True:
        secondary = dataclasses.I3Particle()
        more=cs.NextParticle(secondary)
        if not more:
            break
        tree.append_child(primary,secondary)
        
    print(tree)
    frame2=icetray.I3Frame()
    cs.EndEvent(frame2)
"""
