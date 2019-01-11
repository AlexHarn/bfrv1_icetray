from icecube import icetray,dataio,dataclasses

####################################
## Add Tanks from these Pulses to an existing (or not yet) ExcludedTankList
## Application : SRTexcludedPulses to ClusterCleanedExcludedStations...
#######
class AddTanks(icetray.I3ConditionalModule):
    def __init__(self,context):
        icetray.I3ConditionalModule.__init__(self, context)
        self.AddParameter('TanksToAddName','Name of Tanks to add in the frame',0)
        self.AddParameter('ExcludedTanksName','Name of Already ExcludedTanks in the frame',0)
        self.AddParameter('OutputExcludedTanksName','Name of New ExcludedTanks in the frame',0)
        self.AddOutBox('OutBox')
        
    def Configure(self):
        self.toAdd = self.GetParameter('TanksToAddName')
        self.exclname = self.GetParameter('ExcludedTanksName')
        self.outputname = self.GetParameter('OutputExcludedTanksName')

    def Physics(self,frame):
        excltanks = 0
        if not self.exclname in frame:
            excltanks = dataclasses.TankKey.I3VectorTankKey()
        else:
            excltanks = dataclasses.TankKey.I3VectorTankKey(frame[self.exclname])  # Do a COPY !! 

        if self.toAdd in frame:
            tanksToAdd = frame[self.toAdd]
            for tank in tanksToAdd:
                if not tank in excltanks:
                    excltanks.append(tank)
        frame[self.outputname] = excltanks
        self.PushFrame(frame,"OutBox")
