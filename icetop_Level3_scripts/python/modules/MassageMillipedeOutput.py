from icecube import dataclasses, icetray, phys_services

class MassageMillipedeOutput(icetray.I3ConditionalModule):
    '''
    Massage the output: remove the fake peaks.
    TODO : use the covariance matrix/Fisher information matrix to decide better
    '''
    def __init__(self, ctx):
        icetray.I3ConditionalModule.__init__(self, ctx)
        self.AddParameter("ShowerSpacing","Spacing along the track",20)
        self.AddParameter("min_energy","Minimum energy to take bin into account",-1.0)
        self.AddParameter("inputname","key with Energy reconstruction along track","Millipede")
        self.AddParameter("outputname","Output I3ParticleVector to feed to I3Stochastics","Millipede_dEdX")
        self.AddParameter("min_dust","Lower boundary of dust layer",-140)
        self.AddParameter("max_dust","Upper boundary of dust layer",-30)
        self.AddParameter("fill_dust","Fill eloss track reco with average in dust layer",False)

    def Configure(self):
        self.spacing=self.GetParameter("ShowerSpacing")
        self.minE=self.GetParameter("min_energy")
        self.inputName=self.GetParameter("inputname")
        self.outputName=self.GetParameter("outputname")
        self.minDust=self.GetParameter("min_dust")
        self.maxDust=self.GetParameter("max_dust")
        self.fillDust=self.GetParameter("fill_dust")

    def Physics(self,frame):
        output2 = dataclasses.I3VectorI3Particle()
        if self.inputName not in frame:
            icetray.i3logging.log_warn(self.inputName+" not in frame, while it should be there.")
        else:    
            phd = frame[self.inputName]
            geo = frame['I3Geometry']
            scaling = phys_services.I3ScaleCalculator(geo)
            energies_cut = [part.energy/float(self.spacing) for part in phd if ((part.pos.z < 450 and part.pos.z > -450) and (part.pos.z < self.minDust or part.pos.z > self.maxDust) and scaling.scale_inice(part) < 1.1)]
            if len(energies_cut) > 0:
                av_loss = sum(energies_cut)
                av_loss /= len(energies_cut)
                for part in phd:
                    new_part = dataclasses.I3Particle(part)
                    new_part.energy = part.energy/float(self.spacing)
                    new_part.shape = dataclasses.I3Particle.Cascade
                    if part.pos.z < 450 and part.pos.z > -450 and scaling.scale_inice(part) < 1.1:
                        if part.pos.z < self.minDust or part.pos.z > self.maxDust:
                            if part.energy/float(self.spacing) > self.minE:
                                new_part = dataclasses.I3Particle(part)
                                new_part.energy = part.energy/float(self.spacing)
                                new_part.shape = dataclasses.I3Particle.Cascade
                                output2.append(new_part)
                                
                        elif self.fillDust:
                            new_part = dataclasses.I3Particle(part)
                            new_part.energy = av_loss
                            new_part.shape = dataclasses.I3Particle.Cascade
                            output2.append(new_part)
                            
                frame[self.outputName] = output2
        self.PushFrame(frame,"OutBox")
